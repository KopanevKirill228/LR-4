#pragma once

#include "Generator.h"
#include "../lazy/Cardinal.h"
#include "../lazy/TransfiniteIndex.h"

#include <stdexcept>


template <class T>
class LazySequence;


template <class T>
class ConcatGenerator : public Generator<T> {
private:
    LazySequence<T>* first_;
    LazySequence<T>* second_;

    Cardinal first_length_;
    Cardinal second_length_;
    Cardinal result_length_;

    int position_;

public:
    ConcatGenerator(
        const LazySequence<T>& first,
        const LazySequence<T>& second
    );

    ConcatGenerator(const ConcatGenerator<T>& other);

    ConcatGenerator<T>& operator=(const ConcatGenerator<T>& other);

    ~ConcatGenerator() override;

    bool HasNext() const override;
    T GetNext() override;

    int GetPosition() const override;

    void SetSource(const Sequence<T>& source) override;

    void Reset() override;

    Generator<T>* Clone() const override;

    T GetByTransfiniteIndex(const TransfiniteIndex& index) const override;

    Cardinal GetResultCardinality() const override;
};

#include "ConcatGenerator.tpp"