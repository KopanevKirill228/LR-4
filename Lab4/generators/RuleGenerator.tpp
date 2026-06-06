#pragma once

#include "RuleGenerator.h"


template <class T>
RuleGenerator<T>::RuleGenerator(
    T(*rule)(const Sequence<T>&),
    const Sequence<T>& source
)
    : rule_(rule),
    source_(&source),
    position_(0)
{
    if (rule_ == nullptr) {
        throw std::invalid_argument("RuleGenerator: rule is nullptr");
    }
}

template <class T>
RuleGenerator<T>::RuleGenerator(const RuleGenerator<T>& other)
    : rule_(other.rule_),
    source_(other.source_),
    position_(other.position_) {
}


template <class T>
RuleGenerator<T>& RuleGenerator<T>::operator=(const RuleGenerator<T>& other) {
    if (this == &other) {
        return *this;
    }

    rule_ = other.rule_;
    source_ = other.source_;
    position_ = other.position_;

    return *this;
}


template <class T>
bool RuleGenerator<T>::HasNext() const {
    return true;
}


template <class T>
T RuleGenerator<T>::GetNext() {
    if (!rule_) {
        throw std::runtime_error("RuleGenerator: rule is empty");
    }

    if (source_ == nullptr) {
        throw std::runtime_error("RuleGenerator: source is nullptr");
    }

    T value = rule_(*source_);
    ++position_;

    return value;
}


template <class T>
int RuleGenerator<T>::GetPosition() const {
    return position_;
}


template <class T>
void RuleGenerator<T>::SetSource(const Sequence<T>& source) {
    source_ = &source;
}


template <class T>
void RuleGenerator<T>::Reset() {
    position_ = 0;
}


template <class T>
Generator<T>* RuleGenerator<T>::Clone() const {
    return new RuleGenerator<T>(*this);
}

template <class T>
Cardinal RuleGenerator<T>::GetResultCardinality() const {
    return Cardinal::Infinity();
}

template <class T>
TransfiniteLength RuleGenerator<T>::GetResultLength() const {
    return TransfiniteLength::Omega();
}