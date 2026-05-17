#pragma once

#include "../lib/Sequence.h"


template <class T>
class Generator {
public:
    virtual ~Generator() = default;

    virtual bool HasNext() const = 0;
    virtual T GetNext() = 0;

    virtual int GetPosition() const = 0;

    virtual void SetSource(const Sequence<T>& source) = 0;

    virtual void Reset() = 0;

    virtual Generator<T>* Clone() const = 0;
};