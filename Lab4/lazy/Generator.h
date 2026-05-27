#pragma once

#include "../lib/Sequence.h"
#include "TransfiniteIndex.h"
#include <stdexcept>


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

    virtual T GetByTransfiniteIndex(const TransfiniteIndex& index) const {
        throw std::logic_error("Generator: transfinite index is not supported");
    }
};