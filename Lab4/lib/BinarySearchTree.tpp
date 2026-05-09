#pragma once

#include "ArraySequence.h"
#include "BinarySearchTree.h"

template <class T>
BinarySearchTree<T>::BinarySearchTree()
    : root_(nullptr), count_(0) {
}


template <class T>
BinarySearchTree<T>::BinarySearchTree(const BinarySearchTree<T>& other)
    : root_(nullptr), count_(0) {
    try {
        root_ = CopyTree(other.root_);
        count_ = other.count_;
    }
    catch (...) {
        DeleteTree(root_);
        root_ = nullptr;
        count_ = 0;
        throw;
    }
}


template <class T>
BinarySearchTree<T>& BinarySearchTree<T>::operator=(const BinarySearchTree<T>& other) {
    if (this == &other) {
        return *this;
    }

    Node* newRoot = nullptr;

    try {
        newRoot = CopyTree(other.root_);
    }
    catch (...) {
        DeleteTree(newRoot);
        throw;
    }

    DeleteTree(root_);

    root_ = newRoot;
    count_ = other.count_;

    return *this;
}


template <class T>
BinarySearchTree<T>::~BinarySearchTree() {
    DeleteTree(root_);
}


template <class T>
typename BinarySearchTree<T>::Node* BinarySearchTree<T>::CopyTree(const Node* node) {
    if (node == nullptr) {
        return nullptr;
    }

    Node* newNode = nullptr;

    try {
        newNode = new Node(node->data);
        newNode->left = CopyTree(node->left);
        newNode->right = CopyTree(node->right);

        return newNode;
    }
    catch (...) {
        DeleteTree(newNode);
        throw;
    }
}


template <class T>
void BinarySearchTree<T>::DeleteTree(Node* node) {
    if (node == nullptr) {
        return;
    }

    DeleteTree(node->left);
    DeleteTree(node->right);

    delete node;
}


template <class T>
void BinarySearchTree<T>::Insert(const T& item) {
    bool inserted = false;

    root_ = InsertNode(root_, item, inserted);

    if (inserted) {
        ++count_;
    }
}


template <class T>
typename BinarySearchTree<T>::Node* BinarySearchTree<T>::InsertNode(
    Node* node,
    const T& item,
    bool& inserted)
{
    if (node == nullptr) {
        inserted = true;
        return new Node(item);
    }

    if (item < node->data) {
        node->left = InsertNode(node->left, item, inserted);
    }
    else if (node->data < item) {
        node->right = InsertNode(node->right, item, inserted);
    }
    else {
        inserted = false;
    }

    return node;
}


template <class T>
bool BinarySearchTree<T>::Remove(const T& item) {
    bool removed = false;

    root_ = RemoveNode(root_, item, removed);

    if (removed) {
        --count_;
    }

    return removed;
}


template <class T>
typename BinarySearchTree<T>::Node* BinarySearchTree<T>::RemoveNode(
    Node* node,
    const T& item,
    bool& removed)
{
    if (node == nullptr) {
        removed = false;
        return nullptr;
    }

    if (item < node->data) {
        node->left = RemoveNode(node->left, item, removed);
        return node;
    }

    if (node->data < item) {
        node->right = RemoveNode(node->right, item, removed);
        return node;
    }

    removed = true;

    if (node->left == nullptr && node->right == nullptr) {
        delete node;
        return nullptr;
    }

    if (node->left == nullptr) {
        Node* rightChild = node->right;
        delete node;
        return rightChild;
    }

    if (node->right == nullptr) {
        Node* leftChild = node->left;
        delete node;
        return leftChild;
    }

    Node* minRight = FindMin(node->right);
    node->data = minRight->data;

    bool tempRemoved = false;
    node->right = RemoveNode(node->right, minRight->data, tempRemoved);

    return node;
}


template <class T>
typename BinarySearchTree<T>::Node* BinarySearchTree<T>::FindMin(Node* node) const {
    if (node == nullptr) {
        return nullptr;
    }

    Node* current = node;

    while (current->left != nullptr) {
        current = current->left;
    }

    return current;
}


template <class T>
const typename BinarySearchTree<T>::Node* BinarySearchTree<T>::FindNode(
    const Node* node,
    const T& item) const
{
    const Node* current = node;

    while (current != nullptr) {
        if (item < current->data) {
            current = current->left;
        }
        else if (current->data < item) {
            current = current->right;
        }
        else {
            return current;
        }
    }

    return nullptr;
}


template <class T>
bool BinarySearchTree<T>::Contains(const T& item) const {
    return FindNode(root_, item) != nullptr;
}


template <class T>
const T& BinarySearchTree<T>::GetMin() const {
    if (root_ == nullptr) {
        throw std::out_of_range("BinarySearchTree is empty");
    }

    Node* minNode = FindMin(root_);

    return minNode->data;
}


template <class T>
const T& BinarySearchTree<T>::GetMax() const {
    if (root_ == nullptr) {
        throw std::out_of_range("BinarySearchTree is empty");
    }

    Node* current = root_;

    while (current->right != nullptr) {
        current = current->right;
    }

    return current->data;
}


template <class T>
int BinarySearchTree<T>::GetCount() const {
    return count_;
}


template <class T>
bool BinarySearchTree<T>::IsEmpty() const {
    return count_ == 0;
}


template <class T>
int BinarySearchTree<T>::GetHeight() const {
    return Height(root_);
}


template <class T>
int BinarySearchTree<T>::Height(Node* node) const {
    if (node == nullptr) {
        return 0;
    }

    int leftHeight = Height(node->left);
    int rightHeight = Height(node->right);

    if (leftHeight > rightHeight) {
        return leftHeight + 1;
    }

    return rightHeight + 1;
}


template <class T>
void BinarySearchTree<T>::Clear() {
    DeleteTree(root_);

    root_ = nullptr;
    count_ = 0;
}


template <class T>
void BinarySearchTree<T>::AppendToSequence(Sequence<T>*& seq, const T& item) const {
    Sequence<T>* old = seq;
    Sequence<T>* next = seq->Append(item);

    if (next == nullptr) {
        throw std::runtime_error("Append returned nullptr");
    }

    if (next != old) {
        delete old;
        seq = next;
    }
}


template <class T>
void BinarySearchTree<T>::InOrder(Node* node, Sequence<T>*& result) const {
    if (node == nullptr) {
        return;
    }

    InOrder(node->left, result);
    AppendToSequence(result, node->data);
    InOrder(node->right, result);
}


template <class T>
void BinarySearchTree<T>::PreOrder(Node* node, Sequence<T>*& result) const {
    if (node == nullptr) {
        return;
    }

    AppendToSequence(result, node->data);
    PreOrder(node->left, result);
    PreOrder(node->right, result);
}


template <class T>
void BinarySearchTree<T>::PostOrder(Node* node, Sequence<T>*& result) const {
    if (node == nullptr) {
        return;
    }

    PostOrder(node->left, result);
    PostOrder(node->right, result);
    AppendToSequence(result, node->data);
}


template <class T>
Sequence<T>* BinarySearchTree<T>::ToSequenceInOrder() const {
    Sequence<T>* result = new MutableArraySequence<T>();

    try {
        InOrder(root_, result);
        return result;
    }
    catch (...) {
        delete result;
        throw;
    }
}


template <class T>
Sequence<T>* BinarySearchTree<T>::ToSequencePreOrder() const {
    Sequence<T>* result = new MutableArraySequence<T>();

    try {
        PreOrder(root_, result);
        return result;
    }
    catch (...) {
        delete result;
        throw;
    }
}


template <class T>
Sequence<T>* BinarySearchTree<T>::ToSequencePostOrder() const {
    Sequence<T>* result = new MutableArraySequence<T>();

    try {
        PostOrder(root_, result);
        return result;
    }
    catch (...) {
        delete result;
        throw;
    }
}