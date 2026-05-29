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
    first_transfinite_length_(first.GetTransfiniteLength()),
    second_transfinite_length_(second.GetTransfiniteLength()),
    result_transfinite_length_(
        TransfiniteLength::Add(
            first.GetTransfiniteLength(),
            second.GetTransfiniteLength()
        )
    ),
    position_(0) {
}


template <class T>
ConcatGenerator<T>::ConcatGenerator(const ConcatGenerator<T>& other)
    : first_(other.first_ == nullptr ? nullptr : new LazySequence<T>(*other.first_)),
    second_(other.second_ == nullptr ? nullptr : new LazySequence<T>(*other.second_)),
    first_length_(other.first_length_),
    second_length_(other.second_length_),
    result_length_(other.result_length_),
    first_transfinite_length_(other.first_transfinite_length_),
    second_transfinite_length_(other.second_transfinite_length_),
    result_transfinite_length_(other.result_transfinite_length_),
    position_(other.position_) {
}


template <class T>
ConcatGenerator<T>& ConcatGenerator<T>::operator=(const ConcatGenerator<T>& other) {
    if (this == &other) {
        return *this;
    }

    delete first_;
    delete second_;

    first_ = other.first_ == nullptr ? nullptr : new LazySequence<T>(*other.first_);
    second_ = other.second_ == nullptr ? nullptr : new LazySequence<T>(*other.second_);

    first_length_ = other.first_length_;
    second_length_ = other.second_length_;
    result_length_ = other.result_length_;

    first_transfinite_length_ = other.first_transfinite_length_;
    second_transfinite_length_ = other.second_transfinite_length_;
    result_transfinite_length_ = other.result_transfinite_length_;

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
T ConcatGenerator<T>::GetAfterInfinite(int index) const {
    if (index < 0) {
        throw std::out_of_range("ConcatGenerator: after-infinity index is negative");
    }

    if (first_length_.IsInfinite()) {
        int used_by_first = 0;

        while (used_by_first <= index) {
            try {
                T value = first_->GetAfterInfinite(used_by_first);

                if (used_by_first == index) {
                    return value;
                }

                ++used_by_first;
            }
            catch (...) {
                return second_->Get(index - used_by_first);
            }
        }
    }

    return second_->GetAfterInfinite(index);
}

template <class T>
T ConcatGenerator<T>::GetByTransfiniteIndex(const TransfiniteIndex& index) const {
    if (first_transfinite_length_.Contains(index)) {
        if (index.IsFinite()) {
            return first_->Get(index.GetFiniteIndex());
        }

        return first_->Get(index);
    }

    TransfiniteIndex second_index =
        first_transfinite_length_.SubtractFrom(index);

    if (second_index.IsFinite()) {
        return second_->Get(second_index.GetFiniteIndex());
    }

    return second_->Get(second_index);
}

template <class T>
Cardinal ConcatGenerator<T>::GetResultCardinality() const {
    return result_length_;
}

template <class T>
TransfiniteLength ConcatGenerator<T>::GetResultLength() const {
    return result_transfinite_length_;
}
