#pragma once

#include "../lib/Sequence.h"
#include "../lazy/Cardinal.h"
#include "../lazy/TransfiniteIndex.h"
#include "../lazy/TransfiniteLength.h"
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

    virtual T GetAfterInfinite(int index) const {
        throw std::logic_error("Generator: after-infinity index is not supported");

        return GetByTransfiniteIndex(TransfiniteIndex::AfterInfinity(index));
    }

    virtual Cardinal GetResultCardinality() const = 0;

    virtual TransfiniteLength GetResultLength() const = 0;
};