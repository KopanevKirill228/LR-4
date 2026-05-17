#pragma once

#include "../lib/Sequence.h"
#include "../lib/ArraySequence.h"
#include "../lib/IEnumerator.h"

#include "Generator.h"
#include "RuleGenerator.h"
#include "SequenceGenerator.h"
#include "Cardinal.h"

#include <functional>
#include <stdexcept>


template <class T>
class LazySequence : public Sequence<T> {
private:
    mutable MutableArraySequence<T> materialized_;
    Generator<T>* generator_;

    bool is_infinite_;
    int finite_length_;

    LazySequence(Generator<T>* generator, bool isInfinite, int finiteLength);

    void CheckIndex(int index) const;
    void MaterializeTo(int index) const;
    void AppendMaterialized(const T& item) const;

public:
    LazySequence();
    LazySequence(const T* items, int count);
    LazySequence(const Sequence<T>& seq);

    LazySequence(
        std::function<T(const Sequence<T>&)> rule,
        const Sequence<T>& initialValues
    );

    LazySequence(const LazySequence<T>& other);

    LazySequence<T>& operator=(const LazySequence<T>& other);

    ~LazySequence();

    const T& GetFirst() const override;
    const T& GetLast() const override;
    const T& Get(int index) const override;
    int GetLength() const override;
    Cardinal GetCardinality() const;

    int GetMaterializedCount() const;
    bool IsInfinite() const;

    Sequence<T>* GetSubsequence(int startIndex, int endIndex) const override;

    Sequence<T>* Append(const T& item) override;
    Sequence<T>* Prepend(const T& item) override;
    Sequence<T>* InsertAt(const T& item, int index) override;
    Sequence<T>* Concat(const Sequence<T>& other) const override;

    T operator[](int index) const override;
    Sequence<T>* operator+(const Sequence<T>& other) const override;

    class Enumerator : public IEnumerator<T> {
    private:
        const LazySequence<T>* sequence_;
        int index_;
        bool is_valid_;

    public:
        Enumerator(const LazySequence<T>* sequence)
            : sequence_(sequence), index_(-1), is_valid_(false)
        {
            if (sequence_ == nullptr) {
                throw std::invalid_argument("LazySequence Enumerator: sequence is nullptr");
            }
        }

        bool MoveNext() override {
            if (!sequence_->IsInfinite()) {
                if (index_ + 1 >= sequence_->GetLength()) {
                    is_valid_ = false;
                    return false;
                }
            }

            ++index_;
            is_valid_ = true;

            return true;
        }

        const T& GetCurrent() const override {
            if (!is_valid_ || index_ < 0) {
                throw std::out_of_range("LazySequence Enumerator is out of range");
            }

            return sequence_->Get(index_);
        }

        void Reset() override {
            index_ = -1;
            is_valid_ = false;
        }
    };

    IEnumerator<T>* GetEnumerator() const override;
};

#include "LazySequence.tpp"