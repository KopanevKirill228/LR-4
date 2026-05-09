#include <iostream>
#include <stdexcept>

#include "lib/BinarySearchTree.h"
#include "lib/BinaryHeap.h"
#include "lib/Sequence.h"


static int total = 0;
static int failed = 0;


void Check(const char* name, bool expr) {
    ++total;

    if (expr) {
        std::cout << "  [PASS] " << name << "\n";
    }
    else {
        ++failed;
        std::cout << "  [FAIL] " << name << "\n";
    }
}


template <class F>
void CheckThrows(const char* name, F func) {
    ++total;

    bool threw = false;

    try {
        func();
    }
    catch (...) {
        threw = true;
    }

    if (threw) {
        std::cout << "  [PASS] " << name << "\n";
    }
    else {
        ++failed;
        std::cout << "  [FAIL] " << name << "\n";
    }
}


template <class T>
void PrintSequence(const Sequence<T>* seq) {
    std::cout << "[";

    for (int i = 0; i < seq->GetLength(); ++i) {
        if (i > 0) {
            std::cout << ", ";
        }

        std::cout << seq->Get(i);
    }

    std::cout << "]";
}


bool MaxCompare(const int& a, const int& b) {
    return a > b;
}


void TestBinarySearchTree() {
    std::cout << "\n=== BinarySearchTree ===\n";

    BinarySearchTree<int> tree;

    Check("new tree is empty", tree.IsEmpty());
    Check("new tree count is 0", tree.GetCount() == 0);

    CheckThrows("GetMin on empty throws", [&]() {
        tree.GetMin();
        });

    CheckThrows("GetMax on empty throws", [&]() {
        tree.GetMax();
        });

    tree.Insert(8);
    tree.Insert(3);
    tree.Insert(10);
    tree.Insert(1);
    tree.Insert(6);
    tree.Insert(14);

    Check("tree is not empty", !tree.IsEmpty());
    Check("count after inserts", tree.GetCount() == 6);

    Check("contains 8", tree.Contains(8));
    Check("contains 1", tree.Contains(1));
    Check("contains 14", tree.Contains(14));
    Check("does not contain 100", !tree.Contains(100));

    Check("min is 1", tree.GetMin() == 1);
    Check("max is 14", tree.GetMax() == 14);

    tree.Insert(8);
    Check("duplicate insert does not increase count", tree.GetCount() == 6);

    Sequence<int>* inOrder = tree.ToSequenceInOrder();
    std::cout << "  InOrder: ";
    PrintSequence(inOrder);
    std::cout << "\n";

    Check("inorder length", inOrder->GetLength() == 6);
    Check("inorder sorted first", inOrder->Get(0) == 1);
    Check("inorder sorted last", inOrder->Get(5) == 14);

    delete inOrder;

    Sequence<int>* preOrder = tree.ToSequencePreOrder();
    std::cout << "  PreOrder: ";
    PrintSequence(preOrder);
    std::cout << "\n";
    delete preOrder;

    Sequence<int>* postOrder = tree.ToSequencePostOrder();
    std::cout << "  PostOrder: ";
    PrintSequence(postOrder);
    std::cout << "\n";
    delete postOrder;

    Check("remove existing element", tree.Remove(3));
    Check("count after remove", tree.GetCount() == 5);
    Check("removed element not found", !tree.Contains(3));

    Check("remove missing element returns false", !tree.Remove(999));
    Check("count after failed remove unchanged", tree.GetCount() == 5);

    BinarySearchTree<int> copy(tree);
    Check("copy count", copy.GetCount() == tree.GetCount());
    Check("copy contains 8", copy.Contains(8));
    Check("copy contains 14", copy.Contains(14));

    tree.Clear();
    Check("tree empty after clear", tree.IsEmpty());
    Check("tree count after clear", tree.GetCount() == 0);
    Check("copy independent after original clear", copy.GetCount() == 5);
}


void TestBinaryHeapMin() {
    std::cout << "\n=== BinaryHeap min-heap ===\n";

    BinaryHeap<int> heap;

    Check("new heap is empty", heap.IsEmpty());
    Check("new heap count is 0", heap.GetCount() == 0);

    CheckThrows("Peek on empty throws", [&]() {
        heap.Peek();
        });

    CheckThrows("Pop on empty throws", [&]() {
        heap.Pop();
        });

    heap.Push(5);
    heap.Push(3);
    heap.Push(8);
    heap.Push(1);
    heap.Push(4);

    Check("heap count after pushes", heap.GetCount() == 5);
    Check("peek min", heap.Peek() == 1);

    Check("pop 1", heap.Pop() == 1);
    Check("pop 3", heap.Pop() == 3);
    Check("pop 4", heap.Pop() == 4);
    Check("pop 5", heap.Pop() == 5);
    Check("pop 8", heap.Pop() == 8);

    Check("heap empty after pops", heap.IsEmpty());

    int data[] = { 9, 2, 7, 1, 6 };
    BinaryHeap<int> fromArray(data, 5);

    Check("heap from array count", fromArray.GetCount() == 5);
    Check("heap from array peek", fromArray.Peek() == 1);

    Sequence<int>* seq = fromArray.ToSequence();
    std::cout << "  Heap internal order: ";
    PrintSequence(seq);
    std::cout << "\n";
    delete seq;

    fromArray.Clear();
    Check("heap clear makes empty", fromArray.IsEmpty());
}


void TestBinaryHeapMax() {
    std::cout << "\n=== BinaryHeap max-heap ===\n";

    BinaryHeap<int> heap(MaxCompare);

    heap.Push(5);
    heap.Push(3);
    heap.Push(8);
    heap.Push(1);
    heap.Push(4);

    Check("max heap count", heap.GetCount() == 5);
    Check("max heap peek", heap.Peek() == 8);

    Check("max pop 8", heap.Pop() == 8);
    Check("max pop 5", heap.Pop() == 5);
    Check("max pop 4", heap.Pop() == 4);
    Check("max pop 3", heap.Pop() == 3);
    Check("max pop 1", heap.Pop() == 1);

    Check("max heap empty after pops", heap.IsEmpty());
}


void TestBinaryHeapExceptions() {
    std::cout << "\n=== BinaryHeap exceptions ===\n";

    CheckThrows("negative count throws", []() {
        BinaryHeap<int> heap(nullptr, -1);
        });

    CheckThrows("null items with positive count throws", []() {
        BinaryHeap<int> heap(nullptr, 3);
        });

    CheckThrows("null compare throws", []() {
        BinaryHeap<int> heap(nullptr);
        });
}


int main() {
    TestBinarySearchTree();
    TestBinaryHeapMin();
    TestBinaryHeapMax();
    TestBinaryHeapExceptions();

    std::cout << "\n=== RESULT ===\n";
    std::cout << "Passed: " << total - failed << " / " << total << "\n";

    if (failed > 0) {
        std::cout << "Failed: " << failed << "\n";
        return 1;
    }

    return 0;
}