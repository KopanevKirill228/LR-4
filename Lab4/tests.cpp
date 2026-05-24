#include <iostream>
#include <stdexcept>

#include "lib/ArraySequence.h"

#include "lazy/Cardinal.h"
#include "lazy/RuleGenerator.h"
#include "lazy/SequenceGenerator.h"
#include "lazy/LazySequence.h"
#include "tasks/Event.h"
#include "tasks/OnlineEventStatistics.h"


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


static bool near(double first, double second) {
    double diff = first - second;

    if (diff < 0) {
        diff = -diff;
    }

    return diff < 0.000001;
}


#define CHECK_NEAR(desc, first, second) \
    do { \
        if (near((first), (second))) { ok(desc); } \
        else { fail(desc, __FILE__, __LINE__, #first " near " #second); } \
    } while (0)


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


void test_ConcatCardinality() {
    SUITE("Concat cardinality");

    {
        int left_data[] = { 10, 20 };
        int right_data[] = { 30, 40, 50 };

        LazySequence<int> left(left_data, 2);
        LazySequence<int> right(right_data, 3);

        Sequence<int>* result = left.Concat(right);
        LazySequence<int>* lazy_result = dynamic_cast<LazySequence<int>*>(result);

        CHECK("finite + finite result is lazy", lazy_result != nullptr);
        CHECK("finite + finite cardinality finite", lazy_result->GetCardinality().IsFinite());
        CHECK("finite + finite cardinality value", lazy_result->GetCardinality().GetFiniteValue() == 5);
        CHECK("finite + finite length", lazy_result->GetLength() == 5);

        CHECK("finite + finite value 0", lazy_result->Get(0) == 10);
        CHECK("finite + finite value 1", lazy_result->Get(1) == 20);
        CHECK("finite + finite value 2", lazy_result->Get(2) == 30);
        CHECK("finite + finite value 4", lazy_result->Get(4) == 50);

        delete result;
    }

    {
        int left_data[] = { 100, 200 };
        LazySequence<int> left(left_data, 2);

        LazySequence<int>* right = CreateNaturals();

        Sequence<int>* result = left.Concat(*right);
        LazySequence<int>* lazy_result = dynamic_cast<LazySequence<int>*>(result);

        CHECK("finite + infinite result is lazy", lazy_result != nullptr);
        CHECK("finite + infinite cardinality infinite", lazy_result->GetCardinality().IsInfinite());
        CHECK_THROWS("finite + infinite GetLength throws", lazy_result->GetLength());

        CHECK("finite + infinite left 0", lazy_result->Get(0) == 100);
        CHECK("finite + infinite left 1", lazy_result->Get(1) == 200);
        CHECK("finite + infinite right 0", lazy_result->Get(2) == 0);
        CHECK("finite + infinite right 1", lazy_result->Get(3) == 1);
        CHECK("finite + infinite right 5", lazy_result->Get(7) == 5);

        delete result;
        delete right;
    }

    {
        LazySequence<int>* left = CreateNaturals();

        int right_data[] = { 100, 200 };
        LazySequence<int> right(right_data, 2);

        Sequence<int>* result = left->Concat(right);
        LazySequence<int>* lazy_result = dynamic_cast<LazySequence<int>*>(result);

        CHECK("infinite + finite result is lazy", lazy_result != nullptr);
        CHECK("infinite + finite cardinality infinite", lazy_result->GetCardinality().IsInfinite());
        CHECK_THROWS("infinite + finite GetLength throws", lazy_result->GetLength());

        CHECK("infinite + finite finite index 0 from left", lazy_result->Get(0) == 0);
        CHECK("infinite + finite finite index 1 from left", lazy_result->Get(1) == 1);
        CHECK("infinite + finite finite index 5 from left", lazy_result->Get(5) == 5);

        delete result;
        delete left;
    }

    {
        LazySequence<int>* left = CreateNaturals();
        LazySequence<int>* right = CreateFibonacci();

        Sequence<int>* result = left->Concat(*right);
        LazySequence<int>* lazy_result = dynamic_cast<LazySequence<int>*>(result);

        CHECK("infinite + infinite result is lazy", lazy_result != nullptr);
        CHECK("infinite + infinite cardinality infinite", lazy_result->GetCardinality().IsInfinite());
        CHECK("infinite + infinite finite index 0 from left", lazy_result->Get(0) == 0);
        CHECK("infinite + infinite finite index 10 from left", lazy_result->Get(10) == 10);

        delete result;
        delete right;
        delete left;
    }
}


void test_InsertSequenceFinite() {
    SUITE("InsertSequence finite inserted");

    int source_data[] = { 10, 20, 30, 40 };
    int inserted_data[] = { 111, 222 };

    LazySequence<int> source(source_data, 4);
    LazySequence<int> inserted(inserted_data, 2);

    LazySequence<int>* result = source.InsertSequenceAt(inserted, 2);

    CHECK("finite insert finite cardinality finite", result->GetCardinality().IsFinite());
    CHECK("finite insert finite cardinality value", result->GetCardinality().GetFiniteValue() == 6);
    CHECK("finite insert finite length", result->GetLength() == 6);

    CHECK("finite insert finite value 0", result->Get(0) == 10);
    CHECK("finite insert finite value 1", result->Get(1) == 20);
    CHECK("finite insert finite inserted 0", result->Get(2) == 111);
    CHECK("finite insert finite inserted 1", result->Get(3) == 222);
    CHECK("finite insert finite shifted 0", result->Get(4) == 30);
    CHECK("finite insert finite shifted 1", result->Get(5) == 40);

    delete result;
}


void test_InsertSequenceInfiniteSource() {
    SUITE("InsertSequence into infinite source");

    LazySequence<int>* source = CreateNaturals();

    int inserted_data[] = { 100, 200, 300 };
    LazySequence<int> inserted(inserted_data, 3);

    LazySequence<int>* result = source->InsertSequenceAt(inserted, 2);

    CHECK("infinite source insert finite cardinality infinite", result->GetCardinality().IsInfinite());
    CHECK_THROWS("infinite source insert finite GetLength throws", result->GetLength());

    CHECK("infinite source insert finite value 0", result->Get(0) == 0);
    CHECK("infinite source insert finite value 1", result->Get(1) == 1);
    CHECK("infinite source insert finite inserted 0", result->Get(2) == 100);
    CHECK("infinite source insert finite inserted 1", result->Get(3) == 200);
    CHECK("infinite source insert finite inserted 2", result->Get(4) == 300);
    CHECK("infinite source insert finite shifted 0", result->Get(5) == 2);
    CHECK("infinite source insert finite shifted 1", result->Get(6) == 3);

    delete result;
    delete source;
}


void test_InsertSequenceInfiniteInserted() {
    SUITE("InsertSequence infinite inserted");

    int source_data[] = { 10, 20, 30 };
    LazySequence<int> source(source_data, 3);

    LazySequence<int>* inserted = CreateNaturals();

    LazySequence<int>* result = source.InsertSequenceAt(*inserted, 1);

    CHECK("finite source insert infinite cardinality infinite", result->GetCardinality().IsInfinite());
    CHECK_THROWS("finite source insert infinite GetLength throws", result->GetLength());

    CHECK("finite source insert infinite left prefix", result->Get(0) == 10);
    CHECK("finite source insert infinite inserted 0", result->Get(1) == 0);
    CHECK("finite source insert infinite inserted 1", result->Get(2) == 1);
    CHECK("finite source insert infinite inserted 2", result->Get(3) == 2);
    CHECK("finite source insert infinite inserted 10", result->Get(11) == 10);

    delete result;
    delete inserted;
}


void test_InsertItemAsSpecialCase() {
    SUITE("InsertAt item as special case");

    int source_data[] = { 10, 20, 30 };
    LazySequence<int> source(source_data, 3);

    Sequence<int>* result = source.InsertAt(777, 1);
    LazySequence<int>* lazy_result = dynamic_cast<LazySequence<int>*>(result);

    CHECK("item insert result is lazy", lazy_result != nullptr);
    CHECK("item insert cardinality finite", lazy_result->GetCardinality().IsFinite());
    CHECK("item insert cardinality value", lazy_result->GetCardinality().GetFiniteValue() == 4);

    CHECK("item insert value 0", lazy_result->Get(0) == 10);
    CHECK("item insert item", lazy_result->Get(1) == 777);
    CHECK("item insert shifted 0", lazy_result->Get(2) == 20);
    CHECK("item insert shifted 1", lazy_result->Get(3) == 30);

    delete result;
}


void test_OnlineEventStatisticsMedianVariance() {
    SUITE("OnlineEventStatistics median and variance");

    OnlineEventStatistics<double> stats;

    CHECK_THROWS("median without measurements throws", stats.GetMedianMeasure());
    CHECK_THROWS("variance without measurements throws", stats.GetVarianceMeasure());

    stats.AddEvent(Event<double>(EventType::Measure, 10.0, ""));

    CHECK("one value median", stats.GetMedianMeasure() == 10.0);
    CHECK("one value variance", stats.GetVarianceMeasure() == 0.0);

    stats.AddEvent(Event<double>(EventType::Measure, 20.0, ""));

    CHECK("two values median", stats.GetMedianMeasure() == 15.0);
    CHECK("two values variance", stats.GetVarianceMeasure() == 25.0);

    stats.AddEvent(Event<double>(EventType::Measure, 30.0, ""));

    CHECK("three values median", stats.GetMedianMeasure() == 20.0);
    CHECK("three values average", stats.GetAverageMeasure() == 20.0);
    CHECK_NEAR("three values variance", stats.GetVarianceMeasure(), 200.0 / 3.0);

    stats.AddEvent(Event<double>(EventType::Measure, 40.0, ""));

    CHECK("four values median", stats.GetMedianMeasure() == 25.0);
    CHECK("four values average", stats.GetAverageMeasure() == 25.0);
    CHECK("four values variance", stats.GetVarianceMeasure() == 125.0);

    stats.Clear();

    CHECK("clear has no measurements", !stats.HasMeasurements());
    CHECK_THROWS("median after clear throws", stats.GetMedianMeasure());
    CHECK_THROWS("variance after clear throws", stats.GetVarianceMeasure());
}



void test_GetAfterInfinite() {
    SUITE("LazySequence GetAfterInfinite");

    {
        LazySequence<int>* left = CreateNaturals();

        int right_data[] = { 100, 200 };
        LazySequence<int> right(right_data, 2);

        Sequence<int>* result = left->Concat(right);
        LazySequence<int>* lazy_result = dynamic_cast<LazySequence<int>*>(result);

        CHECK("concat infinite finite ordinary index", lazy_result->Get(5) == 5);
        CHECK("concat infinite finite after infinity 0", lazy_result->GetAfterInfinite(0) == 100);
        CHECK("concat infinite finite after infinity 1", lazy_result->GetAfterInfinite(1) == 200);
        CHECK_THROWS("concat infinite finite after infinity out of range", lazy_result->GetAfterInfinite(2));

        delete result;
        delete left;
    }

    {
        int source_data[] = { 10, 20, 30 };
        LazySequence<int> source(source_data, 3);

        LazySequence<int>* inserted = CreateNaturals();

        LazySequence<int>* result = source.InsertSequenceAt(*inserted, 1);

        CHECK("insert infinite ordinary prefix", result->Get(0) == 10);
        CHECK("insert infinite ordinary inserted 0", result->Get(1) == 0);
        CHECK("insert infinite ordinary inserted 3", result->Get(4) == 3);

        CHECK("insert infinite after infinity 0", result->GetAfterInfinite(0) == 20);
        CHECK("insert infinite after infinity 1", result->GetAfterInfinite(1) == 30);
        CHECK_THROWS("insert infinite after infinity out of range", result->GetAfterInfinite(2));

        delete result;
        delete inserted;
    }

    {
        LazySequence<int>* source = CreateNaturals();

        Sequence<int>* result = source->Append(999);
        LazySequence<int>* lazy_result = dynamic_cast<LazySequence<int>*>(result);

        CHECK("append to infinite ordinary index", lazy_result->Get(10) == 10);
        CHECK("append to infinite after infinity", lazy_result->GetAfterInfinite(0) == 999);

        delete result;
        delete source;
    }
}

void run_all_tests() {
    test_ConcatCardinality();

    test_InsertSequenceFinite();
    test_InsertSequenceInfiniteSource();
    test_InsertSequenceInfiniteInserted();
    test_InsertItemAsSpecialCase();


    test_OnlineEventStatisticsMedianVariance();

    test_GetAfterInfinite();


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