#pragma once

#include "SequenceWriteOnlyStream.h"


template <class T>
SequenceWriteOnlyStream<T>::SequenceWriteOnlyStream(
    Sequence<T>*& sequence,
    bool deleteReplaced)
    : sequence_(sequence),
    position_(0),
    is_open_(false),
    delete_replaced_(deleteReplaced)
{
    if (sequence_ == nullptr) {
        throw std::invalid_argument("SequenceWriteOnlyStream: sequence is nullptr");
    }
}


template <class T>
void SequenceWriteOnlyStream<T>::Open() {
    position_ = 0;
    is_open_ = true;
}


template <class T>
void SequenceWriteOnlyStream<T>::Close() {
    is_open_ = false;
}


template <class T>
int SequenceWriteOnlyStream<T>::GetPosition() const {
    return position_;
}


template <class T>
int SequenceWriteOnlyStream<T>::Write(const T& item) {
    if (!is_open_) {
        throw StreamIsClosedException();
    }

    Sequence<T>* old = sequence_;
    Sequence<T>* next = sequence_->Append(item);

    if (next == nullptr) {
        throw std::runtime_error("SequenceWriteOnlyStream: Append returned nullptr");
    }

    if (next != old) {
        if (delete_replaced_) {
            delete old;
        }

        sequence_ = next;
    }

    ++position_;

    return position_;
}