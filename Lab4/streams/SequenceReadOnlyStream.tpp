#pragma once

#include "SequenceReadOnlyStream.h"


template <class T>
SequenceReadOnlyStream<T>::SequenceReadOnlyStream(const Sequence<T>* sequence)
    : sequence_(sequence),
    position_(0),
    is_open_(false)
{
    if (sequence_ == nullptr) {
        throw std::invalid_argument("SequenceReadOnlyStream: sequence is nullptr");
    }
}


template <class T>
void SequenceReadOnlyStream<T>::Open() {
    position_ = 0;
    is_open_ = true;
}


template <class T>
void SequenceReadOnlyStream<T>::Close() {
    is_open_ = false;
}


template <class T>
bool SequenceReadOnlyStream<T>::IsEndOfStream() const {
    if (!is_open_) {
        throw StreamIsClosedException();
    }

    return position_ >= sequence_->GetLength();
}


template <class T>
T SequenceReadOnlyStream<T>::Read() {
    if (!is_open_) {
        throw StreamIsClosedException();
    }

    if (position_ >= sequence_->GetLength()) {
        throw EndOfStreamException();
    }

    T value = sequence_->Get(position_);
    ++position_;

    return value;
}


template <class T>
int SequenceReadOnlyStream<T>::GetPosition() const {
    return position_;
}


template <class T>
bool SequenceReadOnlyStream<T>::IsCanSeek() const {
    return true;
}


template <class T>
int SequenceReadOnlyStream<T>::Seek(int index) {
    if (!is_open_) {
        throw StreamIsClosedException();
    }

    if (index < 0 || index > sequence_->GetLength()) {
        throw StreamSeekException();
    }

    position_ = index;

    return position_;
}


template <class T>
bool SequenceReadOnlyStream<T>::IsCanGoBack() const {
    return true;
}