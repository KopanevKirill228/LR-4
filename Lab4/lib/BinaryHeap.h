#pragma once

#include "Sequence.h"

#include <stdexcept>


template <class T>
class BinaryHeap {
private:
    T* data_;
    int count_;
    int capacity_;

    bool (*has_higher_priority_)(const T& first, const T& second);

    static bool DefaultCompare(const T& first, const T& second);

    void EnsureCapacity(int requiredCapacity);
    void Swap(T& first, T& second);

    int ParentIndex(int index) const;
    int LeftChildIndex(int index) const;
    int RightChildIndex(int index) const;

    void SiftUp(int index);
    void SiftDown(int index);

    void CopyFrom(const BinaryHeap<T>& other);

public:
    BinaryHeap();
    BinaryHeap(bool (*compare)(const T& first, const T& second));
    BinaryHeap(const T* items, int count);
    BinaryHeap(
        const T* items,
        int count,
        bool (*compare)(const T& first, const T& second)
    );

    BinaryHeap(const BinaryHeap<T>& other);

    BinaryHeap<T>& operator=(const BinaryHeap<T>& other);

    ~BinaryHeap();

    void Push(const T& item);
    T Pop();
    const T& Peek() const;

    int GetCount() const;
    bool IsEmpty() const;

    void Clear();

    Sequence<T>* ToSequence() const;
};

#include "BinaryHeap.tpp"