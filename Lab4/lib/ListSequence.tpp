#include "ListSequence.h"
#include <stdexcept>


template <class T>
ListSequence<T>::ListSequence()
    : items_() {
}

template <class T>
ListSequence<T>::ListSequence(const T* items, int count) {
    if (count < 0)
        throw std::invalid_argument("Count cannot be negative");
    if (items == nullptr && count > 0)
        throw std::invalid_argument("Items cannot be null");
    items_ = LinkedList<T>(items, count);
}

template <class T>
ListSequence<T>::ListSequence(const LinkedList<T>& list)
    : items_(list) {
}

template <class T>
ListSequence<T>::ListSequence(const ListSequence<T>& other)
    : items_(other.items_) {
}

template <class T>
const T& ListSequence<T>::GetFirst() const {
    return items_.GetFirst();
}

template <class T>
const T& ListSequence<T>::GetLast() const {
    return items_.GetLast();
}

template <class T>
const T& ListSequence<T>::Get(int index) const {
    return items_.Get(index);
}


template <class T>
int ListSequence<T>::GetLength() const {
    return items_.GetLength();
}

template <class T>
void ListSequence<T>::AppendInternal(const T& item) {
    items_.Append(item);
}

template <class T>
void ListSequence<T>::PrependInternal(const T& item) {
    items_.Prepend(item);
}

template <class T>
void ListSequence<T>::InsertAtInternal(const T& item, int index) {
    items_.InsertAt(item, index);
}

template <class T>
Sequence<T>* ListSequence<T>::Append(const T& item) {
    ListSequence<T>* instance = GetInstance();
    instance->AppendInternal(item);
    return instance;
}

template <class T>
Sequence<T>* ListSequence<T>::Prepend(const T& item) {
    ListSequence<T>* instance = GetInstance();
    instance->PrependInternal(item);
    return instance;
}

template <class T>
Sequence<T>* ListSequence<T>::InsertAt(const T& item, int index) {
    ListSequence<T>* instance = GetInstance();
    instance->InsertAtInternal(item, index);
    return instance;
}

template <class T>
Sequence<T>* ListSequence<T>::GetSubsequence(int startIndex, int endIndex) const {
    if (startIndex < 0 || endIndex < 0 ||
        endIndex >= GetLength() || startIndex > endIndex)
        throw std::out_of_range("Invalid subsequence range");
    LinkedList<T> subList = items_.GetSubList(startIndex, endIndex);
    return new MutableListSequence<T>(subList);
}

template <class T>
Sequence<T>* ListSequence<T>::Concat(const Sequence<T>& other) const {
    MutableListSequence<T>* result = new MutableListSequence<T>();

    auto* en1 = this->GetEnumerator();
    while (en1->MoveNext())
        result->AppendInternal(en1->GetCurrent());
    delete en1;

    auto* en2 = other.GetEnumerator();
    while (en2->MoveNext())
        result->AppendInternal(en2->GetCurrent());
    delete en2;

    return result;
}

template <class T>
T ListSequence<T>::operator[](int index) const {
    return items_.Get(index);
}

template <class T>
Sequence<T>* ListSequence<T>::operator+(const Sequence<T>& other) const {
    return Concat(other);
}

// MutableListSequence

template <class T>
MutableListSequence<T>::MutableListSequence()
    : ListSequence<T>() {
}

template <class T>
MutableListSequence<T>::MutableListSequence(const T* items, int count)
    : ListSequence<T>(items, count) {
}

template <class T>
MutableListSequence<T>::MutableListSequence(const LinkedList<T>& list)
    : ListSequence<T>(list) {
}


template <class T>
MutableListSequence<T>::MutableListSequence(const MutableListSequence<T>& other)
    : ListSequence<T>(other) {
}

template <typename T>
void MutableListSequence<T>::RemoveFirst() {
    this->items_.PopFront();
}

template <class T>
ListSequence<T>* MutableListSequence<T>::GetInstance() {
    return this;
}

template <class T>
ListSequence<T>* MutableListSequence<T>::Clone() const {
    return new MutableListSequence<T>(*this);
}

// ImmutableListSequence

template <class T>
ImmutableListSequence<T>::ImmutableListSequence()
    : ListSequence<T>() {
}

template <class T>
ImmutableListSequence<T>::ImmutableListSequence(const T* items, int count)
    : ListSequence<T>(items, count) {
}

template <class T>
ImmutableListSequence<T>::ImmutableListSequence(const LinkedList<T>& list)
    : ListSequence<T>(list) {
}

template <class T>
ImmutableListSequence<T>::ImmutableListSequence(const ImmutableListSequence<T>& other)
    : ListSequence<T>(other) {
}

template <class T>
ListSequence<T>* ImmutableListSequence<T>::GetInstance() {
    return Clone();
}

template <class T>
ListSequence<T>* ImmutableListSequence<T>::Clone() const {
    return new ImmutableListSequence<T>(*this);
}

// Builder

template <class T>
typename MutableListSequence<T>::Builder&
MutableListSequence<T>::Builder::Append(const T& item) {
    seq_->Append(item);
    return *this;
}

template <class T>
typename MutableListSequence<T>::Builder&
MutableListSequence<T>::Builder::AppendAll(const T* items, int count) {
    if (count < 0)
        throw std::invalid_argument("Count cannot be negative");
    if (items == nullptr && count > 0)
        throw std::invalid_argument("Items cannot be null");
    for (int i = 0; i < count; i++)
        seq_->Append(items[i]);
    return *this;
}

template <class T>
typename MutableListSequence<T>::Builder&
MutableListSequence<T>::Builder::AppendSequence(const Sequence<T>& other) {
    auto* en = other.GetEnumerator();
    while (en->MoveNext())
        seq_->Append(en->GetCurrent());
    delete en;
    return *this;
}

template <class T>
MutableListSequence<T>*
MutableListSequence<T>::Builder::Build() {
    MutableListSequence<T>* result = seq_;
    seq_ = new MutableListSequence<T>();
    return result;
}
