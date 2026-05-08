#pragma once

#include "Sequence.h"
#include "DynamicArray.h"
#include "IEnumerator.h"
#include <stdexcept>

template <class T>
class ArraySequence : public Sequence<T> {
public:
    ArraySequence();
    ArraySequence(const T* items, int count);
    ArraySequence(const ArraySequence<T>& other);
    virtual ~ArraySequence() = default;

    const T& GetFirst() const override;
    const T& GetLast() const override;
    const T& Get(int index) const override;
    int GetLength() const override;

    Sequence<T>* GetSubsequence(int startIndex, int endIndex) const override;
    Sequence<T>* Append(const T& item) override;
    Sequence<T>* Prepend(const T& item) override;
    Sequence<T>* InsertAt(const T& item, int index) override;
    Sequence<T>* Concat(const Sequence<T>& other) const override;

    T operator[](int index) const override;
    Sequence<T>* operator+(const Sequence<T>& other) const override;

private:
    class Enumerator : public IEnumerator<T> {
    private:
        const ArraySequence<T>* seq_;
        int index_;
    public:
        Enumerator(const ArraySequence<T>* seq) : seq_(seq), index_(-1) {}

        bool MoveNext() override {
            ++index_;
            return index_ < seq_->GetLength();
        }

        const T& GetCurrent() const override {
            if (index_ < 0 || index_ >= seq_->GetLength())
                throw std::out_of_range("Enumerator is out of range");
            return seq_->Get(index_);
        }

        void Reset() override { index_ = -1; }
    };

public:
    IEnumerator<T>* GetEnumerator() const override {
        return new Enumerator(this);
    }

protected:
    virtual ArraySequence<T>* GetInstance() = 0;
    virtual ArraySequence<T>* Clone() const = 0;

    void AppendInternal(const T& item);
    void PrependInternal(const T& item);
    void InsertAtInternal(const T& item, int index);
    void SetInternal(int index, const T& value);
    void RemoveAtInternal(int index);

private:
    DynamicArray<T> items_;
    int size_;
    int capacity_;

    void EnsureCapacity(int required);
};


template <class T>
class MutableArraySequence : public ArraySequence<T> {
public:
    MutableArraySequence();
    MutableArraySequence(const T* items, int count);
    MutableArraySequence(const MutableArraySequence<T>& other);

    void Set(int index, const T& value);
    void RemoveAt(int index);

    class Builder {
    private:
        MutableArraySequence<T>* seq_;
    public:
        Builder() : seq_(new MutableArraySequence<T>()) {}
        ~Builder() { delete seq_; }

        Builder& Append(const T& item);
        Builder& AppendAll(const T* items, int count);
        Builder& AppendSequence(const Sequence<T>& other);
        MutableArraySequence<T>* Build();
    };

protected:
    ArraySequence<T>* GetInstance() override;
    ArraySequence<T>* Clone() const override;
};


template <class T>
class ImmutableArraySequence : public ArraySequence<T> {
public:
    ImmutableArraySequence();
    ImmutableArraySequence(const T* items, int count);
    ImmutableArraySequence(const ImmutableArraySequence<T>& other);

protected:
    ArraySequence<T>* GetInstance() override;
    ArraySequence<T>* Clone() const override;
};

#include "ArraySequence.tpp"
