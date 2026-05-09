#pragma once

#include "LazyReadOnlyStream.h"


template <class T>
LazyReadOnlyStream<T>::LazyReadOnlyStream(const LazySequence<T>* sequence)
    : sequence_(sequence),
    position_(0),
    is_open_(false)
{
    if (sequence_ == nullptr) {
        throw std::invalid_argument("LazyReadOnlyStream: sequence is nullptr");
    }
}


template <class T>
void LazyReadOnlyStream<T>::Open() {
    position_ = 0;
    is_open_ = true;
}


template <class T>
void LazyReadOnlyStream<T>::Close() {
    is_open_ = false;
}


template <class T>
bool LazyReadOnlyStream<T>::IsEndOfStream() const {
    if (!is_open_) {
        throw StreamIsClosedException();
    }

    if (sequence_->IsInfinite()) {
        return false;
    }

    return position_ >= sequence_->GetLength();
}


template <class T>
T LazyReadOnlyStream<T>::Read() {
    if (!is_open_) {
        throw StreamIsClosedException();
    }

    if (IsEndOfStream()) {
        throw EndOfStreamException();
    }

    T value = sequence_->Get(position_);
    ++position_;

    return value;
}


template <class T>
int LazyReadOnlyStream<T>::GetPosition() const {
    return position_;
}


template <class T>
bool LazyReadOnlyStream<T>::IsCanSeek() const {
    return true;
}


template <class T>
int LazyReadOnlyStream<T>::Seek(int index) {
    if (!is_open_) {
        throw StreamIsClosedException();
    }

    if (index < 0) {
        throw StreamSeekException();
    }

    if (!sequence_->IsInfinite() && index > sequence_->GetLength()) {
        throw StreamSeekException();
    }

    position_ = index;

    return position_;
}


template <class T>
bool LazyReadOnlyStream<T>::IsCanGoBack() const {
    return true;
}