#include <stdexcept>
#include "LinkedList.h"

template <class T>
struct LinkedList<T>::Node {
    T data;
    Node* next;
    Node(const T& value, Node* nextNode = nullptr)
        : data(value), next(nextNode) {
    }
};

template <class T>
LinkedList<T>::LinkedList()
    : head_(nullptr), tail_(nullptr), length_(0) {
}

template <class T>
LinkedList<T>::LinkedList(const T* items, int count)
    : head_(nullptr), tail_(nullptr), length_(0) {
    if (count < 0)
        throw std::invalid_argument("Count cannot be negative");
    if (items == nullptr && count > 0)
        throw std::invalid_argument("Items cannot be null");
    for (int i = 0; i < count; ++i)
        Append(items[i]);
}

template <class T>
LinkedList<T>::LinkedList(const LinkedList<T>& other)
    : head_(nullptr), tail_(nullptr), length_(0) {
    Node* current = other.head_;
    while (current != nullptr) {
        Append(current->data);
        current = current->next;
    }
}

template <class T>
LinkedList<T>& LinkedList<T>::operator=(const LinkedList<T>& other) {
    if (this != &other) {
        Clear();
        Node* current = other.head_;
        while (current != nullptr) {
            Append(current->data);
            current = current->next;
        }
    }
    return *this;
}

template <class T>
LinkedList<T>::~LinkedList() {
    Clear();
}

template <class T>
void LinkedList<T>::CheckIndex(int index) const {
    if (index < 0 || index >= length_)
        throw std::out_of_range("Index is out of range");
}

template <class T>
typename LinkedList<T>::Node* LinkedList<T>::GetNode(int index) const {
    CheckIndex(index);
    Node* current = head_;
    for (int i = 0; i < index; ++i)
        current = current->next;
    return current;
}

template <class T>
void LinkedList<T>::Clear() {
    Node* current = head_;
    while (current != nullptr) {
        Node* next = current->next;
        delete current;
        current = next;
    }
    head_ = nullptr;
    tail_ = nullptr;
    length_ = 0;
}

template <class T>
const T& LinkedList<T>::GetFirst() const {
    if (length_ == 0)
        throw std::out_of_range("List is empty");
    return head_->data;
}

template <class T>
const T& LinkedList<T>::GetLast() const {
    if (length_ == 0)
        throw std::out_of_range("List is empty");
    return tail_->data;
}

template <class T>
const T& LinkedList<T>::Get(int index) const {
    return GetNode(index)->data;
}

template <class T>
LinkedList<T> LinkedList<T>::GetSubList(int startIndex, int endIndex) const {
    if (startIndex < 0 || endIndex < 0 ||
        endIndex >= length_ || startIndex > endIndex)
        throw std::out_of_range("Invalid range");
    LinkedList<T> result;
    Node* current = GetNode(startIndex);
    for (int i = startIndex; i <= endIndex; ++i) {
        result.Append(current->data);
        current = current->next;
    }
    return result;
}

template <class T>
int LinkedList<T>::GetLength() const {
    return length_;
}

template <class T>
void LinkedList<T>::Append(const T& item) {
    Node* newNode = new Node(item);
    if (length_ == 0) {
        head_ = tail_ = newNode;
    }
    else {
        tail_->next = newNode;
        tail_ = newNode;
    }
    ++length_;
}

template <class T>
void LinkedList<T>::Prepend(const T& item) {
    Node* newNode = new Node(item, head_);
    head_ = newNode;
    if (length_ == 0)
        tail_ = newNode;
    ++length_;
}

template <class T>
void LinkedList<T>::InsertAt(const T& item, int index) {
    if (index < 0 || index > length_)
        throw std::out_of_range("Index is out of range");

    Node** current = &head_;

    for (int i = 0; i < index; ++i)
        current = &(*current)->next;

    *current = new Node(item, *current);

    if ((*current)->next == nullptr)
        tail_ = *current;

    ++length_;
}

template <class T>
void LinkedList<T>::PopFront() {
    if (length_ == 0)
        throw std::out_of_range("List is empty");
    Node* old = head_;
    head_ = head_->next;
    if (head_ == nullptr) tail_ = nullptr;
    delete old;
    --length_;
}

template <class T>
LinkedList<T> LinkedList<T>::Concat(const LinkedList<T>& list) const {
    LinkedList<T> result(*this);
    Node* current = list.head_;
    while (current != nullptr) {
        result.Append(current->data);
        current = current->next;
    }
    return result;
}

template <class T>
T LinkedList<T>::operator[](int index) const {
    return Get(index);
}
