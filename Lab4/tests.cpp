#include <iostream>
#include <stdexcept>

#include "lib/ArraySequence.h"

#include "lazy/Generator.h"
#include "lazy/LazySequence.h"

#include "streams/StreamExceptions.h"
#include "streams/SequenceReadOnlyStream.h"
#include "streams/LazyReadOnlyStream.h"
#include "streams/SequenceWriteOnlyStream.h"


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
        if (expr) { \
            ok(desc); \
        } \
        else { \
            fail(desc, __FILE__, __LINE__, #expr); \
        } \
    } while (0)


#define CHECK_THROWS(desc, expr) \
    do { \
        bool threw = false; \
        try { \
            expr; \
        } \
        catch (...) { \
            threw = true; \
        } \
        if (threw) { \
            ok(desc); \
        } \
        else { \
            fail(desc, __FILE__, __LINE__, "expected exception: " #expr); \
        } \
    } while (0)


#define SUITE(name) \
    do { \
        std::cout << "\n=== " << name << " ===\n"; \
    } while (0)


static LazySequence<int>* CreateFibonacciLazySequence() {
    int initData[] = { 0, 1 };
    MutableArraySequence<int> init(initData, 2);

    return new LazySequence<int>(
        [](const Sequence<int>* seq) {
            int n = seq->GetLength();
            return seq->Get(n - 1) + seq->Get(n - 2);
        },
        &init
    );
}


void test_Generator() {
    SUITE("Generator");

    {
        int data[] = { 1, 2, 3 };
        MutableArraySequence<int> seq(data, 3);

        Generator<int> gen(
            [](const Sequence<int>* source) {
                return source->GetLength() * 10;
            },
            &seq
        );

        CHECK("generator has next", gen.HasNext());
        CHECK("generator first value", gen.GetNext() == 30);
        CHECK("generator position after one GetNext", gen.GetPosition() == 1);
        CHECK("generator second value", gen.GetNext() == 30);
        CHECK("generator position after two GetNext", gen.GetPosition() == 2);

        gen.Reset();
        CHECK("generator reset position", gen.GetPosition() == 0);
    }

    CHECK_THROWS("generator empty rule throws",
        Generator<int>(std::function<int(const Sequence<int>*)>(), nullptr));
}


void test_LazySequence_Finite() {
    SUITE("LazySequence finite");

    {
        int data[] = { 10, 20, 30 };
        LazySequence<int> seq(data, 3);

        CHECK("finite length", seq.GetLength() == 3);
        CHECK("finite materialized count", seq.GetMaterializedCount() == 3);
        CHECK("finite first", seq.GetFirst() == 10);
        CHECK("finite last", seq.GetLast() == 30);
        CHECK("finite get middle", seq.Get(1) == 20);
        CHECK("finite is not infinite", !seq.IsInfinite());

        CHECK_THROWS("finite get negative throws", seq.Get(-1));
        CHECK_THROWS("finite get out of range throws", seq.Get(3));
    }

    {
        LazySequence<int> empty;

        CHECK("empty length", empty.GetLength() == 0);
        CHECK("empty materialized count", empty.GetMaterializedCount() == 0);
        CHECK_THROWS("empty GetFirst throws", empty.GetFirst());
        CHECK_THROWS("empty GetLast throws", empty.GetLast());
    }

    {
        int data[] = { 1, 2, 3 };
        LazySequence<int> seq(data, 3);

        Sequence<int>* appended = seq.Append(4);

        CHECK("append returns new length", appended->GetLength() == 4);
        CHECK("append new last", appended->Get(3) == 4);
        CHECK("append original unchanged", seq.GetLength() == 3);

        delete appended;
    }

    {
        int data[] = { 2, 3 };
        LazySequence<int> seq(data, 2);

        Sequence<int>* prepended = seq.Prepend(1);

        CHECK("prepend returns new length", prepended->GetLength() == 3);
        CHECK("prepend first", prepended->Get(0) == 1);
        CHECK("prepend second", prepended->Get(1) == 2);

        delete prepended;
    }

    {
        int data[] = { 1, 2, 4 };
        LazySequence<int> seq(data, 3);

        Sequence<int>* inserted = seq.InsertAt(3, 2);

        CHECK("insert length", inserted->GetLength() == 4);
        CHECK("insert value", inserted->Get(2) == 3);
        CHECK("insert last", inserted->Get(3) == 4);

        delete inserted;
    }

    {
        int data[] = { 10, 20, 30, 40 };
        LazySequence<int> seq(data, 4);

        Sequence<int>* sub = seq.GetSubsequence(1, 2);

        CHECK("subsequence length", sub->GetLength() == 2);
        CHECK("subsequence first", sub->Get(0) == 20);
        CHECK("subsequence second", sub->Get(1) == 30);

        delete sub;
    }
}


void test_LazySequence_Infinite() {
    SUITE("LazySequence infinite");

    LazySequence<int>* fib = CreateFibonacciLazySequence();

    CHECK("fib is infinite", fib->IsInfinite());
    CHECK("fib initial materialized count", fib->GetMaterializedCount() == 2);

    CHECK("fib 0", fib->Get(0) == 0);
    CHECK("fib 1", fib->Get(1) == 1);
    CHECK("fib 2", fib->Get(2) == 1);
    CHECK("fib 3", fib->Get(3) == 2);
    CHECK("fib 4", fib->Get(4) == 3);
    CHECK("fib 5", fib->Get(5) == 5);
    CHECK("fib 6", fib->Get(6) == 8);

    CHECK("fib materialized after Get(6)", fib->GetMaterializedCount() == 7);

    int oldCount = fib->GetMaterializedCount();
    CHECK("fib repeated get", fib->Get(6) == 8);
    CHECK("fib repeated get does not materialize more",
        fib->GetMaterializedCount() == oldCount);

    CHECK_THROWS("infinite GetLength throws", fib->GetLength());
    CHECK_THROWS("infinite GetLast throws", fib->GetLast());
    CHECK_THROWS("infinite Append throws", fib->Append(100));

    Sequence<int>* sub = fib->GetSubsequence(2, 5);

    CHECK("infinite subsequence length", sub->GetLength() == 4);
    CHECK("infinite subsequence first", sub->Get(0) == 1);
    CHECK("infinite subsequence last", sub->Get(3) == 5);

    delete sub;
    delete fib;
}


void test_LazySequence_Enumerator() {
    SUITE("LazySequence enumerator");

    LazySequence<int>* fib = CreateFibonacciLazySequence();

    IEnumerator<int>* en = fib->GetEnumerator();

    CHECK_THROWS("GetCurrent before MoveNext throws", en->GetCurrent());

    int expected[] = { 0, 1, 1, 2, 3, 5, 8 };

    for (int i = 0; i < 7; ++i) {
        CHECK("enumerator MoveNext true", en->MoveNext());
        CHECK("enumerator value", en->GetCurrent() == expected[i]);
    }

    en->Reset();

    CHECK("enumerator MoveNext after reset", en->MoveNext());
    CHECK("enumerator value after reset", en->GetCurrent() == 0);

    delete en;
    delete fib;
}


void test_SequenceReadOnlyStream() {
    SUITE("SequenceReadOnlyStream");

    int data[] = { 10, 20, 30 };
    MutableArraySequence<int> seq(data, 3);

    SequenceReadOnlyStream<int> stream(&seq);

    CHECK_THROWS("read before open throws", stream.Read());
    CHECK_THROWS("IsEndOfStream before open throws", stream.IsEndOfStream());

    stream.Open();

    CHECK("stream can seek", stream.IsCanSeek());
    CHECK("stream can go back", stream.IsCanGoBack());
    CHECK("position after open", stream.GetPosition() == 0);
    CHECK("not end after open", !stream.IsEndOfStream());

    CHECK("read first", stream.Read() == 10);
    CHECK("position after first read", stream.GetPosition() == 1);

    CHECK("read second", stream.Read() == 20);
    CHECK("position after second read", stream.GetPosition() == 2);

    CHECK("seek to 1", stream.Seek(1) == 1);
    CHECK("read after seek", stream.Read() == 20);

    CHECK("read last", stream.Read() == 30);
    CHECK("end after reading all", stream.IsEndOfStream());

    CHECK_THROWS("read after end throws", stream.Read());
    CHECK_THROWS("seek negative throws", stream.Seek(-1));
    CHECK_THROWS("seek too far throws", stream.Seek(4));

    stream.Close();

    CHECK_THROWS("read after close throws", stream.Read());
}


void test_LazyReadOnlyStream() {
    SUITE("LazyReadOnlyStream");

    LazySequence<int>* fib = CreateFibonacciLazySequence();

    LazyReadOnlyStream<int> stream(fib);

    CHECK_THROWS("lazy stream read before open throws", stream.Read());

    stream.Open();

    CHECK("lazy stream is not end", !stream.IsEndOfStream());
    CHECK("lazy stream position open", stream.GetPosition() == 0);

    CHECK("lazy read 0", stream.Read() == 0);
    CHECK("lazy read 1", stream.Read() == 1);
    CHECK("lazy read 2", stream.Read() == 1);
    CHECK("lazy read 3", stream.Read() == 2);
    CHECK("lazy read 4", stream.Read() == 3);

    CHECK("lazy stream position after reads", stream.GetPosition() == 5);
    CHECK("lazy sequence materialized", fib->GetMaterializedCount() >= 5);

    CHECK("lazy seek to 2", stream.Seek(2) == 2);
    CHECK("lazy read after seek", stream.Read() == 1);

    CHECK_THROWS("lazy seek negative throws", stream.Seek(-1));

    stream.Close();

    CHECK_THROWS("lazy stream read after close throws", stream.Read());

    delete fib;
}


void test_SequenceWriteOnlyStream() {
    SUITE("SequenceWriteOnlyStream");

    Sequence<int>* seq = new MutableArraySequence<int>();

    SequenceWriteOnlyStream<int> stream(seq);

    CHECK_THROWS("write before open throws", stream.Write(10));

    stream.Open();

    CHECK("write first returns position 1", stream.Write(10) == 1);
    CHECK("write second returns position 2", stream.Write(20) == 2);
    CHECK("write third returns position 3", stream.Write(30) == 3);

    CHECK("write stream position", stream.GetPosition() == 3);

    stream.Close();

    CHECK("written sequence length", seq->GetLength() == 3);
    CHECK("written sequence first", seq->Get(0) == 10);
    CHECK("written sequence second", seq->Get(1) == 20);
    CHECK("written sequence third", seq->Get(2) == 30);

    CHECK_THROWS("write after close throws", stream.Write(40));

    delete seq;
}


void run_all_tests() {
    test_Generator();

    test_LazySequence_Finite();
    test_LazySequence_Infinite();
    test_LazySequence_Enumerator();

    test_SequenceReadOnlyStream();
    test_LazyReadOnlyStream();
    test_SequenceWriteOnlyStream();

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