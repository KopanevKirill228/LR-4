#pragma once

#include <stdexcept>
#include "EventReadOnlyStream.h"

template <class T>
EventReadOnlyStream<T>::EventReadOnlyStream(ReadOnlyStream<std::string>* source)
    : source_(source) {
    if (source_ == nullptr) {
        throw std::invalid_argument("EventReadOnlyStream: source is nullptr");
    }
}


template <class T>
bool EventReadOnlyStream<T>::IsEndOfStream() const {
    return source_->IsEndOfStream();
}


template <class T>
Event<T> EventReadOnlyStream<T>::Read() {
    std::string line = source_->Read();

    return EventParser<T>::ParseLine(line);
}


template <class T>
int EventReadOnlyStream<T>::GetPosition() const {
    return source_->GetPosition();
}


template <class T>
bool EventReadOnlyStream<T>::IsCanSeek() const {
    return source_->IsCanSeek();
}


template <class T>
int EventReadOnlyStream<T>::Seek(int index) {
    return source_->Seek(index);
}


template <class T>
bool EventReadOnlyStream<T>::IsCanGoBack() const {
    return source_->IsCanGoBack();
}


template <class T>
void EventReadOnlyStream<T>::Open() {
    source_->Open();
}


template <class T>
void EventReadOnlyStream<T>::Close() {
    source_->Close();
}