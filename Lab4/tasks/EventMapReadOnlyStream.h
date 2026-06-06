#pragma once

#include "../streams/ReadOnlyStream.h"
#include "Event.h"
#include "EventType.h"

#include <stdexcept>


template <class T>
class EventMapReadOnlyStream : public ReadOnlyStream<Event<T>> {
private:
    ReadOnlyStream<Event<T>>* source_;
    T(*value_mapper_)(const T&);

public:
    EventMapReadOnlyStream(
        ReadOnlyStream<Event<T>>* source,
        T(*valueMapper)(const T&)
    );

    void Open() override;
    void Close() override;

    bool IsEndOfStream() const override;

    bool IsCanSeek() const override;
    bool IsCanGoBack() const override;

    int GetPosition() const override;

    Event<T> Read() override;
    int Seek(int index) override;
};

#include "EventMapReadOnlyStream.tpp"