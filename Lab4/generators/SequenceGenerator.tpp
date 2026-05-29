#pragma once


template <class T>
SequenceGenerator<T>::SequenceGenerator(const Sequence<T>& source)
    : source_(&source),
    position_(0) {
}


template <class T>
SequenceGenerator<T>::SequenceGenerator(const SequenceGenerator<T>& other)
    : source_(other.source_),
    position_(other.position_) {
}


template <class T>
SequenceGenerator<T>& SequenceGenerator<T>::operator=(const SequenceGenerator<T>& other) {
    if (this == &other) {
        return *this;
    }

    source_ = other.source_;
    position_ = other.position_;

    return *this;
}


template <class T>
bool SequenceGenerator<T>::HasNext() const {
    if (source_ == nullptr) {
        throw std::runtime_error("SequenceGenerator: source is nullptr");
    }

    return position_ < source_->GetLength();
}


template <class T>
T SequenceGenerator<T>::GetNext() {
    if (source_ == nullptr) {
        throw std::runtime_error("SequenceGenerator: source is nullptr");
    }

    if (position_ >= source_->GetLength()) {
        throw std::out_of_range("SequenceGenerator: no next element");
    }

    T value = source_->Get(position_);
    ++position_;

    return value;
}


template <class T>
int SequenceGenerator<T>::GetPosition() const {
    return position_;
}


template <class T>
void SequenceGenerator<T>::SetSource(const Sequence<T>& source) {
    source_ = &source;
    position_ = 0;
}


template <class T>
void SequenceGenerator<T>::Reset() {
    position_ = 0;
}


template <class T>
Generator<T>* SequenceGenerator<T>::Clone() const {
    return new SequenceGenerator<T>(*this);
}

template <class T>
Cardinal SequenceGenerator<T>::GetResultCardinality() const {
    return Cardinal::Finite(source_->GetLength());
}

template <class T>
TransfiniteLength SequenceGenerator<T>::GetResultLength() const {
    return TransfiniteLength::Finite(source_->GetLength());
}