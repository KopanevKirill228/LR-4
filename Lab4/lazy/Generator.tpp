#pragma once

#include "Generator.h"


template <class T>
Generator<T>::Generator()
    : rule_(nullptr),
    source_(nullptr),
    position_(0),
    has_next_(false) {
}


template <class T>
Generator<T>::Generator(
    std::function<T(const Sequence<T>*)> rule,
    const Sequence<T>* source)
    : rule_(rule),
    source_(source),
    position_(0),
    has_next_(true)
{
    if (!rule_) {
        throw std::invalid_argument("Generator: rule is empty");
    }

    if (source_ == nullptr) {
        throw std::invalid_argument("Generator: source is nullptr");
    }
}


template <class T>
Generator<T>::Generator(const Generator<T>& other)
    : rule_(other.rule_),
    source_(other.source_),
    position_(other.position_),
    has_next_(other.has_next_) {
}


template <class T>
Generator<T>& Generator<T>::operator=(const Generator<T>& other) {
    if (this == &other) {
        return *this;
    }

    rule_ = other.rule_;
    source_ = other.source_;
    position_ = other.position_;
    has_next_ = other.has_next_;

    return *this;
}


template <class T>
bool Generator<T>::HasNext() const {
    return has_next_;
}


template <class T>
T Generator<T>::GetNext() {
    if (!has_next_) {
        throw std::out_of_range("Generator: no next element");
    }

    if (!rule_) {
        throw std::runtime_error("Generator: rule is empty");
    }

    if (source_ == nullptr) {
        throw std::runtime_error("Generator: source is nullptr");
    }

    T value = rule_(source_);
    ++position_;

    return value;
}


template <class T>
int Generator<T>::GetPosition() const {
    return position_;
}


template <class T>
void Generator<T>::SetSource(const Sequence<T>* source) {
    if (source == nullptr) {
        throw std::invalid_argument("Generator: source is nullptr");
    }

    source_ = source;
}


template <class T>
void Generator<T>::Reset() {
    position_ = 0;
}