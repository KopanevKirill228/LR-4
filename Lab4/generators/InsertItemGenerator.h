#pragma once

#include "Generator.h"
#include "../lazy/Cardinal.h"
#include "../lazy/TransfiniteIndex.h"

#include <stdexcept>


template <class T>
class LazySequence;


template <class T>
class InsertItemGenerator : public Generator<T> {
private:
    LazySequence<T>* source_;
    T* item_;

    Cardinal source_length_;
    Cardinal result_length_;

    int index_;
    int position_;

public:
    InsertItemGenerator(
        const LazySequence<T>& source,
        const T& item,
        int index
    );

    InsertItemGenerator(const InsertItemGenerator<T>& other);

    InsertItemGenerator<T>& operator=(const InsertItemGenerator<T>& other);

    ~InsertItemGenerator() override;

    bool HasNext() const override;
    T GetNext() override;

    int GetPosition() const override;

    void SetSource(const Sequence<T>& source) override;

    void Reset() override;

    Generator<T>* Clone() const override;

    T GetByTransfiniteIndex(const TransfiniteIndex& index) const override;

    T GetAfterInfinite(int index) const override;

    Cardinal GetResultCardinality() const override;

    TransfiniteLength GetResultLength() const override;
};

#include "InsertItemGenerator.tpp"