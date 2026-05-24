#pragma once

#include "Generator.h"
#include "LazyOperationType.h"
#include "Cardinal.h"

#include <stdexcept>

template <class T>
class LazySequence;

template <class T>
class LazyOperationGenerator : public Generator<T> {
private:
    LazySequence<T>* first_;
    LazySequence<T>* second_;

    LazyOperationType operation_type_;

    Cardinal first_length_;
    Cardinal second_length_;
    Cardinal result_length_;

    int index_;
    int position_;

public:
    LazyOperationGenerator(
        const LazySequence<T>& first,
        const LazySequence<T>& second,
        bool isConcat
    );

    LazyOperationGenerator(
        const LazySequence<T>& source,
        const LazySequence<T>& inserted,
        int index
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

    Cardinal GetResultCardinality() const;

    T GetAfterInfinite(int index) const;
};

#include "LazyOperationGenerator.tpp"