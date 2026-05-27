#pragma once


static Cardinal ConcatAddCardinal(const Cardinal& first, const Cardinal& second) {
    if (first.IsInfinite() || second.IsInfinite()) {
        return Cardinal::Infinity();
    }

    return Cardinal::Finite(first.GetFiniteValue() + second.GetFiniteValue());
}


template <class T>
ConcatGenerator<T>::ConcatGenerator(
    const LazySequence<T>& first,
    const LazySequence<T>& second
)
    : first_(new LazySequence<T>(first)),
    second_(new LazySequence<T>(second)),
    first_length_(first.GetCardinality()),
    second_length_(second.GetCardinality()),
    result_length_(ConcatAddCardinal(first.GetCardinality(), second.GetCardinality())),
    position_(0) {
}


template <class T>
ConcatGenerator<T>::ConcatGenerator(const ConcatGenerator<T>& other)
    : first_(other.first_ == nullptr ? nullptr : new LazySequence<T>(*other.first_)),
    second_(other.second_ == nullptr ? nullptr : new LazySequence<T>(*other.second_)),
    first_length_(other.first_length_),
    second_length_(other.second_length_),
    result_length_(other.result_length_),
    position_(other.position_) {
}


template <class T>
ConcatGenerator<T>& ConcatGenerator<T>::operator=(const ConcatGenerator<T>& other) {
    if (this == &other) {
        return *this;
    }

    LazySequence<T>* new_first = other.first_ == nullptr
        ? nullptr
        : new LazySequence<T>(*other.first_);

    LazySequence<T>* new_second = other.second_ == nullptr
        ? nullptr
        : new LazySequence<T>(*other.second_);

    delete first_;
    delete second_;

    first_ = new_first;
    second_ = new_second;

    first_length_ = other.first_length_;
    second_length_ = other.second_length_;
    result_length_ = other.result_length_;
    position_ = other.position_;

    return *this;
}


template <class T>
ConcatGenerator<T>::~ConcatGenerator() {
    delete first_;
    delete second_;
}


template <class T>
bool ConcatGenerator<T>::HasNext() const {
    if (result_length_.IsInfinite()) {
        return true;
    }

    return position_ < result_length_.GetFiniteValue();
}


template <class T>
T ConcatGenerator<T>::GetNext() {
    if (!HasNext()) {
        throw std::out_of_range("ConcatGenerator: no next element");
    }

    T value;

    if (first_length_.IsInfinite()) {
        value = first_->Get(position_);
    }
    else {
        int first_length = first_length_.GetFiniteValue();

        if (position_ < first_length) {
            value = first_->Get(position_);
        }
        else {
            value = second_->Get(position_ - first_length);
        }
    }

    ++position_;

    return value;
}


template <class T>
int ConcatGenerator<T>::GetPosition() const {
    return position_;
}


template <class T>
void ConcatGenerator<T>::SetSource(const Sequence<T>&) {
}


template <class T>
void ConcatGenerator<T>::Reset() {
    position_ = 0;
}


template <class T>
Generator<T>* ConcatGenerator<T>::Clone() const {
    return new ConcatGenerator<T>(*this);
}


template <class T>
T ConcatGenerator<T>::GetByTransfiniteIndex(const TransfiniteIndex& index) const {
    if (index.IsFinite()) {
        int finite_index = index.GetFiniteIndex();

        if (first_length_.IsInfinite()) {
            return first_->Get(finite_index);
        }

        int first_length = first_length_.GetFiniteValue();

        if (finite_index < first_length) {
            return first_->Get(finite_index);
        }

        return second_->Get(finite_index - first_length);
    }

    int infinity_count = index.GetInfinityCount();
    int tail_index = index.GetFiniteIndex();

    if (first_length_.IsInfinite()) {
        if (infinity_count == 1) {
            int used_by_first = 0;

            while (used_by_first <= tail_index) {
                try {
                    T value = first_->Get(
                        TransfiniteIndex::AfterInfinity(used_by_first)
                    );

                    if (used_by_first == tail_index) {
                        return value;
                    }

                    ++used_by_first;
                }
                catch (...) {
                    return second_->Get(tail_index - used_by_first);
                }
            }
        }

        try {
            return first_->Get(index);
        }
        catch (...) {
            return second_->Get(
                TransfiniteIndex(infinity_count - 1, tail_index)
            );
        }
    }

    return second_->Get(index);
}


template <class T>
Cardinal ConcatGenerator<T>::GetResultCardinality() const {
    return result_length_;
}