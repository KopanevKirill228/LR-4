#pragma once

#include "IEnumerator.h"
#include <stdexcept>

template <class T>
class DynamicArray {
public:
    DynamicArray();
    DynamicArray(int size);
    DynamicArray(const T* items, int count);
    DynamicArray(const DynamicArray<T>& other);

    DynamicArray<T>& operator=(const DynamicArray<T>& other);

    ~DynamicArray();

    const T& Get(int index) const;
    int GetSize() const;
    void Set(int index, const T& value);
    void Resize(int newSize);

    T& operator[](int index);
    const T& operator[](int index) const;

    class Enumerator : public IEnumerator<T> {
    private:
        const DynamicArray<T>* array_;
        int index_;

    public:
        Enumerator(const DynamicArray<T>* array) : array_(array), index_(-1) {}

        bool MoveNext() override {
            index_++;
            return index_ < array_->GetSize();
        }

        const T& GetCurrent() const override {
            if (index_ < 0 || index_ >= array_->GetSize()) {
                throw std::out_of_range("Enumerator is out of range");
            }

            return array_->Get(index_);
        }

        void Reset() override {
            index_ = -1;
        }
    };

    IEnumerator<T>* GetEnumerator() const {
        return new Enumerator(this);
    }

private:
    T* data_;
    int size_;

    void CheckIndex(int index) const;
};

#include "DynamicArray.tpp"
