#pragma once


template <class T>
EventMapReadOnlyStream<T>::EventMapReadOnlyStream(
    ReadOnlyStream<Event<T>>* source,
    T(*valueMapper)(const T&)
)
    : source_(source),
    value_mapper_(valueMapper)
{
    if (source_ == nullptr) {
        throw std::invalid_argument("EventMapReadOnlyStream: source is nullptr");
    }

    if (value_mapper_ == nullptr) {
        throw std::invalid_argument("EventMapReadOnlyStream: mapper is nullptr");
    }
}


template <class T>
void EventMapReadOnlyStream<T>::Open() {
    source_->Open();
}


template <class T>
void EventMapReadOnlyStream<T>::Close() {
    source_->Close();
}


template <class T>
bool EventMapReadOnlyStream<T>::IsEndOfStream() const {
    return source_->IsEndOfStream();
}


template <class T>
bool EventMapReadOnlyStream<T>::IsCanSeek() const {
    return source_->IsCanSeek();
}


template <class T>
bool EventMapReadOnlyStream<T>::IsCanGoBack() const {
    return source_->IsCanGoBack();
}


template <class T>
int EventMapReadOnlyStream<T>::GetPosition() const {
    return source_->GetPosition();
}


template <class T>
Event<T> EventMapReadOnlyStream<T>::Read() {
    Event<T> event = source_->Read();

    if (event.type == EventType::Measure) {
        return Event<T>(
            EventType::Measure,
            value_mapper_(event.value),
            event.message
        );
    }

    return event;
}


template <class T>
int EventMapReadOnlyStream<T>::Seek(int index) {
    return source_->Seek(index);
}