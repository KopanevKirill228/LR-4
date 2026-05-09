#pragma once

#include "Sequence.h"

#include <stdexcept>


template <class T>
class BinarySearchTree {
private:
    struct Node {
        T data;
        Node* left;
        Node* right;

        Node(const T& value)
            : data(value), left(nullptr), right(nullptr) {
        }
    };

    Node* root_;
    int count_;

    Node* CopyTree(const Node* node);
    void DeleteTree(Node* node);

    Node* InsertNode(Node* node, const T& item, bool& inserted);
    Node* RemoveNode(Node* node, const T& item, bool& removed);

    Node* FindMin(Node* node) const;
    const Node* FindNode(const Node* node, const T& item) const;

    void InOrder(Node* node, Sequence<T>*& result) const;
    void PreOrder(Node* node, Sequence<T>*& result) const;
    void PostOrder(Node* node, Sequence<T>*& result) const;

    int Height(Node* node) const;

    void AppendToSequence(Sequence<T>*& seq, const T& item) const;

public:
    BinarySearchTree();
    BinarySearchTree(const BinarySearchTree<T>& other);

    BinarySearchTree<T>& operator=(const BinarySearchTree<T>& other);

    ~BinarySearchTree();

    void Insert(const T& item);
    bool Remove(const T& item);
    bool Contains(const T& item) const;

    const T& GetMin() const;
    const T& GetMax() const;

    int GetCount() const;
    bool IsEmpty() const;
    int GetHeight() const;

    void Clear();

    Sequence<T>* ToSequenceInOrder() const;
    Sequence<T>* ToSequencePreOrder() const;
    Sequence<T>* ToSequencePostOrder() const;
};

#include "BinarySearchTree.tpp"