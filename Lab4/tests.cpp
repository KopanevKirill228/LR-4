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
#include "generators/Generator.h"
#include "generators/RuleGenerator.h"
#include "generators/SequenceGenerator.h"
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
#include "tasks/EventMapReadOnlyStream.h"
#include "tasks/OnlineEventStatistics.h"
#include "tasks/ProtocolStatisticsTask.h"

#include "event_batch_processing/EventBatchProcessingTask.h"
#include "live_event_processing/LiveFileWriter.h"
#include "live_event_processing/LiveFileBatchConsumer.h"

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

int NaturalRule(const Sequence<int>& source) {
    return source.GetLength();
}

int FibonacciRule(const Sequence<int>& source) {
    int length = source.GetLength();
    return source.Get(length - 1) + source.Get(length - 2);
}

double IdentityDoubleMapper(const double& value) {
    return value;
}


double DoubleMeasureValue(const double& value) {
    return value * 2.0;
}


double AddTenMapper(const double& value) {
    return value + 10.0;
}


double SquareMapper(const double& value) {
    return value * value;
}


static int live_message_count = 0;
static int live_line_read_count = 0;
static int live_batch_processed_count = 0;
static int live_finished_count = 0;


void ResetLiveMessageCounters() {
    live_message_count = 0;
    live_line_read_count = 0;
    live_batch_processed_count = 0;
    live_finished_count = 0;
}


void CountLiveConsumerMessage(const LiveConsumerMessage<double>& message) {
    ++live_message_count;

    if (message.type == LiveConsumerMessageType::LineRead) {
        ++live_line_read_count;
    }
    else if (message.type == LiveConsumerMessageType::BatchProcessed) {
        ++live_batch_processed_count;
    }
    else if (message.type == LiveConsumerMessageType::Finished) {
        ++live_finished_count;
    }
}

static void CheckFiniteValue(
    const char* desc,
    LazySequence<int>* sequence,
    int index,
    int expected
) {
    try {
        int value = sequence->Get(index);
        CHECK(desc, value == expected);
    }
    catch (const std::exception& e) {
        std::cout << "  [EXCEPTION] " << desc << ": " << e.what() << "\n";
        CHECK(desc, false);
    }
}

static void CheckTransfiniteValue(
    const char* desc,
    LazySequence<int>* sequence,
    const TransfiniteIndex& index,
    int expected
) {
    try {
        int value = sequence->Get(index);
        CHECK(desc, value == expected);
    }
    catch (const std::exception& e) {
        std::cout << "  [EXCEPTION] " << desc << ": " << e.what() << "\n";
        CHECK(desc, false);
    }
}

static LazySequence<int>* CreateNaturals() {
    int init_data[] = { 0 };
    MutableArraySequence<int> init(init_data, 1);
    return new LazySequence<int>(
        NaturalRule,
        init
    );
}

static LazySequence<int>* CreateThreeNaturalsConcat() {
    LazySequence<int>* first = CreateNaturals();
    LazySequence<int>* second = CreateNaturals();
    LazySequence<int>* third = CreateNaturals();

    LazySequence<int>* first_two = first->Concat(*second);
    LazySequence<int>* all_three = first_two->Concat(*third);

    delete first_two;
    delete third;
    delete second;
    delete first;

    return all_three;
}

static LazySequence<int>* CreateFibonacciFromZeroOne() {
    int init_data[] = { 0, 1 };
    MutableArraySequence<int> init(init_data, 2);
    return new LazySequence<int>(
        FibonacciRule,
        init
    );
}

static LazySequence<int>* CreateFibonacciFromOnes() {
    int init_data[] = { 1, 1 };
    MutableArraySequence<int> init(init_data, 2);
    return new LazySequence<int>(
        FibonacciRule,
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
            NaturalRule,
            empty_init
        )
    );
}

void test_LazySubsequenceAndEnumerator() {
    SUITE("LazySequence subsequence and enumerator");

    int data[] = { 10, 20, 30, 40, 50 };
    LazySequence<int> finite(data, 5);

    LazySequence<int>* sub = finite.GetSubsequence(1, 3);

    CHECK("subsequence length", sub->GetLength() == 3);
    CHECK("subsequence 0", sub->Get(0) == 20);
    CHECK("subsequence 2", sub->Get(2) == 40);
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

    LazySequence<int>* finite_result = finite.Append(40);

    CHECK("finite append length", finite_result->GetLength() == 4);
    CHECK("finite append old", finite_result->Get(2) == 30);
    CHECK("finite append new", finite_result->Get(3) == 40);
    CHECK_THROWS("finite append omega throws", finite_result->Get(TransfiniteIndex::AfterInfinity(0)));
    delete finite_result;

    LazySequence<int>* fib = CreateFibonacciFromOnes();
    LazySequence<int>* infinite_result = fib->Append(0);

    CHECK("infinite append cardinality infinite", infinite_result->GetCardinality().IsInfinite());
    CHECK("infinite append ordinary 4", infinite_result->Get(4) == 5);
    CHECK("infinite append ordinary 5 unaffected", infinite_result->Get(5) == 8);
    CHECK("infinite append omega 0", infinite_result->Get(TransfiniteIndex::AfterInfinity(0)) == 0);
    CHECK_THROWS("infinite append omega 1 throws", infinite_result->Get(TransfiniteIndex::AfterInfinity(1)));
    delete infinite_result;
    delete fib;
}

void test_SeveralAppendsToInfinite() {
    SUITE("Several appends to infinite");

    LazySequence<int>* fib = CreateFibonacciFromOnes();

    LazySequence<int>* first = fib->Append(0);
    LazySequence<int>* second = first->Append(7);
    LazySequence<int>* third = second->Append(9);

    CHECK("ordinary still fibonacci 4", third->Get(4) == 5);
    CHECK("ordinary still fibonacci 5", third->Get(5) == 8);
    CHECK("ordinary still fibonacci 6", third->Get(6) == 13);
    CHECK("omega first append", third->Get(TransfiniteIndex::AfterInfinity(0)) == 0);
    CHECK("omega second append", third->Get(TransfiniteIndex::AfterInfinity(1)) == 7);
    CHECK("omega third append", third->Get(TransfiniteIndex::AfterInfinity(2)) == 9);
    CHECK_THROWS("omega after last throws", third->Get(TransfiniteIndex::AfterInfinity(3)));

    delete third;
    delete second;
    delete first;
    delete fib;
}

void test_PrependGenerator() {
    SUITE("PrependGenerator");

    int data[] = { 10, 20, 30 };
    LazySequence<int> finite(data, 3);

    LazySequence<int>* finite_result = finite.Prepend(5);

    CHECK("finite prepend length", finite_result->GetLength() == 4);
    CHECK("finite prepend first", finite_result->Get(0) == 5);
    CHECK("finite prepend shifted", finite_result->Get(1) == 10);
    delete finite_result;

    LazySequence<int>* naturals = CreateNaturals();
    LazySequence<int>* infinite_result = naturals->Prepend(-1);

    CHECK("infinite prepend first", infinite_result->Get(0) == -1);
    CHECK("infinite prepend shifted 0", infinite_result->Get(1) == 0);
    CHECK("infinite prepend shifted 5", infinite_result->Get(6) == 5);
    delete infinite_result;
    delete naturals;
}

void test_InsertItemGenerator() {
    SUITE("InsertItemGenerator");

    int data[] = { 10, 20, 30 };
    LazySequence<int> finite(data, 3);

    LazySequence<int>* begin = finite.InsertAt(5, 0);
    CHECK("insert begin length", begin->GetLength() == 4);
    CHECK("insert begin value", begin->Get(0) == 5);
    CHECK("insert begin shifted", begin->Get(1) == 10);
    delete begin;

    LazySequence<int>* middle = finite.InsertAt(777, 1);
    CHECK("insert middle length", middle->GetLength() == 4);
    CHECK("insert middle before", middle->Get(0) == 10);
    CHECK("insert middle value", middle->Get(1) == 777);
    CHECK("insert middle shifted", middle->Get(2) == 20);
    delete middle;

    LazySequence<int>* end = finite.InsertAt(40, 3);
    CHECK("insert end length", end->GetLength() == 4);
    CHECK("insert end old last", end->Get(2) == 30);
    CHECK("insert end new last", end->Get(3) == 40);
    delete end;

    CHECK_THROWS("insert negative throws", finite.InsertAt(0, -1));
    CHECK_THROWS("insert too far throws", finite.InsertAt(0, 4));

    LazySequence<int>* naturals = CreateNaturals();
    LazySequence<int>* infinite_insert = naturals->InsertAt(100, 2);

    CHECK("insert infinite 0", infinite_insert->Get(0) == 0);
    CHECK("insert infinite 1", infinite_insert->Get(1) == 1);
    CHECK("insert infinite item", infinite_insert->Get(2) == 100);
    CHECK("insert infinite shifted", infinite_insert->Get(3) == 2);
    delete infinite_insert;
    delete naturals;
}

void test_ConcatGenerator() {
    SUITE("ConcatGenerator");

    int left_data[] = { 10, 20 };
    int right_data[] = { 30, 40, 50 };
    LazySequence<int> left(left_data, 2);
    LazySequence<int> right(right_data, 3);

    LazySequence<int>* finite_finite = left.Concat(right);
    CHECK("finite finite length", finite_finite->GetLength() == 5);
    CHECK("finite finite 0", finite_finite->Get(0) == 10);
    CHECK("finite finite 4", finite_finite->Get(4) == 50);
    delete finite_finite;

    LazySequence<int>* naturals = CreateNaturals();
    LazySequence<int>* finite_infinite = left.Concat(*naturals);
    CHECK("finite infinite cardinality", finite_infinite->GetCardinality().IsInfinite());
    CHECK("finite infinite left", finite_infinite->Get(0) == 10);
    CHECK("finite infinite right 0", finite_infinite->Get(2) == 0);
    CHECK("finite infinite right 5", finite_infinite->Get(7) == 5);
    delete finite_infinite;

    LazySequence<int>* infinite_finite = naturals->Concat(right);
    CHECK("infinite finite ordinary", infinite_finite->Get(5) == 5);
    CHECK("infinite finite omega 0", infinite_finite->Get(TransfiniteIndex::AfterInfinity(0)) == 30);
    CHECK("infinite finite omega 2", infinite_finite->Get(TransfiniteIndex::AfterInfinity(2)) == 50);
    CHECK_THROWS("infinite finite omega 3 throws", infinite_finite->Get(TransfiniteIndex::AfterInfinity(3)));
    delete infinite_finite;
    delete naturals;

    LazySequence<int>* first = CreateNaturals();
    LazySequence<int>* second = CreateFibonacciFromZeroOne();
    LazySequence<int>* infinite_infinite = first->Concat(*second);

    CHECK("infinite infinite ordinary", infinite_infinite->Get(10) == 10);
    CHECK("infinite infinite omega 0", infinite_infinite->Get(TransfiniteIndex::AfterInfinity(0)) == 0);
    CHECK("infinite infinite omega 1", infinite_infinite->Get(TransfiniteIndex::AfterInfinity(1)) == 1);
    CHECK("infinite infinite omega 5", infinite_infinite->Get(TransfiniteIndex::AfterInfinity(5)) == 5);
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

    LazySequence<int>* first = fib->Append(0);
    LazySequence<int>* second = first->Append(7);

    LazySequence<int> copied(*second);
    CHECK("copy ordinary", copied.Get(5) == 8);
    CHECK("copy omega 0", copied.Get(TransfiniteIndex::AfterInfinity(0)) == 0);
    CHECK("copy omega 1", copied.Get(TransfiniteIndex::AfterInfinity(1)) == 7);

    LazySequence<int> assigned;
    assigned = *second;
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

void test_EventMapReadOnlyStream() {
    SUITE("EventMapReadOnlyStream");

    Event<double> events_data[] = {
        Event<double>(EventType::Start, 0.0, ""),
        Event<double>(EventType::Measure, 10.0, ""),
        Event<double>(EventType::Error, 0.0, "sensor"),
        Event<double>(EventType::Measure, 20.0, ""),
        Event<double>(EventType::End, 0.0, "")
    };

    MutableArraySequence<Event<double>> events(events_data, 5);
    SequenceReadOnlyStream<Event<double>> event_stream(&events);

    EventMapReadOnlyStream<double> mapped_stream(
        &event_stream,
        DoubleMeasureValue
    );

    CHECK_THROWS("mapped read before open throws", mapped_stream.Read());

    mapped_stream.Open();

    Event<double> first = mapped_stream.Read();
    CHECK("mapped start unchanged", first.type == EventType::Start);

    Event<double> second = mapped_stream.Read();
    CHECK("mapped measure type", second.type == EventType::Measure);
    CHECK_NEAR("mapped measure value 1", second.value, 20.0);

    Event<double> third = mapped_stream.Read();
    CHECK("mapped error unchanged type", third.type == EventType::Error);
    CHECK("mapped error message", third.message == "sensor");

    Event<double> fourth = mapped_stream.Read();
    CHECK("mapped second measure type", fourth.type == EventType::Measure);
    CHECK_NEAR("mapped measure value 2", fourth.value, 40.0);

    Event<double> fifth = mapped_stream.Read();
    CHECK("mapped end unchanged", fifth.type == EventType::End);

    CHECK("mapped stream end", mapped_stream.IsEndOfStream());
    CHECK_THROWS("mapped read after end throws", mapped_stream.Read());

    mapped_stream.Close();

    CHECK_THROWS(
        "mapped stream nullptr source throws",
        EventMapReadOnlyStream<double>(nullptr, DoubleMeasureValue)
    );

    CHECK_THROWS(
        "mapped stream nullptr mapper throws",
        EventMapReadOnlyStream<double>(&event_stream, nullptr)
    );
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


void test_EventBatchProcessingTask() {
    SUITE("EventBatchProcessingTask");

    const char* filename = "test_event_batch_processing.txt";

    {
        FileLineWriteOnlyStream writer(filename);
        writer.Open();
        writer.Write("START");
        writer.Write("MEASURE 10");
        writer.Write("MEASURE 20");
        writer.Write("ERROR sensor");
        writer.Write("MEASURE 30");
        writer.Write("END");
        writer.Close();
    }

    {
        FileLineReadOnlyStream reader(filename);

        OnlineEventStatistics<double> stats =
            EventBatchProcessingTask<double>::Process(
                reader,
                2,
                DoubleMeasureValue
            );

        CHECK("batch map total", stats.GetTotalEvents() == 6);
        CHECK("batch map start", stats.GetStartEvents() == 1);
        CHECK("batch map measure", stats.GetMeasureEvents() == 3);
        CHECK("batch map error", stats.GetErrorEvents() == 1);
        CHECK("batch map end", stats.GetEndEvents() == 1);

        CHECK_NEAR("batch map min", stats.GetMinMeasure(), 20.0);
        CHECK_NEAR("batch map max", stats.GetMaxMeasure(), 60.0);
        CHECK_NEAR("batch map average", stats.GetAverageMeasure(), 40.0);
        CHECK_NEAR("batch map median", stats.GetMedianMeasure(), 40.0);
        CHECK_NEAR("batch map variance", stats.GetVarianceMeasure(), 800.0 / 3.0);
    }

    {
        FileLineReadOnlyStream reader(filename);

        OnlineEventStatistics<double> stats =
            EventBatchProcessingTask<double>::Process(
                reader,
                2
            );

        CHECK("batch no map total", stats.GetTotalEvents() == 6);
        CHECK("batch no map measures", stats.GetMeasureEvents() == 3);
        CHECK_NEAR("batch no map min", stats.GetMinMeasure(), 10.0);
        CHECK_NEAR("batch no map max", stats.GetMaxMeasure(), 30.0);
        CHECK_NEAR("batch no map average", stats.GetAverageMeasure(), 20.0);
        CHECK_NEAR("batch no map median", stats.GetMedianMeasure(), 20.0);
        CHECK_NEAR("batch no map variance", stats.GetVarianceMeasure(), 200.0 / 3.0);
    }

    {
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
            EventBatchProcessingTask<double>::Process(
                event_stream,
                2,
                AddTenMapper
            );

        CHECK("batch event stream total", stats.GetTotalEvents() == 5);
        CHECK("batch event stream measures", stats.GetMeasureEvents() == 3);
        CHECK_NEAR("batch event stream min", stats.GetMinMeasure(), 11.0);
        CHECK_NEAR("batch event stream max", stats.GetMaxMeasure(), 13.0);
        CHECK_NEAR("batch event stream average", stats.GetAverageMeasure(), 12.0);
        CHECK_NEAR("batch event stream median", stats.GetMedianMeasure(), 12.0);
        CHECK_NEAR("batch event stream variance", stats.GetVarianceMeasure(), 2.0 / 3.0);
    }

    {
        FileLineReadOnlyStream reader(filename);
        CHECK_THROWS(
            "batch size zero throws",
            EventBatchProcessingTask<double>::Process(reader, 0, DoubleMeasureValue)
        );
    }

    {
        FileLineReadOnlyStream reader(filename);
        CHECK_THROWS(
            "batch nullptr mapper throws",
            EventBatchProcessingTask<double>::Process(reader, 2, nullptr)
        );
    }

    std::remove(filename);
}


void test_LiveFileWriter() {
    SUITE("LiveFileWriter");

    const char* source_filename = "test_live_writer_source.txt";
    const char* live_filename = "test_live_writer_live.txt";

    {
        FileLineWriteOnlyStream writer(source_filename);
        writer.Open();
        writer.Write("START");
        writer.Write("MEASURE 10");
        writer.Write("END");
        writer.Close();
    }

    {
        FileLineWriteOnlyStream writer(live_filename);
        writer.Open();
        writer.Write("OLD");
        writer.Close();
    }

    std::ostringstream output;

    LiveFileWriter::Run(
        source_filename,
        live_filename,
        0,
        true,
        output
    );

    FileLineReadOnlyStream reader(live_filename);
    reader.Open();

    CHECK("live writer first line", reader.Read() == "START");
    CHECK("live writer second line", reader.Read() == "MEASURE 10");
    CHECK("live writer third line", reader.Read() == "END");
    CHECK("live writer end", reader.IsEndOfStream());

    reader.Close();

    CHECK("live writer output has text", output.str().size() > 0);

    std::remove(source_filename);
    std::remove(live_filename);
}


void test_LiveFileBatchConsumer() {
    SUITE("LiveFileBatchConsumer");

    const char* filename = "test_live_batch_consumer.txt";

    {
        FileLineWriteOnlyStream writer(filename);
        writer.Open();
        writer.Write("START");
        writer.Write("MEASURE 10");
        writer.Write("MEASURE 20");
        writer.Write("ERROR live");
        writer.Write("MEASURE 30");
        writer.Write("END");
        writer.Close();
    }

    ResetLiveMessageCounters();

    OnlineEventStatistics<double> stats =
        LiveFileBatchConsumer<double>::Run(
            filename,
            2,
            DoubleMeasureValue,
            CountLiveConsumerMessage
        );

    CHECK("live consumer total", stats.GetTotalEvents() == 6);
    CHECK("live consumer start", stats.GetStartEvents() == 1);
    CHECK("live consumer measure", stats.GetMeasureEvents() == 3);
    CHECK("live consumer error", stats.GetErrorEvents() == 1);
    CHECK("live consumer end", stats.GetEndEvents() == 1);

    CHECK_NEAR("live consumer min", stats.GetMinMeasure(), 20.0);
    CHECK_NEAR("live consumer max", stats.GetMaxMeasure(), 60.0);
    CHECK_NEAR("live consumer average", stats.GetAverageMeasure(), 40.0);
    CHECK_NEAR("live consumer median", stats.GetMedianMeasure(), 40.0);
    CHECK_NEAR("live consumer variance", stats.GetVarianceMeasure(), 800.0 / 3.0);

    CHECK("live consumer message count", live_message_count > 0);
    CHECK("live consumer read messages", live_line_read_count == 6);
    CHECK("live consumer batch messages", live_batch_processed_count == 3);
    CHECK("live consumer finished message", live_finished_count == 1);

    {
        CHECK_THROWS(
            "live consumer bad batch throws",
            LiveFileBatchConsumer<double>::Run(
                filename,
                0,
                DoubleMeasureValue,
                CountLiveConsumerMessage
            )
        );
    }

    {
        CHECK_THROWS(
            "live consumer nullptr mapper throws",
            LiveFileBatchConsumer<double>::Run(
                filename,
                2,
                nullptr,
                CountLiveConsumerMessage
            )
        );
    }

    std::remove(filename);
}

void test_ThreeConcat1() {
    SUITE("Three concat with transfinite indexes");

    LazySequence<int>* first = CreateNaturals();
    LazySequence<int>* second = CreateNaturals();
    LazySequence<int>* third = CreateNaturals();

    LazySequence<int>* result = first->Concat(*second)->Concat(*third);

    CHECK(
        "three concat omega + 0",
        result->Get(TransfiniteIndex::AfterInfinity(0)) == 0
    );

    CHECK(
        "three concat 2 omega + 0",
        result->Get(TransfiniteIndex(2, 0)) == 0
    );

    delete result;
    delete third;
    delete second;
    delete first;
}

void test_ThreeConcat() {
    SUITE("Three concat with transfinite indexes");

    LazySequence<int>* first = CreateNaturals();
    LazySequence<int>* second = CreateNaturals();
    LazySequence<int>* third = CreateNaturals();

    LazySequence<int>* first_two = first->Concat(*second);
    LazySequence<int>* all_three = first_two->Concat(*third);

    CHECK("three concat ordinary 0", all_three->Get(0) == 0);
    CHECK("three concat ordinary 5", all_three->Get(5) == 5);

    CHECK(
        "three concat omega + 0",
        all_three->Get(TransfiniteIndex::AfterInfinity(0)) == 0
    );

    CHECK(
        "three concat omega + 3",
        all_three->Get(TransfiniteIndex::AfterInfinity(3)) == 3
    );

    CHECK(
        "three concat 2 omega + 0",
        all_three->Get(TransfiniteIndex(2, 0)) == 0
    );

    CHECK(
        "three concat 2 omega + 3",
        all_three->Get(TransfiniteIndex(2, 3)) == 3
    );

    CHECK(
        "three concat 2 omega + 10",
        all_three->Get(TransfiniteIndex(2, 10)) == 10
    );

    delete all_three;
    delete first_two;
    delete third;
    delete second;
    delete first;
}

void test_InsertThreeConcatIntoFinite() {
    SUITE("Insert three concat into finite");

    int data[] = { 10, 20, 30 };
    LazySequence<int> finite(data, 3);

    LazySequence<int>* inserted = CreateThreeNaturalsConcat();

    LazySequence<int>* result = finite.InsertSequenceAt(*inserted, 1);

    CHECK("insert sequence finite prefix", result->Get(0) == 10);
    CHECK("insert sequence first infinite block", result->Get(1) == 0);
    CHECK("insert sequence first infinite block 5", result->Get(6) == 5);

    CHECK("insert sequence omega + 3",
        result->Get(TransfiniteIndex::AfterInfinity(3)) == 3);

    CHECK("insert sequence 2 omega + 3",
        result->Get(TransfiniteIndex(2, 3)) == 3);

    CHECK("insert sequence finite tail after 3 omega first",
        result->Get(TransfiniteIndex(3, 0)) == 20);

    CHECK("insert sequence finite tail after 3 omega second",
        result->Get(TransfiniteIndex(3, 1)) == 30);

    CHECK_THROWS("insert sequence after finite tail throws",
        result->Get(TransfiniteIndex(3, 2)));

    delete result;
    delete inserted;
}

void test_AllGeneratorsWithMultipleInfinities() {
    SUITE("All generators with multiple infinities");

    // 1. Base: N concat N concat N
    {
        LazySequence<int>* sequence = CreateThreeNaturalsConcat();

        CheckFiniteValue("base ordinary 0", sequence, 0, 0);
        CheckFiniteValue("base ordinary 5", sequence, 5, 5);

        CheckTransfiniteValue("base omega + 0", sequence, TransfiniteIndex::AfterInfinity(0), 0);
        CheckTransfiniteValue("base omega + 3", sequence, TransfiniteIndex::AfterInfinity(3), 3);

        CheckTransfiniteValue("base 2 omega + 0", sequence, TransfiniteIndex(2, 0), 0);
        CheckTransfiniteValue("base 2 omega + 3", sequence, TransfiniteIndex(2, 3), 3);

        CHECK_THROWS("base has no 3 omega", sequence->Get(TransfiniteIndex(3, 0)));

        delete sequence;
    }

    // 2. PrependGenerator over 3 infinities
    {
        LazySequence<int>* sequence = CreateThreeNaturalsConcat();
        LazySequence<int>* result = sequence->Prepend(-1);

        CheckFiniteValue("prepend finite 0", result, 0, -1);
        CheckFiniteValue("prepend finite 1", result, 1, 0);
        CheckFiniteValue("prepend finite 6", result, 6, 5);

        CheckTransfiniteValue("prepend omega + 3", result, TransfiniteIndex::AfterInfinity(3), 3);
        CheckTransfiniteValue("prepend 2 omega + 3", result, TransfiniteIndex(2, 3), 3);

        CHECK_THROWS("prepend has no 3 omega", result->Get(TransfiniteIndex(3, 0)));

        delete result;
        delete sequence;
    }

    // 3. InsertItemGenerator over 3 infinities
    {
        LazySequence<int>* sequence = CreateThreeNaturalsConcat();
        LazySequence<int>* result = sequence->InsertAt(777, 2);

        CheckFiniteValue("insert item before", result, 1, 1);
        CheckFiniteValue("insert item value", result, 2, 777);
        CheckFiniteValue("insert item shifted", result, 3, 2);
        CheckFiniteValue("insert item later", result, 7, 6);

        CheckTransfiniteValue("insert item omega + 3", result, TransfiniteIndex::AfterInfinity(3), 3);
        CheckTransfiniteValue("insert item 2 omega + 3", result, TransfiniteIndex(2, 3), 3);

        CHECK_THROWS("insert item has no 3 omega", result->Get(TransfiniteIndex(3, 0)));

        delete result;
        delete sequence;
    }

    // 4. AppendGenerator after 3 infinities
    {
        LazySequence<int>* sequence = CreateThreeNaturalsConcat();
        LazySequence<int>* result = sequence->Append(999);

        CheckFiniteValue("append ordinary 5", result, 5, 5);

        CheckTransfiniteValue("append omega + 3", result, TransfiniteIndex::AfterInfinity(3), 3);
        CheckTransfiniteValue("append 2 omega + 3", result, TransfiniteIndex(2, 3), 3);

        CheckTransfiniteValue("append value at 3 omega + 0", result, TransfiniteIndex(3, 0), 999);

        CHECK_THROWS("append after value throws", result->Get(TransfiniteIndex(3, 1)));

        delete result;
        delete sequence;
    }

    // 5. ConcatGenerator: 3 infinities + 1 infinity = 4 infinities
    {
        LazySequence<int>* left = CreateThreeNaturalsConcat();
        LazySequence<int>* right = CreateNaturals();

        LazySequence<int>* result = left->Concat(*right);

        CheckFiniteValue("concat ordinary 4", result, 4, 4);

        CheckTransfiniteValue("concat omega + 4", result, TransfiniteIndex::AfterInfinity(4), 4);
        CheckTransfiniteValue("concat 2 omega + 4", result, TransfiniteIndex(2, 4), 4);
        CheckTransfiniteValue("concat 3 omega + 4", result, TransfiniteIndex(3, 4), 4);

        CHECK_THROWS("concat has no 4 omega", result->Get(TransfiniteIndex(4, 0)));

        delete result;
        delete right;
        delete left;
    }

    // 6. InsertSequenceGenerator: insert 3 infinities into finite sequence
    {
        int data[] = { 10, 20, 30 };
        LazySequence<int> finite(data, 3);

        LazySequence<int>* inserted = CreateThreeNaturalsConcat();
        LazySequence<int>* result = finite.InsertSequenceAt(*inserted, 1);

        CheckFiniteValue("insert sequence finite prefix", result, 0, 10);
        CheckFiniteValue("insert sequence first infinite block 0", result, 1, 0);
        CheckFiniteValue("insert sequence first infinite block 5", result, 6, 5);

        CheckTransfiniteValue("insert sequence omega + 3", result, TransfiniteIndex::AfterInfinity(3), 3);
        CheckTransfiniteValue("insert sequence 2 omega + 3", result, TransfiniteIndex(2, 3), 3);

        CheckTransfiniteValue("insert sequence finite tail 3 omega + 0", result, TransfiniteIndex(3, 0), 20);
        CheckTransfiniteValue("insert sequence finite tail 3 omega + 1", result, TransfiniteIndex(3, 1), 30);

        CHECK_THROWS("insert sequence after finite tail throws", result->Get(TransfiniteIndex(3, 2)));

        delete result;
        delete inserted;
    }

    // 7. InsertSequenceGenerator: insert finite sequence into 3 infinities
    {
        int inserted_data[] = { 100, 200 };
        LazySequence<int> inserted(inserted_data, 2);

        LazySequence<int>* source = CreateThreeNaturalsConcat();
        LazySequence<int>* result = source->InsertSequenceAt(inserted, 2);

        CheckFiniteValue("insert finite into infinite before", result, 1, 1);
        CheckFiniteValue("insert finite into infinite first inserted", result, 2, 100);
        CheckFiniteValue("insert finite into infinite second inserted", result, 3, 200);
        CheckFiniteValue("insert finite into infinite shifted", result, 4, 2);

        CheckTransfiniteValue("insert finite into infinite omega + 3", result, TransfiniteIndex::AfterInfinity(3), 3);
        CheckTransfiniteValue("insert finite into infinite 2 omega + 3", result, TransfiniteIndex(2, 3), 3);

        CHECK_THROWS("insert finite into infinite has no 3 omega", result->Get(TransfiniteIndex(3, 0)));

        delete result;
        delete source;
    }
}


void test_CopyAssignmentWithMultipleInfinities() {
    SUITE("Copy and assignment with multiple infinities");

    LazySequence<int>* source = CreateThreeNaturalsConcat();

    LazySequence<int> copied(*source);
    CHECK("copy ordinary", copied.Get(8) == 8);
    CHECK("copy omega + 4", copied.Get(TransfiniteIndex::AfterInfinity(4)) == 4);
    CHECK("copy 2 omega + 4", copied.Get(TransfiniteIndex(2, 4)) == 4);
    CHECK_THROWS("copy has no 3 omega", copied.Get(TransfiniteIndex(3, 0)));

    LazySequence<int> assigned;
    assigned = *source;
    CHECK("assignment ordinary", assigned.Get(8) == 8);
    CHECK("assignment omega + 4", assigned.Get(TransfiniteIndex::AfterInfinity(4)) == 4);
    CHECK("assignment 2 omega + 4", assigned.Get(TransfiniteIndex(2, 4)) == 4);
    CHECK_THROWS("assignment has no 3 omega", assigned.Get(TransfiniteIndex(3, 0)));

    LazySequence<int>* appended = source->Append(999);
    LazySequence<int> copied_appended(*appended);
    CHECK("copy appended 3 omega + 0", copied_appended.Get(TransfiniteIndex(3, 0)) == 999);
    CHECK_THROWS("copy appended 3 omega + 1 throws", copied_appended.Get(TransfiniteIndex(3, 1)));

    delete appended;
    delete source;
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
    test_CopyAssignmentWithMultipleInfinities();

    test_SequenceReadOnlyStream();
    test_LazyReadOnlyStream();
    test_SequenceWriteOnlyStream();
    test_FileLineReadOnlyStream();
    test_FileLineWriteOnlyStream();

    test_EventParser();
    test_EventReadOnlyStream();
    test_EventMapReadOnlyStream();
    test_OnlineEventStatistics();

    test_ProtocolStatisticsTaskWithStringStream();
    test_ProtocolStatisticsTaskWithEventStream();
    test_ProtocolStatisticsTaskWithFile();

    test_EventBatchProcessingTask();
    test_LiveFileWriter();
    test_LiveFileBatchConsumer();
    test_ThreeConcat();

    test_AllGeneratorsWithMultipleInfinities();

    test_ThreeConcat1();

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
