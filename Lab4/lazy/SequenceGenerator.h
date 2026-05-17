#pragma once

#include "Generator.h"

#include <stdexcept>


template <class T>
class SequenceGenerator : public Generator<T> {
private:
    const Sequence<T>* source_;
    int position_;

public:
    SequenceGenerator(const Sequence<T>& source);

    SequenceGenerator(const SequenceGenerator<T>& other);

    SequenceGenerator<T>& operator=(const SequenceGenerator<T>& other);

    bool HasNext() const override;
    T GetNext() override;

    int GetPosition() const override;

    void SetSource(const Sequence<T>& source) override;

    void Reset() override;

    Generator<T>* Clone() const override;
};

#include "SequenceGenerator.tpp"