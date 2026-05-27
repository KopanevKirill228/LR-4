#include <iostream>
#include <stdexcept>
#include <cmath>
#include <string>
#include <fstream>
#include <cstdio>

#include "lib/ArraySequence.h"

#include "lazy/Cardinal.h"
#include "lazy/TransfiniteIndex.h"
#include "lazy/Generator.h"
#include "lazy/RuleGenerator.h"
#include "lazy/SequenceGenerator.h"
#include "lazy/LazySequence.h"

#include "streams/Stream.h"
#include "streams/ReadOnlyStream.h"
#include "streams/WriteOnlyStream.h"
#include "streams/SequenceReadOnlyStream.h"
#include "streams/LazyReadOnlyStream.h"
#include "streams/FileLineReadOnlyStream.h"

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

    if (diff < 0) {
        diff = -diff;
    }

    return diff < 0.000001;
}


#define CHECK(desc, expr) \
    do { \
        if (expr) { ok(desc); } \
        else { fail(desc, __FILE__, __LINE__, #expr); } \
    } while (0)


#define CHECK_NEAR(desc, first, second) \
    do { \
        if (near((first), (second))) { ok(desc); } \
        else { fail(desc, __FILE__, __LINE__, #first " near " #second); } \
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


template <class T>
class TestWriteOnlyStream : public WriteOnlyStream<T> {
private:
    MutableArraySequence<T> data_;
    int position_;
    bool is_open_;

public:
    TestWriteOnlyStream()
        : data_(),
        position_(0),
        is_open_(false) {
    }

    void Open() override {
        is_open_ = true;
        position_ = 0;
    }

    void Close() override {
        is_open_ = false;
    }

    int GetPosition() const override {
        return position_;
    }

    int Write(const T& item) override {
        if (!is_open_) {
            throw std::logic_error("TestWriteOnlyStream: stream is closed");
        }

        Sequence<T>* result = data_.Append(item);

        if (result == nullptr) {
            throw std::runtime_error("TestWriteOnlyStream: append returned nullptr");
        }

        ++position_;

        return position_;
    }

    int GetCount() const {
        return data_.GetLength();
    }

    const T& Get(int index) const {
        return data_.Get(index);
    }
};


void test_Cardinal() {
    SUITE("Cardinal");

    Cardinal finite = Cardinal::Finite(5);
    Cardinal infinity = Cardinal::Infinity();

    CHECK("finite is finite", finite.IsFinite());
    CHECK("finite is not infinite", !finite.IsInfinite());
    CHECK("finite value", finite.GetFiniteValue() == 5);

    CHECK("infinity is infinite", infinity.IsInfinite());
    CHECK("infinity is not finite", !infinity.IsFinite());
    CHECK_THROWS("infinity finite value throws", infinity.GetFiniteValue());

    CHECK_THROWS("negative finite cardinal throws", Cardinal::Finite(-1));
}


void test_TransfiniteIndex() {
    SUITE("TransfiniteIndex");

    TransfiniteIndex finite = TransfiniteIndex::Finite(7);
    TransfiniteIndex omega = TransfiniteIndex::AfterInfinity(2);
    TransfiniteIndex two_omega(2, 3);

    CHECK("finite index is finite", finite.IsFinite());
    CHECK("finite index not after infinity", !finite.IsAfterInfinity());
    CHECK("finite infinity count", finite.GetInfinityCount() == 0);
    CHECK("finite value", finite.GetFiniteIndex() == 7);

    CHECK("omega index is not finite", !omega.IsFinite());
    CHECK("omega index after infinity", omega.IsAfterInfinity());
    CHECK("omega infinity count", omega.GetInfinityCount() == 1);
    CHECK("omega finite part", omega.GetFiniteIndex() == 2);

    CHECK("two omega infinity count", two_omega.GetInfinityCount() == 2);
    CHECK("two omega finite part", two_omega.GetFiniteIndex() == 3);

    CHECK_THROWS("negative finite index throws", TransfiniteIndex::Finite(-1));
    CHECK_THROWS("negative after infinity throws", TransfiniteIndex::AfterInfinity(-1));
    CHECK_THROWS("negative infinity count throws", TransfiniteIndex(-1, 0));
    CHECK_THROWS("negative finite part throws", TransfiniteIndex(1, -1));
}


void test_LazySequenceConstructors() {
    SUITE("LazySequence constructors");

    LazySequence<int> empty;

    CHECK("empty length", empty.GetLength() == 0);
    CHECK("empty cardinality finite", empty.GetCardinality().IsFinite());
    CHECK("empty cardinality value", empty.GetCardinality().GetFiniteValue() == 0);
    CHECK("empty materialized count", empty.GetMaterializedCount() == 0);
    CHECK("empty is not infinite", !empty.IsInfinite());

    CHECK_THROWS("empty GetFirst throws", empty.GetFirst());
    CHECK_THROWS("empty GetLast throws", empty.GetLast());
    CHECK_THROWS("empty Get throws", empty.Get(0));

    int data[] = { 10, 20, 30 };
    LazySequence<int> finite(data, 3);

    CHECK("finite length", finite.GetLength() == 3);
    CHECK("finite first", finite.GetFirst() == 10);
    CHECK("finite last", finite.GetLast() == 30);
    CHECK("finite get", finite.Get(1) == 20);
    CHECK("finite operator[]", finite[2] == 30);
    CHECK("finite materialized", finite.GetMaterializedCount() == 3);

    CHECK_THROWS("constructor negative count throws", LazySequence<int>(data, -1));
    CHECK_THROWS("constructor null items with positive count throws", LazySequence<int>(nullptr, 1));

    LazySequence<int> null_empty(nullptr, 0);
    CHECK("null empty length", null_empty.GetLength() == 0);

    LazySequence<int> copied(finite);
    CHECK("copy length", copied.GetLength() == 3);
    CHECK("copy value", copied.Get(2) == 30);

    LazySequence<int> assigned;
    assigned = finite;

    CHECK("assigned length", assigned.GetLength() == 3);
    CHECK("assigned value", assigned.Get(1) == 20);

    assigned = assigned;
    CHECK("self assignment keeps value", assigned.Get(1) == 20);
}


void test_LazyRuleSequence() {
    SUITE("LazySequence rule generator");

    LazySequence<int>* naturals = CreateNaturals();

    CHECK("naturals infinite", naturals->IsInfinite());
    CHECK("naturals cardinality infinite", naturals->GetCardinality().IsInfinite());
    CHECK_THROWS("naturals length throws", naturals->GetLength());
    CHECK_THROWS("naturals last throws", naturals->GetLast());

    CHECK("naturals 0", naturals->Get(0) == 0);
    CHECK("naturals 1", naturals->Get(1) == 1);
    CHECK("naturals 10", naturals->Get(10) == 10);

    int after_first_materialization = naturals->GetMaterializedCount();
    naturals->Get(10);
    CHECK("repeated get does not rematerialize", naturals->GetMaterializedCount() == after_first_materialization);

    CHECK_THROWS("negative get throws", naturals->Get(-1));

    delete naturals;

    int empty_data[] = {};
    MutableArraySequence<int> empty_init(empty_data, 0);

    CHECK_THROWS(
        "rule sequence with empty initial values throws",
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
    CHECK("subsequence 1", lazy_sub->Get(1) == 30);
    CHECK("subsequence 2", lazy_sub->Get(2) == 40);

    delete sub;

    CHECK_THROWS("subsequence negative start throws", finite.GetSubsequence(-1, 2));
    CHECK_THROWS("subsequence negative end throws", finite.GetSubsequence(1, -2));
    CHECK_THROWS("subsequence end before start throws", finite.GetSubsequence(3, 1));
    CHECK_THROWS("subsequence too far throws", finite.GetSubsequence(1, 5));

    IEnumerator<int>* en = finite.GetEnumerator();

    CHECK("enumerator move 1", en->MoveNext());
    CHECK("enumerator current 1", en->GetCurrent() == 10);
    CHECK("enumerator move 2", en->MoveNext());
    CHECK("enumerator current 2", en->GetCurrent() == 20);

    en->Reset();
    CHECK_THROWS("enumerator current after reset throws", en->GetCurrent());

    int count = 0;
    while (en->MoveNext()) {
        ++count;
    }

    CHECK("enumerator reached end after reset", count == 5);
    CHECK_THROWS("enumerator current after end throws", en->GetCurrent());

    delete en;
}


void test_AppendGeneratorFiniteAndInfinite() {
    SUITE("AppendGenerator");

    int data[] = { 10, 20, 30 };
    LazySequence<int> finite(data, 3);

    Sequence<int>* finite_appended = finite.Append(40);
    LazySequence<int>* lazy_finite_appended = dynamic_cast<LazySequence<int>*>(finite_appended);

    CHECK("append finite result lazy", lazy_finite_appended != nullptr);
    CHECK("append finite length", lazy_finite_appended->GetLength() == 4);
    CHECK("append finite old", lazy_finite_appended->Get(2) == 30);
    CHECK("append finite new", lazy_finite_appended->Get(3) == 40);
    CHECK_THROWS("append finite omega throws", lazy_finite_appended->Get(TransfiniteIndex::AfterInfinity(0)));

    delete finite_appended;

    LazySequence<int>* fib = CreateFibonacciFromOnes();

    Sequence<int>* infinite_appended = fib->Append(0);
    LazySequence<int>* lazy_infinite_appended = dynamic_cast<LazySequence<int>*>(infinite_appended);

    CHECK("append infinite result lazy", lazy_infinite_appended != nullptr);
    CHECK("append infinite cardinality", lazy_infinite_appended->GetCardinality().IsInfinite());
    CHECK("append infinite ordinary 4", lazy_infinite_appended->Get(4) == 5);
    CHECK("append infinite ordinary 5 is 8", lazy_infinite_appended->Get(5) == 8);
    CHECK("append infinite omega", lazy_infinite_appended->Get(TransfiniteIndex::AfterInfinity(0)) == 0);
    CHECK_THROWS("append infinite omega plus one throws", lazy_infinite_appended->Get(TransfiniteIndex::AfterInfinity(1)));

    delete infinite_appended;
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
    CHECK_THROWS("omega after last append throws", lazy_third->Get(TransfiniteIndex::AfterInfinity(3)));

    delete third;
    delete second;
    delete first;
    delete fib;
}


void test_PrependGenerator() {
    SUITE("PrependGenerator");

    int data[] = { 10, 20, 30 };
    LazySequence<int> finite(data, 3);

    Sequence<int>* result = finite.Prepend(5);
    LazySequence<int>* lazy_result = dynamic_cast<LazySequence<int>*>(result);

    CHECK("prepend finite result lazy", lazy_result != nullptr);
    CHECK("prepend finite length", lazy_result->GetLength() == 4);
    CHECK("prepend finite first", lazy_result->Get(0) == 5);
    CHECK("prepend finite shifted 0", lazy_result->Get(1) == 10);
    CHECK("prepend finite shifted 2", lazy_result->Get(3) == 30);

    delete result;

    LazySequence<int>* naturals = CreateNaturals();

    Sequence<int>* infinite_result = naturals->Prepend(-1);
    LazySequence<int>* lazy_infinite_result = dynamic_cast<LazySequence<int>*>(infinite_result);

    CHECK("prepend infinite result lazy", lazy_infinite_result != nullptr);
    CHECK("prepend infinite cardinality", lazy_infinite_result->GetCardinality().IsInfinite());
    CHECK("prepend infinite first", lazy_infinite_result->Get(0) == -1);
    CHECK("prepend infinite shifted 0", lazy_infinite_result->Get(1) == 0);
    CHECK("prepend infinite shifted 5", lazy_infinite_result->Get(6) == 5);

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
    CHECK("insert end value", lazy_end->Get(3) == 40);

    delete end;

    CHECK_THROWS("insert negative throws", finite.InsertAt(0, -1));
    CHECK_THROWS("insert too far throws", finite.InsertAt(0, 4));

    LazySequence<int>* naturals = CreateNaturals();

    Sequence<int>* infinite_insert = naturals->InsertAt(100, 2);
    LazySequence<int>* lazy_infinite_insert = dynamic_cast<LazySequence<int>*>(infinite_insert);

    CHECK("insert infinite ordinary 0", lazy_infinite_insert->Get(0) == 0);
    CHECK("insert infinite ordinary 1", lazy_infinite_insert->Get(1) == 1);
    CHECK("insert infinite inserted", lazy_infinite_insert->Get(2) == 100);
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

    CHECK("concat finite finite result lazy", lazy_finite_finite != nullptr);
    CHECK("concat finite finite length", lazy_finite_finite->GetLength() == 5);
    CHECK("concat finite finite 0", lazy_finite_finite->Get(0) == 10);
    CHECK("concat finite finite 4", lazy_finite_finite->Get(4) == 50);

    delete finite_finite;

    LazySequence<int>* naturals = CreateNaturals();

    Sequence<int>* finite_infinite = left.Concat(*naturals);
    LazySequence<int>* lazy_finite_infinite = dynamic_cast<LazySequence<int>*>(finite_infinite);

    CHECK("concat finite infinite cardinality", lazy_finite_infinite->GetCardinality().IsInfinite());
    CHECK("concat finite infinite left", lazy_finite_infinite->Get(0) == 10);
    CHECK("concat finite infinite right 0", lazy_finite_infinite->Get(2) == 0);
    CHECK("concat finite infinite right 5", lazy_finite_infinite->Get(7) == 5);

    delete finite_infinite;

    Sequence<int>* infinite_finite = naturals->Concat(right);
    LazySequence<int>* lazy_infinite_finite = dynamic_cast<LazySequence<int>*>(infinite_finite);

    CHECK("concat infinite finite ordinary", lazy_infinite_finite->Get(5) == 5);
    CHECK("concat infinite finite omega 0", lazy_infinite_finite->Get(TransfiniteIndex::AfterInfinity(0)) == 30);
    CHECK("concat infinite finite omega 2", lazy_infinite_finite->Get(TransfiniteIndex::AfterInfinity(2)) == 50);
    CHECK_THROWS("concat infinite finite omega 3 throws", lazy_infinite_finite->Get(TransfiniteIndex::AfterInfinity(3)));

    delete infinite_finite;
    delete naturals;

    LazySequence<int>* first = CreateNaturals();
    LazySequence<int>* second = CreateFibonacciFromZeroOne();

    Sequence<int>* infinite_infinite = first->Concat(*second);
    LazySequence<int>* lazy_infinite_infinite = dynamic_cast<LazySequence<int>*>(infinite_infinite);

    CHECK("concat infinite infinite ordinary", lazy_infinite_infinite->Get(10) == 10);
    CHECK("concat infinite infinite omega 0", lazy_infinite_infinite->Get(TransfiniteIndex::AfterInfinity(0)) == 0);
    CHECK("concat infinite infinite omega 1", lazy_infinite_infinite->Get(TransfiniteIndex::AfterInfinity(1)) == 1);
    CHECK("concat infinite infinite omega 5", lazy_infinite_infinite->Get(TransfiniteIndex::AfterInfinity(5)) == 5);

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

    CHECK("insert sequence finite finite length", finite_finite->GetLength() == 6);
    CHECK("insert sequence finite finite 0", finite_finite->Get(0) == 10);
    CHECK("insert sequence finite finite inserted 0", finite_finite->Get(2) == 111);
    CHECK("insert sequence finite finite inserted 1", finite_finite->Get(3) == 222);
    CHECK("insert sequence finite finite shifted", finite_finite->Get(4) == 30);

    delete finite_finite;

    LazySequence<int>* naturals = CreateNaturals();

    LazySequence<int>* infinite_source = naturals->InsertSequenceAt(inserted, 2);

    CHECK("insert sequence into infinite cardinality", infinite_source->GetCardinality().IsInfinite());
    CHECK("insert sequence into infinite 0", infinite_source->Get(0) == 0);
    CHECK("insert sequence into infinite 1", infinite_source->Get(1) == 1);
    CHECK("insert sequence into infinite inserted 0", infinite_source->Get(2) == 111);
    CHECK("insert sequence into infinite shifted", infinite_source->Get(4) == 2);

    delete infinite_source;

    int finite_source_data[] = { 10, 20, 30 };
    LazySequence<int> finite_source(finite_source_data, 3);
    LazySequence<int>* infinite_inserted = CreateNaturals();

    LazySequence<int>* result_infinite_inserted =
        finite_source.InsertSequenceAt(*infinite_inserted, 1);

    CHECK("insert infinite inserted prefix", result_infinite_inserted->Get(0) == 10);
    CHECK("insert infinite inserted ordinary 0", result_infinite_inserted->Get(1) == 0);
    CHECK("insert infinite inserted ordinary 10", result_infinite_inserted->Get(11) == 10);
    CHECK("insert infinite inserted omega 0", result_infinite_inserted->Get(TransfiniteIndex::AfterInfinity(0)) == 20);
    CHECK("insert infinite inserted omega 1", result_infinite_inserted->Get(TransfiniteIndex::AfterInfinity(1)) == 30);
    CHECK_THROWS("insert infinite inserted omega 2 throws", result_infinite_inserted->Get(TransfiniteIndex::AfterInfinity(2)));

    delete result_infinite_inserted;
    delete infinite_inserted;

    LazySequence<int>* trans_source = CreateNaturals();

    LazySequence<int>* trans_result = trans_source->InsertSequenceAt(
        inserted,
        TransfiniteIndex::AfterInfinity(0)
    );

    CHECK("insert at omega ordinary", trans_result->Get(5) == 5);
    CHECK("insert at omega omega 0", trans_result->Get(TransfiniteIndex::AfterInfinity(0)) == 111);
    CHECK("insert at omega omega 1", trans_result->Get(TransfiniteIndex::AfterInfinity(1)) == 222);

    delete trans_result;
    delete trans_source;
    delete naturals;

    CHECK_THROWS("insert sequence negative throws", source.InsertSequenceAt(inserted, -1));
    CHECK_THROWS("insert sequence too far throws", source.InsertSequenceAt(inserted, 5));
    CHECK_THROWS("insert sequence at omega into finite throws",
        source.InsertSequenceAt(inserted, TransfiniteIndex::AfterInfinity(0)));
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


void test_SequenceGeneratorDirectly() {
    SUITE("SequenceGenerator directly");

    int data[] = { 10, 20, 30 };
    MutableArraySequence<int> sequence(data, 3);

    SequenceGenerator<int> generator(sequence);

    CHECK("sequence generator has next 0", generator.HasNext());
    CHECK("sequence generator next 0", generator.GetNext() == 10);
    CHECK("sequence generator position 1", generator.GetPosition() == 1);
    CHECK("sequence generator next 1", generator.GetNext() == 20);
    CHECK("sequence generator next 2", generator.GetNext() == 30);
    CHECK("sequence generator has no next", !generator.HasNext());
    CHECK_THROWS("sequence generator no next throws", generator.GetNext());

    generator.Reset();
    CHECK("sequence generator reset position", generator.GetPosition() == 0);
    CHECK("sequence generator after reset", generator.GetNext() == 10);

    Generator<int>* clone = generator.Clone();
    CHECK("sequence generator clone next", clone->GetNext() == 20);

    delete clone;
}


void test_StreamReadOnlyBase() {
    SUITE("Stream base with ReadOnlyStream");

    int data[] = { 10, 20, 30 };
    MutableArraySequence<int> sequence(data, 3);

    SequenceReadOnlyStream<int> read_stream(&sequence);

    Stream<int>* stream_base = &read_stream;
    ReadOnlyStream<int>* read_base = &read_stream;

    CHECK("read stream can seek", read_base->IsCanSeek());
    CHECK("read stream can go back", read_base->IsCanGoBack());

    stream_base->Open();

    CHECK("read stream position open", stream_base->GetPosition() == 0);
    CHECK("read stream first", read_base->Read() == 10);
    CHECK("read stream position 1", stream_base->GetPosition() == 1);
    CHECK("read stream second", read_base->Read() == 20);
    CHECK("read stream position 2", stream_base->GetPosition() == 2);

    CHECK("read stream seek 0", read_base->Seek(0) == 0);
    CHECK("read stream after seek", read_base->Read() == 10);

    CHECK_THROWS("read stream seek negative throws", read_base->Seek(-1));
    CHECK_THROWS("read stream seek too far throws", read_base->Seek(4));

    stream_base->Close();
}


void test_LazyReadOnlyStream() {
    SUITE("LazyReadOnlyStream");

    LazySequence<int>* naturals = CreateNaturals();
    LazyReadOnlyStream<int> stream(naturals);

    Stream<int>* stream_base = &stream;
    ReadOnlyStream<int>* read_base = &stream;

    stream_base->Open();

    CHECK("lazy stream position open", stream_base->GetPosition() == 0);
    CHECK("lazy stream read 0", read_base->Read() == 0);
    CHECK("lazy stream read 1", read_base->Read() == 1);
    CHECK("lazy stream position 2", stream_base->GetPosition() == 2);

    CHECK("lazy stream seek 10", read_base->Seek(10) == 10);
    CHECK("lazy stream read 10", read_base->Read() == 10);

    stream_base->Close();

    delete naturals;
}


void test_WriteOnlyStreamBase() {
    SUITE("Stream base with WriteOnlyStream");

    TestWriteOnlyStream<int> write_stream;

    Stream<int>* stream_base = &write_stream;
    WriteOnlyStream<int>* write_base = &write_stream;

    CHECK_THROWS("write before open throws", write_base->Write(1));

    stream_base->Open();

    CHECK("write stream position open", stream_base->GetPosition() == 0);
    CHECK("write first returns 1", write_base->Write(100) == 1);
    CHECK("write second returns 2", write_base->Write(200) == 2);
    CHECK("write stream position 2", stream_base->GetPosition() == 2);

    stream_base->Close();

    CHECK("write count", write_stream.GetCount() == 2);
    CHECK("write value 0", write_stream.Get(0) == 100);
    CHECK("write value 1", write_stream.Get(1) == 200);

    CHECK_THROWS("write after close throws", write_base->Write(300));
}


void test_FileLineReadOnlyStream() {
    SUITE("FileLineReadOnlyStream");

    const char* filename = "test_protocol_tmp.txt";

    {
        std::ofstream output(filename);
        output << "START\n";
        output << "MEASURE 10\n";
        output << "ERROR sensor\n";
        output << "END\n";
    }

    FileLineReadOnlyStream stream(filename);

    stream.Open();

    CHECK("file stream position open", stream.GetPosition() == 0);
    CHECK("file stream cannot seek", !stream.IsCanSeek());
    CHECK("file stream cannot go back", !stream.IsCanGoBack());
    CHECK_THROWS("file seek throws", stream.Seek(0));

    CHECK("file read START", stream.Read() == "START");
    CHECK("file position 1", stream.GetPosition() == 1);
    CHECK("file read MEASURE", stream.Read() == "MEASURE 10");
    CHECK("file read ERROR", stream.Read() == "ERROR sensor");
    CHECK("file read END", stream.Read() == "END");
    CHECK("file end", stream.IsEndOfStream());

    stream.Close();

    std::remove(filename);

    CHECK_THROWS("file open missing throws", FileLineReadOnlyStream("missing_file_123.txt").Open());
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
    CHECK("parse ERROR empty type", error_empty.type == EventType::Error);
    CHECK("parse ERROR empty message", error_empty.message == "");

    Event<double> unknown = EventParser<double>::ParseLine("BAD DATA");
    CHECK("parse UNKNOWN", unknown.type == EventType::Unknown);
}


void test_EventReadOnlyStream() {
    SUITE("EventReadOnlyStream");

    std::string lines_data[] = {
        "START",
        "MEASURE 10",
        "ERROR sensor",
        "END"
    };

    MutableArraySequence<std::string> lines(lines_data, 4);
    SequenceReadOnlyStream<std::string> line_stream(&lines);
    EventReadOnlyStream<double> event_stream(&line_stream);

    event_stream.Open();

    Event<double> first = event_stream.Read();
    CHECK("event stream first START", first.type == EventType::Start);

    Event<double> second = event_stream.Read();
    CHECK("event stream second MEASURE", second.type == EventType::Measure);
    CHECK_NEAR("event stream second value", second.value, 10.0);

    Event<double> third = event_stream.Read();
    CHECK("event stream third ERROR", third.type == EventType::Error);
    CHECK("event stream third message", third.message == "sensor");

    Event<double> fourth = event_stream.Read();
    CHECK("event stream fourth END", fourth.type == EventType::End);

    CHECK("event stream end", event_stream.IsEndOfStream());

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
    CHECK_NEAR("min measure", stats.GetMinMeasure(), 10.0);
    CHECK_NEAR("max measure", stats.GetMaxMeasure(), 30.0);
    CHECK_NEAR("average measure", stats.GetAverageMeasure(), 20.0);
    CHECK_NEAR("variance measure", stats.GetVarianceMeasure(), 200.0 / 3.0);
    CHECK_NEAR("median measure", stats.GetMedianMeasure(), 20.0);

    stats.AddEvent(Event<double>(EventType::Measure, 40.0, ""));

    CHECK_NEAR("median even count", stats.GetMedianMeasure(), 25.0);
    CHECK_NEAR("variance even count", stats.GetVarianceMeasure(), 125.0);

    stats.Clear();

    CHECK("clear total", stats.GetTotalEvents() == 0);
    CHECK("clear no measurements", !stats.HasMeasurements());
    CHECK_THROWS("median after clear throws", stats.GetMedianMeasure());
}


void test_ProtocolStatisticsTaskWithStringStream() {
    SUITE("ProtocolStatisticsTask with string stream");

    std::string lines_data[] = {
        "START",
        "MEASURE 10",
        "MEASURE 20",
        "ERROR sensor",
        "MEASURE 30",
        "END"
    };

    MutableArraySequence<std::string> lines(lines_data, 6);
    SequenceReadOnlyStream<std::string> line_stream(&lines);

    OnlineEventStatistics<double> stats =
        ProtocolStatisticsTask<double>::Process(line_stream);

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

    OnlineEventStatistics<double> stats =
        ProtocolStatisticsTask<double>::Process(event_stream);

    CHECK("event protocol total", stats.GetTotalEvents() == 5);
    CHECK("event protocol measures", stats.GetMeasureEvents() == 3);
    CHECK_NEAR("event protocol average", stats.GetAverageMeasure(), 2.0);
    CHECK_NEAR("event protocol median", stats.GetMedianMeasure(), 2.0);
    CHECK_NEAR("event protocol variance", stats.GetVarianceMeasure(), 2.0 / 3.0);
}


void test_ProtocolStatisticsTaskWithFile() {
    SUITE("ProtocolStatisticsTask with file");

    const char* filename = "test_protocol_task_tmp.txt";

    {
        std::ofstream output(filename);
        output << "START\n";
        output << "MEASURE 5\n";
        output << "MEASURE 15\n";
        output << "MEASURE 25\n";
        output << "END\n";
    }

    FileLineReadOnlyStream file_stream(filename);

    OnlineEventStatistics<double> stats =
        ProtocolStatisticsTask<double>::Process(file_stream);

    CHECK("file protocol total", stats.GetTotalEvents() == 5);
    CHECK("file protocol measures", stats.GetMeasureEvents() == 3);
    CHECK_NEAR("file protocol average", stats.GetAverageMeasure(), 15.0);
    CHECK_NEAR("file protocol median", stats.GetMedianMeasure(), 15.0);
    CHECK_NEAR("file protocol variance", stats.GetVarianceMeasure(), 200.0 / 3.0);

    std::remove(filename);
}


void run_all_tests() {
    test_Cardinal();
    test_TransfiniteIndex();

    test_LazySequenceConstructors();
    test_LazyRuleSequence();
    test_LazySubsequenceAndEnumerator();

    test_AppendGeneratorFiniteAndInfinite();
    test_SeveralAppendsToInfinite();
    test_PrependGenerator();
    test_InsertItemGenerator();
    test_ConcatGenerator();
    test_InsertSequenceGenerator();

    test_CopyAssignmentForGeneratedSequences();

    test_SequenceGeneratorDirectly();

    test_StreamReadOnlyBase();
    test_LazyReadOnlyStream();
    test_WriteOnlyStreamBase();
    test_FileLineReadOnlyStream();

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