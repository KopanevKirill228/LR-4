#pragma once


template <class T>
class Stream {
public:
    virtual ~Stream() = default;

    virtual void Open() = 0;
    virtual void Close() = 0;

    virtual int GetPosition() const = 0;
};