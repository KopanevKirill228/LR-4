#pragma once

#include "../streams/ReadOnlyStream.h"
#include "EventParser.h"


template <class T>
class EventReadOnlyStream : public ReadOnlyStream<Event<T>> {
private:
    ReadOnlyStream<std::string>* source_;

public:
    explicit EventReadOnlyStream(ReadOnlyStream<std::string>* source);

    bool IsEndOfStream() const override;
    Event<T> Read() override;

    int GetPosition() const override;

    bool IsCanSeek() const override;
    int Seek(int index) override;

    bool IsCanGoBack() const override;

    void Open() override;
    void Close() override;
};

#include "EventReadOnlyStream.tpp"