#pragma once


template <class T>
class ReadOnlyStream {
public:
    virtual ~ReadOnlyStream() = default;

    virtual void Open() = 0;
    virtual void Close() = 0;

    virtual bool IsEndOfStream() const = 0;
    virtual T Read() = 0;

    virtual int GetPosition() const = 0;

    virtual bool IsCanSeek() const = 0;
    virtual int Seek(int index) = 0;

    virtual bool IsCanGoBack() const = 0;
};