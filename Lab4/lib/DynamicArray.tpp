#include <stdexcept>
#include "DynamicArray.h"


template <class T>
DynamicArray<T>::DynamicArray()
    : data_(nullptr), size_(0) {
}

template <class T>
DynamicArray<T>::DynamicArray(int size) {
    if (size < 0)
        throw std::invalid_argument("Size cannot be negative");
    size_ = size;
    data_ = (size_ > 0) ? new T[size_] : nullptr;
}

template <class T>
DynamicArray<T>::DynamicArray(const T* items, int count) {
    if (count < 0)
        throw std::invalid_argument("Count cannot be negative");
    if (items == nullptr && count > 0)
        throw std::invalid_argument("Items cannot be null");
    size_ = count;
    data_ = (size_ > 0) ? new T[size_] : nullptr;
    for (int i = 0; i < size_; ++i)
        data_[i] = items[i];
}

template <class T>
DynamicArray<T>::DynamicArray(const DynamicArray<T>& other)
    : size_(other.size_) {
    data_ = (size_ > 0) ? new T[size_] : nullptr;
    for (int i = 0; i < size_; ++i)
        data_[i] = other.data_[i];
}

template <class T>
DynamicArray<T>& DynamicArray<T>::operator=(const DynamicArray<T>& other) {
    if (this != &other) {
        T* newData = (other.size_ > 0) ? new T[other.size_] : nullptr;
        for (int i = 0; i < other.size_; ++i)
            newData[i] = other.data_[i];
        delete[] data_;
        data_ = newData;
        size_ = other.size_;
    }
    return *this;
}

template <class T>
DynamicArray<T>::~DynamicArray() {
    delete[] data_;
}

template <class T>
void DynamicArray<T>::CheckIndex(int index) const {
    if (index < 0 || index >= size_)
        throw std::out_of_range("Index is out of range");
}

template <class T>
const T& DynamicArray<T>::Get(int index) const {
    CheckIndex(index);
    return data_[index];
}

template <class T>
int DynamicArray<T>::GetSize() const {
    return size_;
}

template <class T>
void DynamicArray<T>::Set(int index, const T& value) {
    CheckIndex(index);
    data_[index] = value;
}

template <class T>
void DynamicArray<T>::Resize(int newSize) {
    if (newSize < 0)
        throw std::invalid_argument("New size cannot be negative");
    T* newData = (newSize > 0) ? new T[newSize] : nullptr;
    int toCopy = (newSize < size_) ? newSize : size_;
    for (int i = 0; i < toCopy; ++i)
        newData[i] = data_[i];
    delete[] data_;
    data_ = newData;
    size_ = newSize;
}


template <class T>
T& DynamicArray<T>::operator[](int index) {
    CheckIndex(index);
    return data_[index];
}

template <class T>
const T& DynamicArray<T>::operator[](int index) const {
    CheckIndex(index);
    return data_[index];
}
