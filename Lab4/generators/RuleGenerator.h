#pragma once

#include "Generator.h"

#include <stdexcept>


template <class T>
class RuleGenerator : public Generator<T> {
private:
    T(*rule_)(const Sequence<T>&);
    const Sequence<T>* source_;
    int position_;

public:
    RuleGenerator(
        T(*rule)(const Sequence<T>&),
        const Sequence<T>& source
    );

    RuleGenerator(const RuleGenerator<T>& other);

    RuleGenerator<T>& operator=(const RuleGenerator<T>& other);

    bool HasNext() const override;
    T GetNext() override;

    int GetPosition() const override;

    void SetSource(const Sequence<T>& source) override;

    void Reset() override;

    Generator<T>* Clone() const override;

    Cardinal GetResultCardinality() const override;

    TransfiniteLength GetResultLength() const override;
};

#include "RuleGenerator.tpp"