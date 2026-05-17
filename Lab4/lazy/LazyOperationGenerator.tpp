#pragma once


#include "LazyOperationGenerator.h"


template <class T>
LazyOperationGenerator<T>::LazyOperationGenerator(
    const LazySequence<T>& source,
    const T& item,
    int index,
    bool isInfinite,
    int finiteLength
)
    : first_(new LazySequence<T>(source)),
    second_(nullptr),
    item_(new T(item)),
    operation_type_(LazyOperationType::Insert),
    index_(index),
    position_(0),
    is_infinite_(isInfinite),
    finite_length_(finiteLength)
{
    if (index_ < 0) {
        delete first_;
        delete item_;

        first_ = nullptr;
        item_ = nullptr;

        throw std::out_of_range("LazyOperationGenerator: index is negative");
    }
}


template <class T>
LazyOperationGenerator<T>::LazyOperationGenerator(
    const LazySequence<T>& first,
    const LazySequence<T>& second,
    bool isInfinite,
    int finiteLength
)
    : first_(new LazySequence<T>(first)),
    second_(new LazySequence<T>(second)),
    item_(nullptr),
    operation_type_(LazyOperationType::Concat),
    index_(first.GetLength()),
    position_(0),
    is_infinite_(isInfinite),
    finite_length_(finiteLength) {
}


template <class T>
LazyOperationGenerator<T>::LazyOperationGenerator(const LazyOperationGenerator<T>& other)
    : first_(other.first_ == nullptr ? nullptr : new LazySequence<T>(*other.first_)),
    second_(other.second_ == nullptr ? nullptr : new LazySequence<T>(*other.second_)),
    item_(other.item_ == nullptr ? nullptr : new T(*other.item_)),
    operation_type_(other.operation_type_),
    index_(other.index_),
    position_(other.position_),
    is_infinite_(other.is_infinite_),
    finite_length_(other.finite_length_) {
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

    T* new_item = other.item_ == nullptr
        ? nullptr
        : new T(*other.item_);

    delete first_;
    delete second_;
    delete item_;

    first_ = new_first;
    second_ = new_second;
    item_ = new_item;

    operation_type_ = other.operation_type_;
    index_ = other.index_;
    position_ = other.position_;
    is_infinite_ = other.is_infinite_;
    finite_length_ = other.finite_length_;

    return *this;
}


template <class T>
LazyOperationGenerator<T>::~LazyOperationGenerator() {
    delete first_;
    delete second_;
    delete item_;
}


template <class T>
bool LazyOperationGenerator<T>::HasNext() const {
    if (is_infinite_) {
        return true;
    }

    return position_ < finite_length_;
}


template <class T>
T LazyOperationGenerator<T>::GetNext() {
    if (!HasNext()) {
        throw std::out_of_range("LazyOperationGenerator: no next element");
    }

    T value;

    if (operation_type_ == LazyOperationType::Insert) {
        if (position_ < index_) {
            value = first_->Get(position_);
        }
        else if (position_ == index_) {
            value = *item_;
        }
        else {
            value = first_->Get(position_ - 1);
        }
    }
    else {
        if (position_ < index_) {
            value = first_->Get(position_);
        }
        else {
            value = second_->Get(position_ - index_);
        }
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