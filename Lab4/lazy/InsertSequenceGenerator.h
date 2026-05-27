#pragma once

#include "Generator.h"
#include "Cardinal.h"
#include "TransfiniteIndex.h"

#include <stdexcept>


template <class T>
class LazySequence;


template <class T>
class InsertSequenceGenerator : public Generator<T> {
private:
    LazySequence<T>* source_;
    LazySequence<T>* inserted_;

    Cardinal source_length_;
    Cardinal inserted_length_;
    Cardinal result_length_;

    TransfiniteIndex index_;

    int position_;

public:
    InsertSequenceGenerator(
        const LazySequence<T>& source,
        const LazySequence<T>& inserted,
        const TransfiniteIndex& index
    );

    InsertSequenceGenerator(
        const LazySequence<T>& source,
        const LazySequence<T>& inserted,
        int index
    );

    InsertSequenceGenerator(const InsertSequenceGenerator<T>& other);

    InsertSequenceGenerator<T>& operator=(const InsertSequenceGenerator<T>& other);

    ~InsertSequenceGenerator() override;

    bool HasNext() const override;
    T GetNext() override;

    int GetPosition() const override;

    void SetSource(const Sequence<T>& source) override;

    void Reset() override;

    Generator<T>* Clone() const override;

    T GetByTransfiniteIndex(const TransfiniteIndex& index) const override;

    Cardinal GetResultCardinality() const;
};

#include "InsertSequenceGenerator.tpp"



