#pragma once

#include "Generator.h"
#include "LazyOperationType.h"

#include <stdexcept>

template <class T>
class LazySequence;

template <class T>
class LazyOperationGenerator : public Generator<T> {
private:
    LazySequence<T>* first_;
    LazySequence<T>* second_;
    T* item_;

    LazyOperationType operation_type_;

    int index_;
    int position_;

    bool is_infinite_;
    int finite_length_;

public:
    LazyOperationGenerator(
        const LazySequence<T>& source,
        const T& item,
        int index,
        bool isInfinite,
        int finiteLength
    );

    LazyOperationGenerator(
        const LazySequence<T>& first,
        const LazySequence<T>& second,
        bool isInfinite,
        int finiteLength
    );

    LazyOperationGenerator(const LazyOperationGenerator<T>& other);

    LazyOperationGenerator<T>& operator=(const LazyOperationGenerator<T>& other);

    ~LazyOperationGenerator() override;

    bool HasNext() const override;
    T GetNext() override;

    int GetPosition() const override;

    void SetSource(const Sequence<T>& source) override;

    void Reset() override;

    Generator<T>* Clone() const override;
};