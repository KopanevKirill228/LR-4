#pragma once

#include "Stream.h"


template <class T>
class ReadOnlyStream : public Stream<T> {
public:
    virtual ~ReadOnlyStream() override = default;

    virtual bool IsEndOfStream() const = 0;
    virtual T Read() = 0;

    virtual bool IsCanSeek() const = 0;
    virtual int Seek(int index) = 0;

    virtual bool IsCanGoBack() const = 0;
};