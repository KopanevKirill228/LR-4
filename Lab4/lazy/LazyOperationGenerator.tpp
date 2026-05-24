#pragma once


static Cardinal AddCardinal(const Cardinal& first, const Cardinal& second) {
    if (first.IsInfinite() || second.IsInfinite()) {
        return Cardinal::Infinity();
    }

    return Cardinal::Finite(first.GetFiniteValue() + second.GetFiniteValue());
}


template <class T>
LazyOperationGenerator<T>::LazyOperationGenerator(
    const LazySequence<T>& first,
    const LazySequence<T>& second,
    bool isConcat
)
    : first_(new LazySequence<T>(first)),
    second_(new LazySequence<T>(second)),
    operation_type_(LazyOperationType::Concat),
    first_length_(first.GetCardinality()),
    second_length_(second.GetCardinality()),
    result_length_(AddCardinal(first.GetCardinality(), second.GetCardinality())),
    index_(0),
    position_(0)
{
    if (!isConcat) {
        delete first_;
        delete second_;

        first_ = nullptr;
        second_ = nullptr;

        throw std::invalid_argument("LazyOperationGenerator: invalid concat flag");
    }
}


template <class T>
LazyOperationGenerator<T>::LazyOperationGenerator(
    const LazySequence<T>& source,
    const LazySequence<T>& inserted,
    int index
)
    : first_(new LazySequence<T>(source)),
    second_(new LazySequence<T>(inserted)),
    operation_type_(LazyOperationType::InsertSequence),
    first_length_(source.GetCardinality()),
    second_length_(inserted.GetCardinality()),
    result_length_(AddCardinal(source.GetCardinality(), inserted.GetCardinality())),
    index_(index),
    position_(0)
{
    if (index_ < 0) {
        delete first_;
        delete second_;

        first_ = nullptr;
        second_ = nullptr;

        throw std::out_of_range("LazyOperationGenerator: index is negative");
    }

    if (first_length_.IsFinite() && index_ > first_length_.GetFiniteValue()) {
        delete first_;
        delete second_;

        first_ = nullptr;
        second_ = nullptr;

        throw std::out_of_range("LazyOperationGenerator: index is out of range");
    }
}


template <class T>
LazyOperationGenerator<T>::LazyOperationGenerator(const LazyOperationGenerator<T>& other)
    : first_(other.first_ == nullptr ? nullptr : new LazySequence<T>(*other.first_)),
    second_(other.second_ == nullptr ? nullptr : new LazySequence<T>(*other.second_)),
    operation_type_(other.operation_type_),
    first_length_(other.first_length_),
    second_length_(other.second_length_),
    result_length_(other.result_length_),
    index_(other.index_),
    position_(other.position_) {
}


template <class T>
LazyOperationGenerator<T>& LazyOperationGenerator<T>::operator=(
    const LazyOperationGenerator<T>& other
    ) {
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

    operation_type_ = other.operation_type_;
    first_length_ = other.first_length_;
    second_length_ = other.second_length_;
    result_length_ = other.result_length_;
    index_ = other.index_;
    position_ = other.position_;

    return *this;
}


template <class T>
LazyOperationGenerator<T>::~LazyOperationGenerator() {
    delete first_;
    delete second_;
}


template <class T>
bool LazyOperationGenerator<T>::HasNext() const {
    if (result_length_.IsInfinite()) {
        return true;
    }

    return position_ < result_length_.GetFiniteValue();
}


template <class T>
T LazyOperationGenerator<T>::GetNext() {
    if (!HasNext()) {
        throw std::out_of_range("LazyOperationGenerator: no next element");
    }

    T value;

    if (operation_type_ == LazyOperationType::Concat) {
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
    }
    else if (operation_type_ == LazyOperationType::InsertSequence) {
        if (position_ < index_) {
            value = first_->Get(position_);
        }
        else if (second_length_.IsInfinite()) {
            value = second_->Get(position_ - index_);
        }
        else {
            int second_length = second_length_.GetFiniteValue();

            if (position_ < index_ + second_length) {
                value = second_->Get(position_ - index_);
            }
            else {
                value = first_->Get(position_ - second_length);
            }
        }
    }
    else {
        throw std::logic_error("LazyOperationGenerator: unknown operation type");
    }

    ++position_;

    return value;
}


template <class T>
int LazyOperationGenerator<T>::GetPosition() const {
    return position_;
}


template <class T>
void LazyOperationGenerator<T>::SetSource(const Sequence<T>&) {
}


template <class T>
void LazyOperationGenerator<T>::Reset() {
    position_ = 0;
}


template <class T>
Generator<T>* LazyOperationGenerator<T>::Clone() const {
    return new LazyOperationGenerator<T>(*this);
}


template <class T>
Cardinal LazyOperationGenerator<T>::GetResultCardinality() const {
    return result_length_;
}

template <class T>
T LazyOperationGenerator<T>::GetAfterInfinite(int index) const {
    if (index < 0) {
        throw std::out_of_range("LazyOperationGenerator GetAfterInfinite: index is negative");
    }

    if (operation_type_ == LazyOperationType::Concat) {
        if (first_length_.IsInfinite()) {
            return second_->Get(index);
        }

        return second_->GetAfterInfinite(index);
    }

    if (operation_type_ == LazyOperationType::InsertSequence) {
        if (second_length_.IsInfinite()) {
            return first_->Get(index_ + index);
        }

        return first_->GetAfterInfinite(index);
    }

    throw std::logic_error("LazyOperationGenerator GetAfterInfinite: unknown operation type");
}