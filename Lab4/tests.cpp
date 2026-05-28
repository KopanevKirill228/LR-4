
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <cmath>
#include <string>
#include <stdexcept>

#include "lib/ArraySequence.h"

#include "lazy/Cardinal.h"
#include "lazy/CardinalIO.h"
#include "lazy/TransfiniteIndex.h"
#include "lazy/Generator.h"
#include "lazy/RuleGenerator.h"
#include "lazy/SequenceGenerator.h"
#include "lazy/LazySequence.h"

#include "streams/Stream.h"
#include "streams/ReadOnlyStream.h"
#include "streams/WriteOnlyStream.h"
#include "streams/SequenceReadOnlyStream.h"
#include "streams/SequenceWriteOnlyStream.h"
#include "streams/LazyReadOnlyStream.h"
#include "streams/FileLineReadOnlyStream.h"
#include "streams/FileLineWriteOnlyStream.h"
#include "streams/StreamExceptions.h"

#include "tasks/EventType.h"
#include "tasks/Event.h"
#include "tasks/EventParser.h"
#include "tasks/EventReadOnlyStream.h"
#include "tasks/OnlineEventStatistics.h"
#include "tasks/ProtocolStatisticsTask.h"

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

static bool near(double first, double second) {
    double diff = first - second;
    if (diff < 0) diff = -diff;
    return diff < 0.000001;
}

#define CHECK(desc, expr) \
    do { if (expr) ok(desc); else fail(desc, __FILE__, __LINE__, #expr); } while (0)

#define CHECK_NEAR(desc, first, second) \
    do { if (near((first), (second))) ok(desc); else fail(desc, __FILE__, __LINE__, #first " near " #second); } while (0)

#define CHECK_THROWS(desc, expr) \
    do { bool threw = false; try { expr; } catch (...) { threw = true; } \
        if (threw) ok(desc); else fail(desc, __FILE__, __LINE__, "expected exception: " #expr); } while (0)

#define SUITE(name) \
    do { std::cout << "\n=== " << name << " ===\n"; } while (0)

static LazySequence<int>* CreateNaturals() {
    int init_data[] = { 0 };
    MutableArraySequence<int> init(init_data, 1);
    return new LazySequence<int>(
        [](const Sequence<int>& source) {
            return source.GetLength();
        },
        init
    );
}

static LazySequence<int>* CreateFibonacciFromZeroOne() {
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

static LazySequence<int>* CreateFibonacciFromOnes() {
    int init_data[] = { 1, 1 };
    MutableArraySequence<int> init(init_data, 2);
    return new LazySequence<int>(
        [](const Sequence<int>& source) {
            int length = source.GetLength();
            return source.Get(length - 1) + source.Get(length - 2);
        },
        init
    );
}

void test_CardinalAndCardinalIO() {
    SUITE("Cardinal and CardinalIO");

    Cardinal finite = Cardinal::Finite(5);
    Cardinal zero = Cardinal::Finite(0);
    Cardinal infinity = Cardinal::Infinity();

    CHECK("finite is finite", finite.IsFinite());
    CHECK("finite is not infinite", !finite.IsInfinite());
    CHECK("finite value", finite.GetFiniteValue() == 5);
    CHECK("zero cardinal value", zero.GetFiniteValue() == 0);
    CHECK("infinity is infinite", infinity.IsInfinite());
    CHECK("infinity is not finite", !infinity.IsFinite());
    CHECK_THROWS("infinity finite value throws", infinity.GetFiniteValue());
    CHECK_THROWS("negative finite cardinal throws", Cardinal::Finite(-1));

    std::ostringstream finite_output;
    std::ostringstream infinity_output;
    finite_output << finite;
    infinity_output << infinity;

    CHECK("finite CardinalIO has text", finite_output.str().size() > 0);
    CHECK("infinity CardinalIO has text", infinity_output.str().size() > 0);
}

void test_TransfiniteIndex() {
    SUITE("TransfiniteIndex");

    TransfiniteIndex finite = TransfiniteIndex::Finite(7);
    TransfiniteIndex omega = TransfiniteIndex::AfterInfinity(2);
    TransfiniteIndex two_omega(2, 3);

    CHECK("finite is finite", finite.IsFinite());
    CHECK("finite is not after infinity", !finite.IsAfterInfinity());
    CHECK("finite infinity count", finite.GetInfinityCount() == 0);
    CHECK("finite value", finite.GetFiniteIndex() == 7);
    CHECK("omega is not finite", !omega.IsFinite());
    CHECK("omega is after infinity", omega.IsAfterInfinity());
    CHECK("omega infinity count", omega.GetInfinityCount() == 1);
    CHECK("omega finite part", omega.GetFiniteIndex() == 2);
    CHECK("two omega count", two_omega.GetInfinityCount() == 2);
    CHECK("two omega finite part", two_omega.GetFiniteIndex() == 3);

    CHECK_THROWS("negative finite throws", TransfiniteIndex::Finite(-1));
    CHECK_THROWS("negative after infinity throws", TransfiniteIndex::AfterInfinity(-1));
    CHECK_THROWS("negative infinity count throws", TransfiniteIndex(-1, 0));
    CHECK_THROWS("negative finite part throws", TransfiniteIndex(1, -1));
}

void test_LazySequenceBasic() {
    SUITE("LazySequence basic");

    LazySequence<int> empty;
    CHECK("empty length", empty.GetLength() == 0);
    CHECK("empty cardinality finite", empty.GetCardinality().IsFinite());
    CHECK("empty materialized count", empty.GetMaterializedCount() == 0);
    CHECK("empty is not infinite", !empty.IsInfinite());
    CHECK_THROWS("empty first throws", empty.GetFirst());
    CHECK_THROWS("empty last throws", empty.GetLast());
    CHECK_THROWS("empty get throws", empty.Get(0));
    CHECK_THROWS("empty omega throws", empty.Get(TransfiniteIndex::AfterInfinity(0)));

    int data[] = { 10, 20, 30 };
    LazySequence<int> finite(data, 3);

    CHECK("finite length", finite.GetLength() == 3);
    CHECK("finite first", finite.GetFirst() == 10);
    CHECK("finite last", finite.GetLast() == 30);
    CHECK("finite get 1", finite.Get(1) == 20);
    CHECK("finite operator", finite[2] == 30);
    CHECK("finite transfinite finite get", finite.Get(TransfiniteIndex::Finite(2)) == 30);
    CHECK_THROWS("finite negative get throws", finite.Get(-1));
    CHECK_THROWS("finite too far throws", finite.Get(3));
    CHECK_THROWS("finite omega throws", finite.Get(TransfiniteIndex::AfterInfinity(0)));

    CHECK_THROWS("constructor negative count throws", LazySequence<int>(data, -1));
    CHECK_THROWS("constructor null positive throws", LazySequence<int>(nullptr, 1));

    LazySequence<int> null_empty(nullptr, 0);
    CHECK("null empty length", null_empty.GetLength() == 0);

    LazySequence<int> copied(finite);
    CHECK("copy length", copied.GetLength() == 3);
    CHECK("copy value", copied.Get(2) == 30);

    LazySequence<int> assigned;
    assigned = finite;
    CHECK("assignment length", assigned.GetLength() == 3);
    CHECK("assignment value", assigned.Get(1) == 20);

    assigned = assigned;
    CHECK("self assignment value", assigned.Get(1) == 20);
}

void test_RuleGeneratorAndMemoization() {
    SUITE("RuleGenerator and memoization");

    LazySequence<int>* naturals = CreateNaturals();

    CHECK("naturals infinite", naturals->IsInfinite());
    CHECK("naturals cardinality infinite", naturals->GetCardinality().IsInfinite());
    CHECK_THROWS("naturals finite length throws", naturals->GetLength());
    CHECK_THROWS("naturals last throws", naturals->GetLast());
    CHECK("naturals 0", naturals->Get(0) == 0);
    CHECK("naturals 1", naturals->Get(1) == 1);
    CHECK("naturals 100", naturals->Get(100) == 100);

    int materialized = naturals->GetMaterializedCount();
    naturals->Get(100);
    CHECK("repeated get does not grow cache", naturals->GetMaterializedCount() == materialized);

    delete naturals;

    int empty_data[] = {};
    MutableArraySequence<int> empty_init(empty_data, 0);
    CHECK_THROWS(
        "rule with empty init throws",
        LazySequence<int>(
            [](const Sequence<int>& source) {
                return source.GetLength();
            },
            empty_init
        )
    );
}

void test_LazySubsequenceAndEnumerator() {
    SUITE("LazySequence subsequence and enumerator");

    int data[] = { 10, 20, 30, 40, 50 };
    LazySequence<int> finite(data, 5);

    Sequence<int>* sub = finite.GetSubsequence(1, 3);
    LazySequence<int>* lazy_sub = dynamic_cast<LazySequence<int>*>(sub);

    CHECK("subsequence is lazy", lazy_sub != nullptr);
    CHECK("subsequence length", lazy_sub->GetLength() == 3);
    CHECK("subsequence 0", lazy_sub->Get(0) == 20);
    CHECK("subsequence 2", lazy_sub->Get(2) == 40);
    delete sub;

    CHECK_THROWS("subsequence negative start throws", finite.GetSubsequence(-1, 2));
    CHECK_THROWS("subsequence negative end throws", finite.GetSubsequence(1, -2));
    CHECK_THROWS("subsequence end before start throws", finite.GetSubsequence(3, 1));
    CHECK_THROWS("subsequence too far throws", finite.GetSubsequence(1, 5));

    IEnumerator<int>* en = finite.GetEnumerator();
    CHECK_THROWS("current before move throws", en->GetCurrent());
    CHECK("move first", en->MoveNext());
    CHECK("current first", en->GetCurrent() == 10);
    CHECK("move second", en->MoveNext());
    CHECK("current second", en->GetCurrent() == 20);
    en->Reset();
    CHECK_THROWS("current after reset throws", en->GetCurrent());

    int count = 0;
    while (en->MoveNext()) {
        ++count;
    }

    CHECK("enumerator counted all", count == 5);
    CHECK_THROWS("current after end throws", en->GetCurrent());
    delete en;
}

void test_SequenceGeneratorDirectly() {
    SUITE("SequenceGenerator directly");

    int data[] = { 10, 20, 30 };
    MutableArraySequence<int> sequence(data, 3);
    SequenceGenerator<int> generator(sequence);

    CHECK("has next initially", generator.HasNext());
    CHECK("next 0", generator.GetNext() == 10);
    CHECK("position 1", generator.GetPosition() == 1);
    CHECK("next 1", generator.GetNext() == 20);
    CHECK("next 2", generator.GetNext() == 30);
    CHECK("has no next", !generator.HasNext());
    CHECK_THROWS("get after end throws", generator.GetNext());

    generator.Reset();
    CHECK("position after reset", generator.GetPosition() == 0);
    CHECK("next after reset", generator.GetNext() == 10);

    Generator<int>* clone = generator.Clone();
    CHECK("clone position copied", clone->GetPosition() == generator.GetPosition());
    CHECK("clone next", clone->GetNext() == 20);
    delete clone;
}

void test_AppendGenerator() {
    SUITE("AppendGenerator");

    int data[] = { 10, 20, 30 };
    LazySequence<int> finite(data, 3);

    Sequence<int>* finite_result = finite.Append(40);
    LazySequence<int>* lazy_finite_result = dynamic_cast<LazySequence<int>*>(finite_result);

    CHECK("finite append result lazy", lazy_finite_result != nullptr);
    CHECK("finite append length", lazy_finite_result->GetLength() == 4);
    CHECK("finite append old", lazy_finite_result->Get(2) == 30);
    CHECK("finite append new", lazy_finite_result->Get(3) == 40);
    CHECK_THROWS("finite append omega throws", lazy_finite_result->Get(TransfiniteIndex::AfterInfinity(0)));
    delete finite_result;

    LazySequence<int>* fib = CreateFibonacciFromOnes();
    Sequence<int>* infinite_result = fib->Append(0);
    LazySequence<int>* lazy_infinite_result = dynamic_cast<LazySequence<int>*>(infinite_result);

    CHECK("infinite append result lazy", lazy_infinite_result != nullptr);
    CHECK("infinite append cardinality infinite", lazy_infinite_result->GetCardinality().IsInfinite());
    CHECK("infinite append ordinary 4", lazy_infinite_result->Get(4) == 5);
    CHECK("infinite append ordinary 5 unaffected", lazy_infinite_result->Get(5) == 8);
    CHECK("infinite append omega 0", lazy_infinite_result->Get(TransfiniteIndex::AfterInfinity(0)) == 0);
    CHECK_THROWS("infinite append omega 1 throws", lazy_infinite_result->Get(TransfiniteIndex::AfterInfinity(1)));
    delete infinite_result;
    delete fib;
}

void test_SeveralAppendsToInfinite() {
    SUITE("Several appends to infinite");

    LazySequence<int>* fib = CreateFibonacciFromOnes();

    Sequence<int>* first = fib->Append(0);
    LazySequence<int>* lazy_first = dynamic_cast<LazySequence<int>*>(first);
    Sequence<int>* second = lazy_first->Append(7);
    LazySequence<int>* lazy_second = dynamic_cast<LazySequence<int>*>(second);
    Sequence<int>* third = lazy_second->Append(9);
    LazySequence<int>* lazy_third = dynamic_cast<LazySequence<int>*>(third);

    CHECK("ordinary still fibonacci 4", lazy_third->Get(4) == 5);
    CHECK("ordinary still fibonacci 5", lazy_third->Get(5) == 8);
    CHECK("ordinary still fibonacci 6", lazy_third->Get(6) == 13);
    CHECK("omega first append", lazy_third->Get(TransfiniteIndex::AfterInfinity(0)) == 0);
    CHECK("omega second append", lazy_third->Get(TransfiniteIndex::AfterInfinity(1)) == 7);
    CHECK("omega third append", lazy_third->Get(TransfiniteIndex::AfterInfinity(2)) == 9);
    CHECK_THROWS("omega after last throws", lazy_third->Get(TransfiniteIndex::AfterInfinity(3)));

    delete third;
    delete second;
    delete first;
    delete fib;
}

void test_PrependGenerator() {
    SUITE("PrependGenerator");

    int data[] = { 10, 20, 30 };
    LazySequence<int> finite(data, 3);

    Sequence<int>* finite_result = finite.Prepend(5);
    LazySequence<int>* lazy_finite_result = dynamic_cast<LazySequence<int>*>(finite_result);

    CHECK("finite prepend result lazy", lazy_finite_result != nullptr);
    CHECK("finite prepend length", lazy_finite_result->GetLength() == 4);
    CHECK("finite prepend first", lazy_finite_result->Get(0) == 5);
    CHECK("finite prepend shifted", lazy_finite_result->Get(1) == 10);
    delete finite_result;

    LazySequence<int>* naturals = CreateNaturals();
    Sequence<int>* infinite_result = naturals->Prepend(-1);
    LazySequence<int>* lazy_infinite_result = dynamic_cast<LazySequence<int>*>(infinite_result);

    CHECK("infinite prepend result lazy", lazy_infinite_result != nullptr);
    CHECK("infinite prepend first", lazy_infinite_result->Get(0) == -1);
    CHECK("infinite prepend shifted 0", lazy_infinite_result->Get(1) == 0);
    CHECK("infinite prepend shifted 5", lazy_infinite_result->Get(6) == 5);
    delete infinite_result;
    delete naturals;
}

void test_InsertItemGenerator() {
    SUITE("InsertItemGenerator");

    int data[] = { 10, 20, 30 };
    LazySequence<int> finite(data, 3);

    Sequence<int>* begin = finite.InsertAt(5, 0);
    LazySequence<int>* lazy_begin = dynamic_cast<LazySequence<int>*>(begin);
    CHECK("insert begin length", lazy_begin->GetLength() == 4);
    CHECK("insert begin value", lazy_begin->Get(0) == 5);
    CHECK("insert begin shifted", lazy_begin->Get(1) == 10);
    delete begin;

    Sequence<int>* middle = finite.InsertAt(777, 1);
    LazySequence<int>* lazy_middle = dynamic_cast<LazySequence<int>*>(middle);
    CHECK("insert middle length", lazy_middle->GetLength() == 4);
    CHECK("insert middle before", lazy_middle->Get(0) == 10);
    CHECK("insert middle value", lazy_middle->Get(1) == 777);
    CHECK("insert middle shifted", lazy_middle->Get(2) == 20);
    delete middle;

    Sequence<int>* end = finite.InsertAt(40, 3);
    LazySequence<int>* lazy_end = dynamic_cast<LazySequence<int>*>(end);
    CHECK("insert end length", lazy_end->GetLength() == 4);
    CHECK("insert end old last", lazy_end->Get(2) == 30);
    CHECK("insert end new last", lazy_end->Get(3) == 40);
    delete end;

    CHECK_THROWS("insert negative throws", finite.InsertAt(0, -1));
    CHECK_THROWS("insert too far throws", finite.InsertAt(0, 4));

    LazySequence<int>* naturals = CreateNaturals();
    Sequence<int>* infinite_insert = naturals->InsertAt(100, 2);
    LazySequence<int>* lazy_infinite_insert = dynamic_cast<LazySequence<int>*>(infinite_insert);

    CHECK("insert infinite 0", lazy_infinite_insert->Get(0) == 0);
    CHECK("insert infinite 1", lazy_infinite_insert->Get(1) == 1);
    CHECK("insert infinite item", lazy_infinite_insert->Get(2) == 100);
    CHECK("insert infinite shifted", lazy_infinite_insert->Get(3) == 2);
    delete infinite_insert;
    delete naturals;
}

void test_ConcatGenerator() {
    SUITE("ConcatGenerator");

    int left_data[] = { 10, 20 };
    int right_data[] = { 30, 40, 50 };
    LazySequence<int> left(left_data, 2);
    LazySequence<int> right(right_data, 3);

    Sequence<int>* finite_finite = left.Concat(right);
    LazySequence<int>* lazy_finite_finite = dynamic_cast<LazySequence<int>*>(finite_finite);
    CHECK("finite finite result lazy", lazy_finite_finite != nullptr);
    CHECK("finite finite length", lazy_finite_finite->GetLength() == 5);
    CHECK("finite finite 0", lazy_finite_finite->Get(0) == 10);
    CHECK("finite finite 4", lazy_finite_finite->Get(4) == 50);
    delete finite_finite;

    LazySequence<int>* naturals = CreateNaturals();
    Sequence<int>* finite_infinite = left.Concat(*naturals);
    LazySequence<int>* lazy_finite_infinite = dynamic_cast<LazySequence<int>*>(finite_infinite);
    CHECK("finite infinite cardinality", lazy_finite_infinite->GetCardinality().IsInfinite());
    CHECK("finite infinite left", lazy_finite_infinite->Get(0) == 10);
    CHECK("finite infinite right 0", lazy_finite_infinite->Get(2) == 0);
    CHECK("finite infinite right 5", lazy_finite_infinite->Get(7) == 5);
    delete finite_infinite;

    Sequence<int>* infinite_finite = naturals->Concat(right);
    LazySequence<int>* lazy_infinite_finite = dynamic_cast<LazySequence<int>*>(infinite_finite);
    CHECK("infinite finite ordinary", lazy_infinite_finite->Get(5) == 5);
    CHECK("infinite finite omega 0", lazy_infinite_finite->Get(TransfiniteIndex::AfterInfinity(0)) == 30);
    CHECK("infinite finite omega 2", lazy_infinite_finite->Get(TransfiniteIndex::AfterInfinity(2)) == 50);
    CHECK_THROWS("infinite finite omega 3 throws", lazy_infinite_finite->Get(TransfiniteIndex::AfterInfinity(3)));
    delete infinite_finite;
    delete naturals;

    LazySequence<int>* first = CreateNaturals();
    LazySequence<int>* second = CreateFibonacciFromZeroOne();
    Sequence<int>* infinite_infinite = first->Concat(*second);
    LazySequence<int>* lazy_infinite_infinite = dynamic_cast<LazySequence<int>*>(infinite_infinite);

    CHECK("infinite infinite ordinary", lazy_infinite_infinite->Get(10) == 10);
    CHECK("infinite infinite omega 0", lazy_infinite_infinite->Get(TransfiniteIndex::AfterInfinity(0)) == 0);
    CHECK("infinite infinite omega 1", lazy_infinite_infinite->Get(TransfiniteIndex::AfterInfinity(1)) == 1);
    CHECK("infinite infinite omega 5", lazy_infinite_infinite->Get(TransfiniteIndex::AfterInfinity(5)) == 5);
    delete infinite_infinite;
    delete second;
    delete first;
}

void test_InsertSequenceGenerator() {
    SUITE("InsertSequenceGenerator");

    int source_data[] = { 10, 20, 30, 40 };
    int inserted_data[] = { 111, 222 };
    LazySequence<int> source(source_data, 4);
    LazySequence<int> inserted(inserted_data, 2);

    LazySequence<int>* finite_finite = source.InsertSequenceAt(inserted, 2);
    CHECK("finite finite length", finite_finite->GetLength() == 6);
    CHECK("finite finite before", finite_finite->Get(1) == 20);
    CHECK("finite finite inserted 0", finite_finite->Get(2) == 111);
    CHECK("finite finite inserted 1", finite_finite->Get(3) == 222);
    CHECK("finite finite shifted", finite_finite->Get(4) == 30);
    delete finite_finite;

    LazySequence<int>* naturals = CreateNaturals();
    LazySequence<int>* infinite_source = naturals->InsertSequenceAt(inserted, 2);
    CHECK("finite insert into infinite cardinality", infinite_source->GetCardinality().IsInfinite());
    CHECK("finite insert into infinite 0", infinite_source->Get(0) == 0);
    CHECK("finite insert into infinite 1", infinite_source->Get(1) == 1);
    CHECK("finite insert into infinite inserted", infinite_source->Get(2) == 111);
    CHECK("finite insert into infinite shifted", infinite_source->Get(4) == 2);
    delete infinite_source;

    int finite_source_data[] = { 10, 20, 30 };
    LazySequence<int> finite_source(finite_source_data, 3);
    LazySequence<int>* infinite_inserted = CreateNaturals();

    LazySequence<int>* infinite_insert_result = finite_source.InsertSequenceAt(*infinite_inserted, 1);
    CHECK("infinite inserted prefix", infinite_insert_result->Get(0) == 10);
    CHECK("infinite inserted 0", infinite_insert_result->Get(1) == 0);
    CHECK("infinite inserted 10", infinite_insert_result->Get(11) == 10);
    CHECK("infinite inserted omega 0", infinite_insert_result->Get(TransfiniteIndex::AfterInfinity(0)) == 20);
    CHECK("infinite inserted omega 1", infinite_insert_result->Get(TransfiniteIndex::AfterInfinity(1)) == 30);
    CHECK_THROWS("infinite inserted omega 2 throws", infinite_insert_result->Get(TransfiniteIndex::AfterInfinity(2)));
    delete infinite_insert_result;
    delete infinite_inserted;

    LazySequence<int>* trans_source = CreateNaturals();
    LazySequence<int>* trans_result = trans_source->InsertSequenceAt(inserted, TransfiniteIndex::AfterInfinity(0));
    CHECK("insert at omega ordinary", trans_result->Get(5) == 5);
    CHECK("insert at omega 0", trans_result->Get(TransfiniteIndex::AfterInfinity(0)) == 111);
    CHECK("insert at omega 1", trans_result->Get(TransfiniteIndex::AfterInfinity(1)) == 222);
    delete trans_result;
    delete trans_source;
    delete naturals;

    CHECK_THROWS("insert sequence negative throws", source.InsertSequenceAt(inserted, -1));
    CHECK_THROWS("insert sequence too far throws", source.InsertSequenceAt(inserted, 5));
    CHECK_THROWS("insert at omega into finite throws", source.InsertSequenceAt(inserted, TransfiniteIndex::AfterInfinity(0)));
}

void test_CopyAssignmentForGeneratedSequences() {
    SUITE("Copy and assignment for generated sequences");

    LazySequence<int>* fib = CreateFibonacciFromOnes();

    Sequence<int>* first = fib->Append(0);
    LazySequence<int>* lazy_first = dynamic_cast<LazySequence<int>*>(first);
    Sequence<int>* second = lazy_first->Append(7);
    LazySequence<int>* lazy_second = dynamic_cast<LazySequence<int>*>(second);

    LazySequence<int> copied(*lazy_second);
    CHECK("copy ordinary", copied.Get(5) == 8);
    CHECK("copy omega 0", copied.Get(TransfiniteIndex::AfterInfinity(0)) == 0);
    CHECK("copy omega 1", copied.Get(TransfiniteIndex::AfterInfinity(1)) == 7);

    LazySequence<int> assigned;
    assigned = *lazy_second;
    CHECK("assignment ordinary", assigned.Get(5) == 8);
    CHECK("assignment omega 0", assigned.Get(TransfiniteIndex::AfterInfinity(0)) == 0);
    CHECK("assignment omega 1", assigned.Get(TransfiniteIndex::AfterInfinity(1)) == 7);

    delete second;
    delete first;
    delete fib;
}

void test_SequenceReadOnlyStream() {
    SUITE("SequenceReadOnlyStream");

    int data[] = { 10, 20, 30 };
    MutableArraySequence<int> sequence(data, 3);
    SequenceReadOnlyStream<int> stream(&sequence);

    Stream<int>* base = &stream;
    ReadOnlyStream<int>* reader = &stream;

    CHECK_THROWS("read before open throws", reader->Read());
    base->Open();
    CHECK("can seek", reader->IsCanSeek());
    CHECK("can go back", reader->IsCanGoBack());
    CHECK("position open", base->GetPosition() == 0);
    CHECK("read 0", reader->Read() == 10);
    CHECK("position 1", base->GetPosition() == 1);
    CHECK("read 1", reader->Read() == 20);
    CHECK("seek 0", reader->Seek(0) == 0);
    CHECK("read after seek", reader->Read() == 10);
    CHECK("seek end", reader->Seek(3) == 3);
    CHECK("end after seek end", reader->IsEndOfStream());
    CHECK_THROWS("seek negative throws", reader->Seek(-1));
    CHECK_THROWS("seek too far throws", reader->Seek(4));
    CHECK_THROWS("read at end throws", reader->Read());
    base->Close();
    CHECK_THROWS("read after close throws", reader->Read());
}

void test_LazyReadOnlyStream() {
    SUITE("LazyReadOnlyStream");

    LazySequence<int>* naturals = CreateNaturals();
    LazyReadOnlyStream<int> stream(naturals);

    Stream<int>* base = &stream;
    ReadOnlyStream<int>* reader = &stream;

    CHECK_THROWS("lazy read before open throws", reader->Read());
    base->Open();
    CHECK("position open", base->GetPosition() == 0);
    CHECK("read 0", reader->Read() == 0);
    CHECK("read 1", reader->Read() == 1);
    CHECK("position 2", base->GetPosition() == 2);
    CHECK("seek 10", reader->Seek(10) == 10);
    CHECK("read 10", reader->Read() == 10);
    CHECK_THROWS("seek negative throws", reader->Seek(-1));
    base->Close();
    delete naturals;
}

void test_SequenceWriteOnlyStream() {
    SUITE("SequenceWriteOnlyStream");

    Sequence<int>* sequence = new MutableArraySequence<int>();
    SequenceWriteOnlyStream<int> stream(sequence);

    Stream<int>* base = &stream;
    WriteOnlyStream<int>* writer = &stream;

    CHECK_THROWS("write before open throws", writer->Write(10));

    base->Open();

    CHECK("position open", base->GetPosition() == 0);
    CHECK("write first", writer->Write(100) == 1);
    CHECK("write second", writer->Write(200) == 2);
    CHECK("position after writes", base->GetPosition() == 2);

    base->Close();

    CHECK("sequence length", sequence->GetLength() == 2);
    CHECK("sequence value 0", sequence->Get(0) == 100);
    CHECK("sequence value 1", sequence->Get(1) == 200);

    CHECK_THROWS("write after close throws", writer->Write(300));

    delete sequence;
}

void test_FileLineReadOnlyStream() {
    SUITE("FileLineReadOnlyStream");

    const char* filename = "test_file_read_stream.txt";
    {
        std::ofstream output(filename);
        output << "START\n";
        output << "MEASURE 10\n";
        output << "ERROR sensor\n";
        output << "END\n";
    }

    FileLineReadOnlyStream stream(filename);
    CHECK_THROWS("read before open throws", stream.Read());

    stream.Open();
    CHECK("position open", stream.GetPosition() == 0);
    CHECK("cannot seek", !stream.IsCanSeek());
    CHECK("cannot go back", !stream.IsCanGoBack());
    CHECK_THROWS("seek throws", stream.Seek(0));
    CHECK("read START", stream.Read() == "START");
    CHECK("position 1", stream.GetPosition() == 1);
    CHECK("read MEASURE", stream.Read() == "MEASURE 10");
    CHECK("read ERROR", stream.Read() == "ERROR sensor");
    CHECK("read END", stream.Read() == "END");
    CHECK("end after last", stream.IsEndOfStream());
    CHECK_THROWS("read after end throws", stream.Read());
    stream.Close();

    std::remove(filename);
    CHECK_THROWS("open missing file throws", FileLineReadOnlyStream("missing_file_for_test.txt").Open());
}

void test_FileLineWriteOnlyStream() {
    SUITE("FileLineWriteOnlyStream");

    const char* filename = "test_file_write_stream.txt";

    {
        FileLineWriteOnlyStream stream(filename);
        Stream<std::string>* base = &stream;
        WriteOnlyStream<std::string>* writer = &stream;

        CHECK_THROWS("write before open throws", writer->Write("bad"));
        base->Open();
        CHECK("position open", base->GetPosition() == 0);
        CHECK("write START", writer->Write("START") == 1);
        CHECK("write MEASURE", writer->Write("MEASURE 10") == 2);
        CHECK("write END", writer->Write("END") == 3);
        base->Close();
        CHECK_THROWS("write after close throws", writer->Write("bad"));
    }

    {
        FileLineReadOnlyStream read_stream(filename);
        read_stream.Open();
        CHECK("read START written", read_stream.Read() == "START");
        CHECK("read MEASURE written", read_stream.Read() == "MEASURE 10");
        CHECK("read END written", read_stream.Read() == "END");
        CHECK("read written end", read_stream.IsEndOfStream());
        read_stream.Close();
    }

    std::remove(filename);
}

void test_EventParser() {
    SUITE("EventParser");

    Event<double> start = EventParser<double>::ParseLine("START");
    CHECK("parse START", start.type == EventType::Start);
    Event<double> end = EventParser<double>::ParseLine("END");
    CHECK("parse END", end.type == EventType::End);
    Event<double> measure = EventParser<double>::ParseLine("MEASURE 12.5");
    CHECK("parse MEASURE type", measure.type == EventType::Measure);
    CHECK_NEAR("parse MEASURE value", measure.value, 12.5);
    Event<double> measure_spaces = EventParser<double>::ParseLine("   MEASURE   7.25   ");
    CHECK("parse MEASURE spaces type", measure_spaces.type == EventType::Measure);
    CHECK_NEAR("parse MEASURE spaces value", measure_spaces.value, 7.25);
    Event<double> bad_measure = EventParser<double>::ParseLine("MEASURE abc");
    CHECK("bad MEASURE unknown", bad_measure.type == EventType::Unknown);
    Event<double> bad_measure_tail = EventParser<double>::ParseLine("MEASURE 10 abc");
    CHECK("bad MEASURE tail unknown", bad_measure_tail.type == EventType::Unknown);
    Event<double> error = EventParser<double>::ParseLine("ERROR sensor disconnected");
    CHECK("parse ERROR type", error.type == EventType::Error);
    CHECK("parse ERROR message", error.message == "sensor disconnected");
    Event<double> error_empty = EventParser<double>::ParseLine("ERROR");
    CHECK("parse empty ERROR type", error_empty.type == EventType::Error);
    CHECK("parse empty ERROR message", error_empty.message == "");
    Event<double> unknown = EventParser<double>::ParseLine("BAD DATA");
    CHECK("parse unknown", unknown.type == EventType::Unknown);
}

void test_EventReadOnlyStream() {
    SUITE("EventReadOnlyStream");

    std::string lines_data[] = { "START", "MEASURE 10", "ERROR sensor", "END" };
    MutableArraySequence<std::string> lines(lines_data, 4);
    SequenceReadOnlyStream<std::string> line_stream(&lines);
    EventReadOnlyStream<double> event_stream(&line_stream);

    CHECK_THROWS("event read before open throws", event_stream.Read());
    event_stream.Open();
    Event<double> first = event_stream.Read();
    CHECK("first START", first.type == EventType::Start);
    Event<double> second = event_stream.Read();
    CHECK("second MEASURE", second.type == EventType::Measure);
    CHECK_NEAR("second value", second.value, 10.0);
    Event<double> third = event_stream.Read();
    CHECK("third ERROR", third.type == EventType::Error);
    CHECK("third message", third.message == "sensor");
    Event<double> fourth = event_stream.Read();
    CHECK("fourth END", fourth.type == EventType::End);
    CHECK("event stream end", event_stream.IsEndOfStream());
    CHECK_THROWS("event read after end throws", event_stream.Read());
    event_stream.Close();
}

void test_OnlineEventStatistics() {
    SUITE("OnlineEventStatistics");

    OnlineEventStatistics<double> stats;
    CHECK("initial total", stats.GetTotalEvents() == 0);
    CHECK("initial no measurements", !stats.HasMeasurements());
    CHECK_THROWS("min without measurements throws", stats.GetMinMeasure());
    CHECK_THROWS("max without measurements throws", stats.GetMaxMeasure());
    CHECK_THROWS("average without measurements throws", stats.GetAverageMeasure());
    CHECK_THROWS("variance without measurements throws", stats.GetVarianceMeasure());
    CHECK_THROWS("median without measurements throws", stats.GetMedianMeasure());

    stats.AddEvent(Event<double>(EventType::Start, 0.0, ""));
    stats.AddEvent(Event<double>(EventType::Measure, 10.0, ""));
    stats.AddEvent(Event<double>(EventType::Measure, 20.0, ""));
    stats.AddEvent(Event<double>(EventType::Error, 0.0, "sensor"));
    stats.AddEvent(Event<double>(EventType::Unknown, 0.0, "bad"));
    stats.AddEvent(Event<double>(EventType::Measure, 30.0, ""));
    stats.AddEvent(Event<double>(EventType::End, 0.0, ""));

    CHECK("total events", stats.GetTotalEvents() == 7);
    CHECK("start events", stats.GetStartEvents() == 1);
    CHECK("measure events", stats.GetMeasureEvents() == 3);
    CHECK("error events", stats.GetErrorEvents() == 1);
    CHECK("unknown events", stats.GetUnknownEvents() == 1);
    CHECK("end events", stats.GetEndEvents() == 1);
    CHECK("has measurements", stats.HasMeasurements());
    CHECK_NEAR("min", stats.GetMinMeasure(), 10.0);
    CHECK_NEAR("max", stats.GetMaxMeasure(), 30.0);
    CHECK_NEAR("average", stats.GetAverageMeasure(), 20.0);
    CHECK_NEAR("variance", stats.GetVarianceMeasure(), 200.0 / 3.0);
    CHECK_NEAR("median", stats.GetMedianMeasure(), 20.0);

    stats.AddEvent(Event<double>(EventType::Measure, 40.0, ""));
    CHECK_NEAR("median even", stats.GetMedianMeasure(), 25.0);
    CHECK_NEAR("variance even", stats.GetVarianceMeasure(), 125.0);

    stats.Clear();
    CHECK("clear total", stats.GetTotalEvents() == 0);
    CHECK("clear no measurements", !stats.HasMeasurements());
    CHECK_THROWS("median after clear throws", stats.GetMedianMeasure());
}

void test_ProtocolStatisticsTaskWithStringStream() {
    SUITE("ProtocolStatisticsTask with string stream");

    std::string lines_data[] = { "START", "MEASURE 10", "MEASURE 20", "ERROR sensor", "MEASURE 30", "END" };
    MutableArraySequence<std::string> lines(lines_data, 6);
    SequenceReadOnlyStream<std::string> line_stream(&lines);

    OnlineEventStatistics<double> stats = ProtocolStatisticsTask<double>::Process(line_stream);

    CHECK("protocol total", stats.GetTotalEvents() == 6);
    CHECK("protocol start", stats.GetStartEvents() == 1);
    CHECK("protocol measure", stats.GetMeasureEvents() == 3);
    CHECK("protocol error", stats.GetErrorEvents() == 1);
    CHECK("protocol end", stats.GetEndEvents() == 1);
    CHECK_NEAR("protocol average", stats.GetAverageMeasure(), 20.0);
    CHECK_NEAR("protocol median", stats.GetMedianMeasure(), 20.0);
    CHECK_NEAR("protocol variance", stats.GetVarianceMeasure(), 200.0 / 3.0);
}

void test_ProtocolStatisticsTaskWithEventStream() {
    SUITE("ProtocolStatisticsTask with event stream");

    Event<double> events_data[] = {
        Event<double>(EventType::Start, 0.0, ""),
        Event<double>(EventType::Measure, 1.0, ""),
        Event<double>(EventType::Measure, 2.0, ""),
        Event<double>(EventType::Measure, 3.0, ""),
        Event<double>(EventType::End, 0.0, "")
    };

    MutableArraySequence<Event<double>> events(events_data, 5);
    SequenceReadOnlyStream<Event<double>> event_stream(&events);

    OnlineEventStatistics<double> stats = ProtocolStatisticsTask<double>::Process(event_stream);

    CHECK("event protocol total", stats.GetTotalEvents() == 5);
    CHECK("event protocol measures", stats.GetMeasureEvents() == 3);
    CHECK_NEAR("event protocol average", stats.GetAverageMeasure(), 2.0);
    CHECK_NEAR("event protocol median", stats.GetMedianMeasure(), 2.0);
    CHECK_NEAR("event protocol variance", stats.GetVarianceMeasure(), 2.0 / 3.0);
}

void test_ProtocolStatisticsTaskWithFile() {
    SUITE("ProtocolStatisticsTask with file");

    const char* filename = "test_protocol_file_task.txt";

    {
        FileLineWriteOnlyStream writer(filename);
        writer.Open();
        writer.Write("START");
        writer.Write("MEASURE 5");
        writer.Write("MEASURE 15");
        writer.Write("MEASURE 25");
        writer.Write("END");
        writer.Close();
    }

    FileLineReadOnlyStream file_stream(filename);
    OnlineEventStatistics<double> stats = ProtocolStatisticsTask<double>::Process(file_stream);

    CHECK("file protocol total", stats.GetTotalEvents() == 5);
    CHECK("file protocol measures", stats.GetMeasureEvents() == 3);
    CHECK_NEAR("file protocol average", stats.GetAverageMeasure(), 15.0);
    CHECK_NEAR("file protocol median", stats.GetMedianMeasure(), 15.0);
    CHECK_NEAR("file protocol variance", stats.GetVarianceMeasure(), 200.0 / 3.0);

    std::remove(filename);
}

void run_all_tests() {
    test_CardinalAndCardinalIO();
    test_TransfiniteIndex();

    test_LazySequenceBasic();
    test_RuleGeneratorAndMemoization();
    test_LazySubsequenceAndEnumerator();
    test_SequenceGeneratorDirectly();

    test_AppendGenerator();
    test_SeveralAppendsToInfinite();
    test_PrependGenerator();
    test_InsertItemGenerator();
    test_ConcatGenerator();
    test_InsertSequenceGenerator();
    test_CopyAssignmentForGeneratedSequences();

    test_SequenceReadOnlyStream();
    test_LazyReadOnlyStream();
    test_SequenceWriteOnlyStream();
    test_FileLineReadOnlyStream();
    test_FileLineWriteOnlyStream();

    test_EventParser();
    test_EventReadOnlyStream();
    test_OnlineEventStatistics();

    test_ProtocolStatisticsTaskWithStringStream();
    test_ProtocolStatisticsTaskWithEventStream();
    test_ProtocolStatisticsTaskWithFile();

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
