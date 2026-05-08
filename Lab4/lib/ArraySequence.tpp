#include "ArraySequence.h"
#include <stdexcept>


template <class T>
void ArraySequence<T>::EnsureCapacity(int required) {
    if (required <= capacity_) return;
    int newCapacity = (capacity_ == 0) ? 1 : capacity_;
    while (newCapacity < required)
        newCapacity *= 2;
    items_.Resize(newCapacity);
    capacity_ = newCapacity;
}


template <class T>
ArraySequence<T>::ArraySequence()
    : items_(0), size_(0), capacity_(0) {
}

template <class T>
ArraySequence<T>::ArraySequence(const T* items, int count)
    : items_(0), size_(0), capacity_(0) {
    if (count < 0)
        throw std::invalid_argument("Count cannot be negative");
    if (items == nullptr && count > 0)
        throw std::invalid_argument("Items cannot be null");
    EnsureCapacity(count);
    for (int i = 0; i < count; ++i)
        items_.Set(i, items[i]);
    size_ = count;
}

template <class T>
ArraySequence<T>::ArraySequence(const ArraySequence<T>& other)
    : items_(0), size_(0), capacity_(0) {
    EnsureCapacity(other.size_);
    for (int i = 0; i < other.size_; ++i)
        items_.Set(i, other.items_.Get(i));
    size_ = other.size_;
}

template <class T>
const T& ArraySequence<T>::GetFirst() const {
    if (size_ == 0) throw std::out_of_range("Sequence is empty");
    return items_.Get(0);
}

template <class T>
const T& ArraySequence<T>::GetLast() const {
    if (size_ == 0) throw std::out_of_range("Sequence is empty");
    return items_.Get(size_ - 1);
}

template <class T>
const T& ArraySequence<T>::Get(int index) const {
    if (index < 0 || index >= size_)
        throw std::out_of_range("Index is out of range");
    return items_.Get(index);
}

template <class T>
int ArraySequence<T>::GetLength() const {
    return size_;
}

template <class T>
void ArraySequence<T>::AppendInternal(const T& item) {
    EnsureCapacity(size_ + 1);
    items_.Set(size_, item);
    ++size_;
}

template <class T>
void ArraySequence<T>::PrependInternal(const T& item) {
    EnsureCapacity(size_ + 1);
    for (int i = size_; i > 0; --i)
        items_.Set(i, items_.Get(i - 1));
    items_.Set(0, item);
    ++size_;
}

template <class T>
void ArraySequence<T>::InsertAtInternal(const T& item, int index) {
    if (index < 0 || index > size_)
        throw std::out_of_range("Index is out of range");
    EnsureCapacity(size_ + 1);
    for (int i = size_; i > index; --i)
        items_.Set(i, items_.Get(i - 1));
    items_.Set(index, item);
    ++size_;
}

template <typename T>
void ArraySequence<T>::SetInternal(int index, const T& value) {
    if (index < 0 || index >= size_)
        throw std::out_of_range("Index is out of range");
    items_.Set(index, value);
}

template <typename T>
void ArraySequence<T>::RemoveAtInternal(int index) {
    if (index < 0 || index >= size_)
        throw std::out_of_range("Index is out of range");
    for (int i = index; i < size_ - 1; ++i)
        items_.Set(i, items_.Get(i + 1));
    --size_;
}


template <class T>
Sequence<T>* ArraySequence<T>::Append(const T& item) {
    ArraySequence<T>* instance = GetInstance();
    instance->AppendInternal(item);
    return instance;
}

template <class T>
Sequence<T>* ArraySequence<T>::Prepend(const T& item) {
    ArraySequence<T>* instance = GetInstance();
    instance->PrependInternal(item);
    return instance;
}

template <class T>
Sequence<T>* ArraySequence<T>::InsertAt(const T& item, int index) {
    ArraySequence<T>* instance = GetInstance();
    instance->InsertAtInternal(item, index);
    return instance;
}

template <class T>
Sequence<T>* ArraySequence<T>::GetSubsequence(int startIndex, int endIndex) const {
    if (startIndex < 0 || endIndex >= size_ || startIndex > endIndex)
        throw std::out_of_range("Invalid range");
    MutableArraySequence<T>* result = new MutableArraySequence<T>();
    for (int i = startIndex; i <= endIndex; ++i)
        result->AppendInternal(items_.Get(i));
    return result;
}

template <class T>
Sequence<T>* ArraySequence<T>::Concat(const Sequence<T>& other) const {
    MutableArraySequence<T>* result = new MutableArraySequence<T>();
    for (int i = 0; i < size_; ++i)
        result->AppendInternal(items_.Get(i));

    auto* en = other.GetEnumerator();
    while (en->MoveNext())
        result->AppendInternal(en->GetCurrent());
    delete en;

    return result;
}

template <class T>
T ArraySequence<T>::operator[](int index) const {
    return Get(index);
}

template <class T>
Sequence<T>* ArraySequence<T>::operator+(const Sequence<T>& other) const {
    return Concat(other);
}

// MutableArraySequence

template <class T>
MutableArraySequence<T>::MutableArraySequence()
    : ArraySequence<T>() {}

template <class T>
MutableArraySequence<T>::MutableArraySequence(const T* items, int count)
    : ArraySequence<T>(items, count) {}

template <class T>
MutableArraySequence<T>::MutableArraySequence(const MutableArraySequence<T>& other)
    : ArraySequence<T>(other) {}


template <typename T>
void MutableArraySequence<T>::Set(int index, const T& value) {
    this->SetInternal(index, value);
}

template <typename T>
void MutableArraySequence<T>::RemoveAt(int index) {
    this->RemoveAtInternal(index);
}

template <class T>
ArraySequence<T>* MutableArraySequence<T>::GetInstance() { return this; }

template <class T>
ArraySequence<T>* MutableArraySequence<T>::Clone() const {
    return new MutableArraySequence<T>(*this);
}

// ImmutableArraySequence

template <class T>
ImmutableArraySequence<T>::ImmutableArraySequence()
    : ArraySequence<T>() {}

template <class T>
ImmutableArraySequence<T>::ImmutableArraySequence(const T* items, int count)
    : ArraySequence<T>(items, count) {}

template <class T>
ImmutableArraySequence<T>::ImmutableArraySequence(const ImmutableArraySequence<T>& other)
    : ArraySequence<T>(other) {}

template <class T>
ArraySequence<T>* ImmutableArraySequence<T>::GetInstance() { return Clone(); }

template <class T>
ArraySequence<T>* ImmutableArraySequence<T>::Clone() const {
    return new ImmutableArraySequence<T>(*this);
}

// Builder

template <class T>
typename MutableArraySequence<T>::Builder& MutableArraySequence<T>::Builder::Append(const T& item) {
    seq_->AppendInternal(item);
    return *this;
}

template <class T>
typename MutableArraySequence<T>::Builder& MutableArraySequence<T>::Builder::AppendAll(const T* items, int count) {
    if (count < 0)
        throw std::invalid_argument("Count cannot be negative");
    if (items == nullptr && count > 0)
        throw std::invalid_argument("Items cannot be null");
    for (int i = 0; i < count; ++i)
        seq_->AppendInternal(items[i]);
    return *this;
}

template <class T>
typename MutableArraySequence<T>::Builder& MutableArraySequence<T>::Builder::AppendSequence(const Sequence<T>& other) {
    auto* en = other.GetEnumerator();
    while (en->MoveNext())
        seq_->AppendInternal(en->GetCurrent());
    delete en;
    return *this;
}

template <class T>
MutableArraySequence<T>* MutableArraySequence<T>::Builder::Build() {
    MutableArraySequence<T>* result = seq_;
    seq_ = new MutableArraySequence<T>();
    return result;
}
