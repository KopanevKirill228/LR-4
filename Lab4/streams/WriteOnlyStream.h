#pragma once


template <class T>
class WriteOnlyStream {
public:
    virtual ~WriteOnlyStream() = default;

    virtual void Open() = 0;
    virtual void Close() = 0;

    virtual int GetPosition() const = 0;
    virtual int Write(const T& item) = 0;
};