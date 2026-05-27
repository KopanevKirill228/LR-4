#pragma once

#include "Stream.h"


template <class T>
class WriteOnlyStream : public Stream<T> {
public:
    virtual ~WriteOnlyStream() override = default;

    virtual int Write(const T& item) = 0;
};