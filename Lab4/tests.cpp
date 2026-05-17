#include <iostream>
#include <stdexcept>

#include "lib/ArraySequence.h"

#include "lazy/Cardinal.h"
#include "lazy/RuleGenerator.h"
#include "lazy/SequenceGenerator.h"
#include "lazy/LazySequence.h"


static int total = 0;
static int failed = 0;


static void ok(const char* desc) {
    ++total;
    std::cout << "  [PASS] " << desc << "\n";
}


static void fail(const char* desc, const char* file, int line, const char* expr) {
    ++total;
    ++failed;

    std::cout << "  [FAIL] " << desc << "\n"
        << "         " << file << ":" << line << " — " << expr << "\n";
}


#define CHECK(desc, expr) \
    do { \
        if (expr) { ok(desc); } \
        else { fail(desc, __FILE__, __LINE__, #expr); } \
    } while (0)


#define CHECK_THROWS(desc, expr) \
    do { \
        bool threw = false; \
        try { expr; } \
        catch (...) { threw = true; } \
        if (threw) { ok(desc); } \
        else { fail(desc, __FILE__, __LINE__, "expected exception: " #expr); } \
    } while (0)


#define SUITE(name) \
    do { std::cout << "\n=== " << name << " ===\n"; } while (0)


static LazySequence<int>* CreateFibonacci() {
    int init_data[] = { 0, 1 };
    MutableArraySequence<int> init(init_data, 2);

    return new LazySequence<int>(
        [](const Sequence<int>& source) {
            int length = source.GetLength();
            return source.Get(length - 1) + source.Get(length - 2);
        },
        init
    );
}


void test_InsertAtFinite() {
    SUITE("LazySequence InsertAt finite");

    int data[] = { 10, 20, 30 };
    LazySequence<int> seq(data, 3);

    Sequence<int>* inserted = seq.InsertAt(15, 1);

    CHECK("finite insert length", inserted->GetLength() == 4);
    CHECK("finite insert first", inserted->Get(0) == 10);
    CHECK("finite insert item", inserted->Get(1) == 15);
    CHECK("finite insert shifted first", inserted->Get(2) == 20);
    CHECK("finite insert shifted second", inserted->Get(3) == 30);

    CHECK("finite original length unchanged", seq.GetLength() == 3);
    CHECK("finite original value unchanged", seq.Get(1) == 20);

    delete inserted;

    Sequence<int>* insert_begin = seq.InsertAt(5, 0);

    CHECK("finite insert begin length", insert_begin->GetLength() == 4);
    CHECK("finite insert begin value", insert_begin->Get(0) == 5);
    CHECK("finite insert begin shifted", insert_begin->Get(1) == 10);

    delete insert_begin;

    Sequence<int>* insert_end = seq.InsertAt(40, 3);

    CHECK("finite insert end length", insert_end->GetLength() == 4);
    CHECK("finite insert end old last", insert_end->Get(2) == 30);
    CHECK("finite insert end new last", insert_end->Get(3) == 40);

    delete insert_end;

    CHECK_THROWS("finite insert negative throws", seq.InsertAt(100, -1));
    CHECK_THROWS("finite insert too far throws", seq.InsertAt(100, 4));
}


void test_InsertAtInfinite() {
    SUITE("LazySequence InsertAt infinite");

    LazySequence<int>* fib = CreateFibonacci();

    Sequence<int>* inserted = fib->InsertAt(100, 3);
    LazySequence<int>* lazy_inserted = dynamic_cast<LazySequence<int>*>(inserted);

    CHECK("infinite insert result is LazySequence", lazy_inserted != nullptr);
    CHECK("infinite insert result is infinite", lazy_inserted->IsInfinite());
    CHECK("infinite insert GetLength throws", true);

    CHECK_THROWS("infinite insert GetLength really throws", lazy_inserted->GetLength());

    CHECK("infinite insert value 0", lazy_inserted->Get(0) == 0);
    CHECK("infinite insert value 1", lazy_inserted->Get(1) == 1);
    CHECK("infinite insert value 2", lazy_inserted->Get(2) == 1);
    CHECK("infinite insert inserted item", lazy_inserted->Get(3) == 100);
    CHECK("infinite insert shifted 3", lazy_inserted->Get(4) == 2);
    CHECK("infinite insert shifted 4", lazy_inserted->Get(5) == 3);
    CHECK("infinite insert shifted 5", lazy_inserted->Get(6) == 5);

    CHECK("infinite original unchanged", fib->Get(3) == 2);

    delete inserted;

    Sequence<int>* insert_begin = fib->InsertAt(777, 0);
    LazySequence<int>* lazy_begin = dynamic_cast<LazySequence<int>*>(insert_begin);

    CHECK("infinite insert begin is infinite", lazy_begin->IsInfinite());
    CHECK("infinite insert begin item", lazy_begin->Get(0) == 777);
    CHECK("infinite insert begin old first", lazy_begin->Get(1) == 0);
    CHECK("infinite insert begin old second", lazy_begin->Get(2) == 1);
    CHECK("infinite insert begin old third", lazy_begin->Get(3) == 1);

    delete insert_begin;

    CHECK_THROWS("infinite insert negative throws", fib->InsertAt(100, -1));

    delete fib;
}


void test_ConcatFiniteFinite() {
    SUITE("LazySequence Concat finite + finite");

    int left_data[] = { 1, 2 };
    int right_data[] = { 3, 4 };

    LazySequence<int> left(left_data, 2);
    LazySequence<int> right(right_data, 2);

    Sequence<int>* result = left.Concat(right);

    CHECK("finite finite concat length", result->GetLength() == 4);
    CHECK("finite finite concat 0", result->Get(0) == 1);
    CHECK("finite finite concat 1", result->Get(1) == 2);
    CHECK("finite finite concat 2", result->Get(2) == 3);
    CHECK("finite finite concat 3", result->Get(3) == 4);

    CHECK("finite finite original left unchanged", left.GetLength() == 2);
    CHECK("finite finite original right unchanged", right.GetLength() == 2);

    delete result;
}


void test_ConcatInfiniteFinite() {
    SUITE("LazySequence Concat infinite + finite");

    LazySequence<int>* fib = CreateFibonacci();

    int data[] = { 100, 200 };
    LazySequence<int> finite(data, 2);

    Sequence<int>* result = fib->Concat(finite);
    LazySequence<int>* lazy_result = dynamic_cast<LazySequence<int>*>(result);

    CHECK("infinite finite concat result is LazySequence", lazy_result != nullptr);
    CHECK("infinite finite concat is infinite", lazy_result->IsInfinite());

    CHECK("infinite finite concat 0", lazy_result->Get(0) == 0);
    CHECK("infinite finite concat 1", lazy_result->Get(1) == 1);
    CHECK("infinite finite concat 2", lazy_result->Get(2) == 1);
    CHECK("infinite finite concat 3", lazy_result->Get(3) == 2);
    CHECK("infinite finite concat 6", lazy_result->Get(6) == 8);

    delete result;
    delete fib;
}


void test_ConcatFiniteInfinite() {
    SUITE("LazySequence Concat finite + infinite");

    int data[] = { 100, 200 };
    LazySequence<int> finite(data, 2);

    LazySequence<int>* fib = CreateFibonacci();

    Sequence<int>* result = finite.Concat(*fib);
    LazySequence<int>* lazy_result = dynamic_cast<LazySequence<int>*>(result);

    CHECK("finite infinite concat result is LazySequence", lazy_result != nullptr);
    CHECK("finite infinite concat is infinite", lazy_result->IsInfinite());
    CHECK_THROWS("finite infinite concat GetLength throws", lazy_result->GetLength());

    CHECK("finite infinite concat 0", lazy_result->Get(0) == 100);
    CHECK("finite infinite concat 1", lazy_result->Get(1) == 200);
    CHECK("finite infinite concat 2", lazy_result->Get(2) == 0);
    CHECK("finite infinite concat 3", lazy_result->Get(3) == 1);
    CHECK("finite infinite concat 4", lazy_result->Get(4) == 1);
    CHECK("finite infinite concat 5", lazy_result->Get(5) == 2);
    CHECK("finite infinite concat 7", lazy_result->Get(7) == 5);

    delete result;
    delete fib;
}


void run_all_tests() {
    test_InsertAtFinite();
    test_InsertAtInfinite();

    test_ConcatFiniteFinite();
    test_ConcatInfiniteFinite();
    test_ConcatFiniteInfinite();

    std::cout << "\n=== RESULT ===\n";
    std::cout << "Passed: " << total - failed << " / " << total << "\n";

    if (failed > 0) {
        std::cout << "Failed: " << failed << "\n";
    }
}


int main() {
    run_all_tests();

    return failed == 0 ? 0 : 1;
}