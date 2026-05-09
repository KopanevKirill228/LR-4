#pragma once

#include "ArraySequence.h"
#include "BinaryHeap.h"

template <class T>
BinaryHeap<T>::BinaryHeap()
    : data_(nullptr),
    count_(0),
    capacity_(0),
    has_higher_priority_(DefaultCompare)
{
    EnsureCapacity(4);
}


template <class T>
BinaryHeap<T>::BinaryHeap(bool (*compare)(const T& first, const T& second))
    : data_(nullptr),
    count_(0),
    capacity_(0),
    has_higher_priority_(compare)
{
    if (compare == nullptr) {
        throw std::invalid_argument("BinaryHeap: compare is nullptr");
    }

    EnsureCapacity(4);
}


template <class T>
BinaryHeap<T>::BinaryHeap(const T* items, int count)
    : data_(nullptr),
    count_(0),
    capacity_(0),
    has_higher_priority_(DefaultCompare)
{
    if (count < 0) {
        throw std::invalid_argument("BinaryHeap: count must be non-negative");
    }

    if (items == nullptr && count > 0) {
        throw std::invalid_argument("BinaryHeap: items is nullptr");
    }

    EnsureCapacity(count > 4 ? count : 4);

    try {
        for (int i = 0; i < count; ++i) {
            data_[i] = items[i];
        }

        count_ = count;

        for (int i = ParentIndex(count_ - 1); i >= 0; --i) {
            SiftDown(i);
        }
    }
    catch (...) {
        delete[] data_;
        data_ = nullptr;
        count_ = 0;
        capacity_ = 0;
        throw;
    }
}


template <class T>
BinaryHeap<T>::BinaryHeap(
    const T* items,
    int count,
    bool (*compare)(const T& first, const T& second))
    : data_(nullptr),
    count_(0),
    capacity_(0),
    has_higher_priority_(compare)
{
    if (compare == nullptr) {
        throw std::invalid_argument("BinaryHeap: compare is nullptr");
    }

    if (count < 0) {
        throw std::invalid_argument("BinaryHeap: count must be non-negative");
    }

    if (items == nullptr && count > 0) {
        throw std::invalid_argument("BinaryHeap: items is nullptr");
    }

    EnsureCapacity(count > 4 ? count : 4);

    try {
        for (int i = 0; i < count; ++i) {
            data_[i] = items[i];
        }

        count_ = count;

        for (int i = ParentIndex(count_ - 1); i >= 0; --i) {
            SiftDown(i);
        }
    }
    catch (...) {
        delete[] data_;
        data_ = nullptr;
        count_ = 0;
        capacity_ = 0;
        throw;
    }
}


template <class T>
BinaryHeap<T>::BinaryHeap(const BinaryHeap<T>& other)
    : data_(nullptr),
    count_(0),
    capacity_(0),
    has_higher_priority_(other.has_higher_priority_)
{
    CopyFrom(other);
}


template <class T>
BinaryHeap<T>& BinaryHeap<T>::operator=(const BinaryHeap<T>& other) {
    if (this == &other) {
        return *this;
    }

    BinaryHeap<T> temp(other);

    T* tempData = data_;
    data_ = temp.data_;
    temp.data_ = tempData;

    int tempCount = count_;
    count_ = temp.count_;
    temp.count_ = tempCount;

    int tempCapacity = capacity_;
    capacity_ = temp.capacity_;
    temp.capacity_ = tempCapacity;

    bool (*tempCompare)(const T&, const T&) = has_higher_priority_;
    has_higher_priority_ = temp.has_higher_priority_;
    temp.has_higher_priority_ = tempCompare;

    return *this;
}


template <class T>
BinaryHeap<T>::~BinaryHeap() {
    delete[] data_;
}


template <class T>
bool BinaryHeap<T>::DefaultCompare(const T& first, const T& second) {
    return first < second;
}


template <class T>
void BinaryHeap<T>::CopyFrom(const BinaryHeap<T>& other) {
    data_ = nullptr;
    count_ = 0;
    capacity_ = 0;
    has_higher_priority_ = other.has_higher_priority_;

    try {
        EnsureCapacity(other.capacity_ > 4 ? other.capacity_ : 4);

        for (int i = 0; i < other.count_; ++i) {
            data_[i] = other.data_[i];
        }

        count_ = other.count_;
    }
    catch (...) {
        delete[] data_;
        data_ = nullptr;
        count_ = 0;
        capacity_ = 0;
        throw;
    }
}


template <class T>
void BinaryHeap<T>::EnsureCapacity(int requiredCapacity) {
    if (requiredCapacity <= capacity_) {
        return;
    }

    int newCapacity = capacity_;

    if (newCapacity < 4) {
        newCapacity = 4;
    }

    while (newCapacity < requiredCapacity) {
        newCapacity *= 2;
    }

    T* newData = new T[newCapacity];

    try {
        for (int i = 0; i < count_; ++i) {
            newData[i] = data_[i];
        }
    }
    catch (...) {
        delete[] newData;
        throw;
    }

    delete[] data_;

    data_ = newData;
    capacity_ = newCapacity;
}


template <class T>
void BinaryHeap<T>::Swap(T& first, T& second) {
    T temp = first;
    first = second;
    second = temp;
}


template <class T>
int BinaryHeap<T>::ParentIndex(int index) const {
    return (index - 1) / 2;
}


template <class T>
int BinaryHeap<T>::LeftChildIndex(int index) const {
    return index * 2 + 1;
}


template <class T>
int BinaryHeap<T>::RightChildIndex(int index) const {
    return index * 2 + 2;
}


template <class T>
void BinaryHeap<T>::SiftUp(int index) {
    while (index > 0) {
        int parent = ParentIndex(index);

        if (!has_higher_priority_(data_[index], data_[parent])) {
            break;
        }

        Swap(data_[index], data_[parent]);

        index = parent;
    }
}


template <class T>
void BinaryHeap<T>::SiftDown(int index) {
    while (true) {
        int left = LeftChildIndex(index);
        int right = RightChildIndex(index);
        int best = index;

        if (left < count_ && has_higher_priority_(data_[left], data_[best])) {
            best = left;
        }

        if (right < count_ && has_higher_priority_(data_[right], data_[best])) {
            best = right;
        }

        if (best == index) {
            break;
        }

        Swap(data_[index], data_[best]);

        index = best;
    }
}


template <class T>
void BinaryHeap<T>::Push(const T& item) {
    EnsureCapacity(count_ + 1);

    data_[count_] = item;
    SiftUp(count_);
    ++count_;
}


template <class T>
T BinaryHeap<T>::Pop() {
    if (count_ == 0) {
        throw std::out_of_range("BinaryHeap is empty");
    }

    T result = data_[0];

    data_[0] = data_[count_ - 1];
    --count_;

    if (count_ > 0) {
        SiftDown(0);
    }

    return result;
}


template <class T>
const T& BinaryHeap<T>::Peek() const {
    if (count_ == 0) {
        throw std::out_of_range("BinaryHeap is empty");
    }

    return data_[0];
}


template <class T>
int BinaryHeap<T>::GetCount() const {
    return count_;
}


template <class T>
bool BinaryHeap<T>::IsEmpty() const {
    return count_ == 0;
}


template <class T>
void BinaryHeap<T>::Clear() {
    count_ = 0;
}


template <class T>
Sequence<T>* BinaryHeap<T>::ToSequence() const {
    Sequence<T>* result = new MutableArraySequence<T>();

    try {
        for (int i = 0; i < count_; ++i) {
            Sequence<T>* old = result;
            Sequence<T>* next = result->Append(data_[i]);

            if (next == nullptr) {
                throw std::runtime_error("Append returned nullptr");
            }

            if (next != old) {
                delete old;
                result = next;
            }
        }

        return result;
    }
    catch (...) {
        delete result;
        throw;
    }
}