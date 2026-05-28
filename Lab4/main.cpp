
#include <iostream>
#include <string>
#include <limits>
#include <fstream>
#include <stdexcept>

#ifdef _WIN32
#include <windows.h>
#endif

#include "lib/ArraySequence.h"

#include "lazy/Cardinal.h"
#include "lazy/CardinalIO.h"
#include "lazy/TransfiniteIndex.h"
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

static void setupConsole() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

static void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

static bool readInt(int& val) {
    if (!(std::cin >> val)) {
        clearInput();
        return false;
    }

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return true;
}

static bool readIntWithPrompt(int& val, const std::string& prompt) {
    std::cout << prompt;
    return readInt(val);
}

static bool readIntInRange(int& val, const std::string& prompt, int lo, int hi) {
    while (true) {
        std::cout << prompt;

        if (!readInt(val)) {
            std::cout << "  [ERR] Not a number. Try again.\n";
            continue;
        }

        if (val == 0) {
            return false;
        }

        if (val < lo || val > hi) {
            std::cout << "  [ERR] Enter a number from " << lo
                      << " to " << hi << " (0 = back).\n";
            continue;
        }

        return true;
    }
}

static std::string readLineWithPrompt(const std::string& prompt) {
    std::string line;
    std::cout << prompt;
    std::getline(std::cin, line);
    return line;
}

static void printSeparator() {
    std::cout << "------------------------------------------------------------\n";
}

static void printTitle(const std::string& title) {
    printSeparator();
    std::cout << "  " << title << "\n";
    printSeparator();
}

static void printError(const std::string& msg) {
    std::cout << "  [ERR] " << msg << "\n";
}

static void pauseConsole() {
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
}

static bool handleInputError() {
    printError("Not a number");
    return false;
}

static LazySequence<int>* createNaturals() {
    int init_data[] = { 0 };
    MutableArraySequence<int> init(init_data, 1);

    return new LazySequence<int>(
        [](const Sequence<int>& source) {
            return source.GetLength();
        },
        init
    );
}

static LazySequence<int>* createFibonacci() {
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

static LazySequence<int>* readFiniteLazySequence(const std::string& name) {
    int count;

    if (!readIntWithPrompt(count, "  " + name + " count: ") || count < 0) {
        printError("Count must be non-negative");
        return nullptr;
    }

    int* data = nullptr;

    try {
        if (count > 0) {
            data = new int[count];
        }

        for (int i = 0; i < count; ++i) {
            while (!readIntWithPrompt(data[i], "  " + name + "[" + std::to_string(i) + "]: ")) {
                handleInputError();
            }
        }

        LazySequence<int>* result = new LazySequence<int>(data, count);
        delete[] data;
        return result;
    }
    catch (...) {
        delete[] data;
        throw;
    }
}

static LazySequence<int>* chooseLazySequence(const std::string& name) {
    int choice;

    while (true) {
        printSeparator();
        std::cout << "  Choose " << name << "\n";
        std::cout << "  1. Finite sequence from keyboard\n";
        std::cout << "  2. Naturals: 0, 1, 2, ...\n";
        std::cout << "  3. Fibonacci: 1, 1, 2, 3, ...\n";
        std::cout << "  0. Back\n";

        if (!readIntInRange(choice, "Choice: ", 1, 3)) {
            return nullptr;
        }

        try {
            if (choice == 1) {
                return readFiniteLazySequence(name);
            }

            if (choice == 2) {
                return createNaturals();
            }

            if (choice == 3) {
                return createFibonacci();
            }
        }
        catch (const std::exception& e) {
            printError(e.what());
            return nullptr;
        }
    }
}

static void printLazyInfo(const LazySequence<int>& sequence) {
    std::cout << "  Type: LazySequence<int>\n";

    if (sequence.IsInfinite()) {
        std::cout << "  Length: infinite\n";
    }
    else {
        std::cout << "  Length: " << sequence.GetLength() << "\n";
    }

    std::cout << "  Cardinality: " << sequence.GetCardinality() << "\n";
    std::cout << "  Materialized count: " << sequence.GetMaterializedCount() << "\n";
}

static void printLazyPrefix(LazySequence<int>& sequence, int count) {
    if (count < 0) {
        printError("Count must be non-negative");
        return;
    }

    if (!sequence.IsInfinite() && count > sequence.GetLength()) {
        count = sequence.GetLength();
    }

    std::cout << "  [";

    for (int i = 0; i < count; ++i) {
        if (i > 0) {
            std::cout << ", ";
        }

        std::cout << sequence.Get(i);
    }

    std::cout << "]\n";
}

static void printStatistics(const OnlineEventStatistics<double>& stats) {
    std::cout << "  Total events:   " << stats.GetTotalEvents() << "\n";
    std::cout << "  START events:   " << stats.GetStartEvents() << "\n";
    std::cout << "  END events:     " << stats.GetEndEvents() << "\n";
    std::cout << "  MEASURE events: " << stats.GetMeasureEvents() << "\n";
    std::cout << "  ERROR events:   " << stats.GetErrorEvents() << "\n";
    std::cout << "  UNKNOWN events: " << stats.GetUnknownEvents() << "\n";

    if (stats.HasMeasurements()) {
        std::cout << "\n";
        std::cout << "  Measurement statistics:\n";
        std::cout << "  Min:      " << stats.GetMinMeasure() << "\n";
        std::cout << "  Max:      " << stats.GetMaxMeasure() << "\n";
        std::cout << "  Average:  " << stats.GetAverageMeasure() << "\n";
        std::cout << "  Variance: " << stats.GetVarianceMeasure() << "\n";
        std::cout << "  Median:   " << stats.GetMedianMeasure() << "\n";
    }
    else {
        std::cout << "\n  No measurements.\n";
    }
}

static void demoCardinalAndTransfiniteIndex() {
    printTitle("Cardinal, CardinalIO and TransfiniteIndex");

    int finite_length;

    if (!readIntWithPrompt(finite_length, "  Finite cardinal value: ") || finite_length < 0) {
        printError("Value must be non-negative");
        return;
    }

    Cardinal finite = Cardinal::Finite(finite_length);
    Cardinal infinity = Cardinal::Infinity();

    std::cout << "  Cardinal::Finite: " << finite << "\n";
    std::cout << "  Cardinal::Infinity: " << infinity << "\n";

    int usual_index;
    int tail_index;

    if (!readIntWithPrompt(usual_index, "  Usual finite index: ") || usual_index < 0) {
        printError("Index must be non-negative");
        return;
    }

    if (!readIntWithPrompt(tail_index, "  Tail index after omega: ") || tail_index < 0) {
        printError("Index must be non-negative");
        return;
    }

    TransfiniteIndex finite_index = TransfiniteIndex::Finite(usual_index);
    TransfiniteIndex omega_index = TransfiniteIndex::AfterInfinity(tail_index);

    std::cout << "  Finite index: infinity_count = "
              << finite_index.GetInfinityCount()
              << ", finite_part = " << finite_index.GetFiniteIndex() << "\n";

    std::cout << "  Omega index: infinity_count = "
              << omega_index.GetInfinityCount()
              << ", finite_part = " << omega_index.GetFiniteIndex() << "\n";
}

static void demoSequenceGenerator() {
    printTitle("SequenceGenerator");

    LazySequence<int>* sequence = nullptr;

    try {
        sequence = readFiniteLazySequence("source");

        if (sequence == nullptr) {
            return;
        }

        SequenceGenerator<int> generator(*sequence);

        std::cout << "  Reading source through SequenceGenerator:\n";

        while (generator.HasNext()) {
            std::cout << "  position=" << generator.GetPosition()
                      << ", value=" << generator.GetNext() << "\n";
        }

        generator.Reset();

        std::cout << "  After Reset(), position = "
                  << generator.GetPosition() << "\n";

        delete sequence;
    }
    catch (const std::exception& e) {
        delete sequence;
        printError(e.what());
    }
}

static void demoRuleGeneratorThroughLazy() {
    printTitle("RuleGenerator through LazySequence");

    std::cout << "  RuleGenerator is used inside infinite LazySequence.\n";
    std::cout << "  1. Naturals\n";
    std::cout << "  2. Fibonacci\n";

    int choice;
    if (!readIntInRange(choice, "Choice: ", 1, 2)) {
        return;
    }

    int count;

    if (!readIntWithPrompt(count, "  Count to print: ") || count < 0) {
        printError("Count must be non-negative");
        return;
    }

    LazySequence<int>* sequence = nullptr;

    try {
        sequence = choice == 1 ? createNaturals() : createFibonacci();
        printLazyInfo(*sequence);
        printLazyPrefix(*sequence, count);
        delete sequence;
    }
    catch (const std::exception& e) {
        delete sequence;
        printError(e.what());
    }
}

static void demoLazyGetAndSubsequence() {
    printTitle("LazySequence Get, GetSubsequence, Enumerator");

    LazySequence<int>* sequence = nullptr;
    Sequence<int>* subsequence = nullptr;
    IEnumerator<int>* enumerator = nullptr;

    try {
        sequence = chooseLazySequence("source");

        if (sequence == nullptr) {
            return;
        }

        printLazyInfo(*sequence);

        int count;

        if (!readIntWithPrompt(count, "  How many first elements to print: ") || count < 0) {
            printError("Count must be non-negative");
            delete sequence;
            return;
        }

        printLazyPrefix(*sequence, count);

        int index;

        if (readIntWithPrompt(index, "  Get finite index: ")) {
            try {
                std::cout << "  Get(" << index << ") = " << sequence->Get(index) << "\n";
            }
            catch (const std::exception& e) {
                printError(e.what());
            }
        }

        int tail_index;

        if (readIntWithPrompt(tail_index, "  Try Get(omega + k), k = ")) {
            try {
                std::cout << "  Get(omega + " << tail_index << ") = "
                          << sequence->Get(TransfiniteIndex::AfterInfinity(tail_index))
                          << "\n";
            }
            catch (const std::exception& e) {
                printError(e.what());
            }
        }

        if (!sequence->IsInfinite()) {
            int start;
            int end;

            if (readIntWithPrompt(start, "  Subsequence start: ") &&
                readIntWithPrompt(end, "  Subsequence end: ")) {
                try {
                    subsequence = sequence->GetSubsequence(start, end);
                    LazySequence<int>* lazy_sub = dynamic_cast<LazySequence<int>*>(subsequence);

                    if (lazy_sub != nullptr) {
                        std::cout << "  Subsequence:\n";
                        printLazyPrefix(*lazy_sub, lazy_sub->GetLength());
                    }

                    delete subsequence;
                    subsequence = nullptr;
                }
                catch (const std::exception& e) {
                    printError(e.what());
                }
            }
        }

        enumerator = sequence->GetEnumerator();

        std::cout << "  Enumerator output, first " << count << " elements:\n";

        for (int i = 0; i < count; ++i) {
            if (!enumerator->MoveNext()) {
                break;
            }

            std::cout << "  " << enumerator->GetCurrent() << "\n";
        }

        delete enumerator;
        delete sequence;
    }
    catch (const std::exception& e) {
        delete enumerator;
        delete subsequence;
        delete sequence;
        printError(e.what());
    }
}

static void demoAppend() {
    printTitle("AppendGenerator");

    LazySequence<int>* source = nullptr;
    Sequence<int>* result = nullptr;

    try {
        source = chooseLazySequence("source");

        if (source == nullptr) {
            return;
        }

        int item;

        if (!readIntWithPrompt(item, "  Value to append: ")) {
            handleInputError();
            delete source;
            return;
        }

        result = source->Append(item);
        LazySequence<int>* lazy_result = dynamic_cast<LazySequence<int>*>(result);

        if (lazy_result == nullptr) {
            throw std::runtime_error("Append result is not LazySequence");
        }

        printLazyInfo(*lazy_result);

        int count;

        if (readIntWithPrompt(count, "  How many finite elements to print: ") && count >= 0) {
            printLazyPrefix(*lazy_result, count);
        }

        if (lazy_result->IsInfinite()) {
            try {
                std::cout << "  Get(omega + 0) = "
                          << lazy_result->Get(TransfiniteIndex::AfterInfinity(0))
                          << "\n";
            }
            catch (const std::exception& e) {
                printError(e.what());
            }
        }

        delete result;
        delete source;
    }
    catch (const std::exception& e) {
        delete result;
        delete source;
        printError(e.what());
    }
}

static void demoPrepend() {
    printTitle("PrependGenerator");

    LazySequence<int>* source = nullptr;
    Sequence<int>* result = nullptr;

    try {
        source = chooseLazySequence("source");

        if (source == nullptr) {
            return;
        }

        int item;

        if (!readIntWithPrompt(item, "  Value to prepend: ")) {
            handleInputError();
            delete source;
            return;
        }

        result = source->Prepend(item);
        LazySequence<int>* lazy_result = dynamic_cast<LazySequence<int>*>(result);

        if (lazy_result == nullptr) {
            throw std::runtime_error("Prepend result is not LazySequence");
        }

        printLazyInfo(*lazy_result);

        int count;

        if (readIntWithPrompt(count, "  How many elements to print: ") && count >= 0) {
            printLazyPrefix(*lazy_result, count);
        }

        delete result;
        delete source;
    }
    catch (const std::exception& e) {
        delete result;
        delete source;
        printError(e.what());
    }
}

static void demoInsertItem() {
    printTitle("InsertItemGenerator");

    LazySequence<int>* source = nullptr;
    Sequence<int>* result = nullptr;

    try {
        source = chooseLazySequence("source");

        if (source == nullptr) {
            return;
        }

        int item;
        int index;

        if (!readIntWithPrompt(item, "  Value to insert: ")) {
            handleInputError();
            delete source;
            return;
        }

        if (!readIntWithPrompt(index, "  Finite insert index: ")) {
            handleInputError();
            delete source;
            return;
        }

        result = source->InsertAt(item, index);
        LazySequence<int>* lazy_result = dynamic_cast<LazySequence<int>*>(result);

        if (lazy_result == nullptr) {
            throw std::runtime_error("InsertAt result is not LazySequence");
        }

        printLazyInfo(*lazy_result);

        int count;

        if (readIntWithPrompt(count, "  How many elements to print: ") && count >= 0) {
            printLazyPrefix(*lazy_result, count);
        }

        delete result;
        delete source;
    }
    catch (const std::exception& e) {
        delete result;
        delete source;
        printError(e.what());
    }
}

static void demoConcat() {
    printTitle("ConcatGenerator");

    LazySequence<int>* left = nullptr;
    LazySequence<int>* right = nullptr;
    Sequence<int>* result = nullptr;

    try {
        left = chooseLazySequence("left sequence");

        if (left == nullptr) {
            return;
        }

        right = chooseLazySequence("right sequence");

        if (right == nullptr) {
            delete left;
            return;
        }

        result = left->Concat(*right);
        LazySequence<int>* lazy_result = dynamic_cast<LazySequence<int>*>(result);

        if (lazy_result == nullptr) {
            throw std::runtime_error("Concat result is not LazySequence");
        }

        printLazyInfo(*lazy_result);

        int count;

        if (readIntWithPrompt(count, "  How many finite elements to print: ") && count >= 0) {
            printLazyPrefix(*lazy_result, count);
        }

        if (lazy_result->IsInfinite()) {
            int tail;

            if (readIntWithPrompt(tail, "  Try transfinite index omega + k, k = ")) {
                try {
                    std::cout << "  Get(omega + " << tail << ") = "
                              << lazy_result->Get(TransfiniteIndex::AfterInfinity(tail))
                              << "\n";
                }
                catch (const std::exception& e) {
                    printError(e.what());
                }
            }
        }

        delete result;
        delete right;
        delete left;
    }
    catch (const std::exception& e) {
        delete result;
        delete right;
        delete left;
        printError(e.what());
    }
}

static void demoInsertSequence() {
    printTitle("InsertSequenceGenerator");

    LazySequence<int>* source = nullptr;
    LazySequence<int>* inserted = nullptr;
    LazySequence<int>* result = nullptr;

    try {
        source = chooseLazySequence("source sequence");

        if (source == nullptr) {
            return;
        }

        inserted = chooseLazySequence("inserted sequence");

        if (inserted == nullptr) {
            delete source;
            return;
        }

        std::cout << "  Insert index type:\n";
        std::cout << "  1. Finite index\n";
        std::cout << "  2. Transfinite index omega + k\n";

        int index_type;

        if (!readIntInRange(index_type, "Choice: ", 1, 2)) {
            delete inserted;
            delete source;
            return;
        }

        if (index_type == 1) {
            int index;

            if (!readIntWithPrompt(index, "  Finite index: ")) {
                handleInputError();
                delete inserted;
                delete source;
                return;
            }

            result = source->InsertSequenceAt(*inserted, TransfiniteIndex::Finite(index));
        }
        else {
            int tail_index;

            if (!readIntWithPrompt(tail_index, "  Tail index k for omega + k: ")) {
                handleInputError();
                delete inserted;
                delete source;
                return;
            }

            result = source->InsertSequenceAt(*inserted, TransfiniteIndex::AfterInfinity(tail_index));
        }

        printLazyInfo(*result);

        int count;

        if (readIntWithPrompt(count, "  How many finite elements to print: ") && count >= 0) {
            printLazyPrefix(*result, count);
        }

        if (result->IsInfinite()) {
            int tail;

            if (readIntWithPrompt(tail, "  Try transfinite index omega + k, k = ")) {
                try {
                    std::cout << "  Get(omega + " << tail << ") = "
                              << result->Get(TransfiniteIndex::AfterInfinity(tail))
                              << "\n";
                }
                catch (const std::exception& e) {
                    printError(e.what());
                }
            }
        }

        delete result;
        delete inserted;
        delete source;
    }
    catch (const std::exception& e) {
        delete result;
        delete inserted;
        delete source;
        printError(e.what());
    }
}

static void demoAppendDoesNotChangeFibonacci() {
    printTitle("Append does not change Fibonacci rule");

    LazySequence<int>* fib = nullptr;
    Sequence<int>* result = nullptr;

    try {
        fib = createFibonacci();

        int item;

        if (!readIntWithPrompt(item, "  Value to append to Fibonacci: ")) {
            handleInputError();
            delete fib;
            return;
        }

        result = fib->Append(item);
        LazySequence<int>* lazy_result = dynamic_cast<LazySequence<int>*>(result);

        if (lazy_result == nullptr) {
            throw std::runtime_error("Append result is not LazySequence");
        }

        std::cout << "  Finite part after append:\n";
        printLazyPrefix(*lazy_result, 8);

        std::cout << "  Append creates an operation generator.\n";
        std::cout << "  It does not write item into Fibonacci RuleGenerator history.\n";
        std::cout << "  Therefore after 1, 1, 2, 3, 5 ordinary next value is 8.\n";
        std::cout << "  Appended item is located at omega + 0.\n";

        std::cout << "  Get(omega + 0) = "
                  << lazy_result->Get(TransfiniteIndex::AfterInfinity(0))
                  << "\n";

        delete result;
        delete fib;
    }
    catch (const std::exception& e) {
        delete result;
        delete fib;
        printError(e.what());
    }
}

static void lazyMenu() {
    int choice;

    while (true) {
        printTitle("Lazy sequences and generators");
        std::cout << "  1. Cardinal / CardinalIO / TransfiniteIndex\n";
        std::cout << "  2. SequenceGenerator directly\n";
        std::cout << "  3. RuleGenerator through infinite LazySequence\n";
        std::cout << "  4. LazySequence Get / Subsequence / Enumerator\n";
        std::cout << "  5. AppendGenerator\n";
        std::cout << "  6. PrependGenerator\n";
        std::cout << "  7. InsertItemGenerator\n";
        std::cout << "  8. ConcatGenerator\n";
        std::cout << "  9. InsertSequenceGenerator\n";
        std::cout << " 10. Fibonacci append rule check\n";
        std::cout << "  0. Back\n";

        if (!readIntInRange(choice, "Choice: ", 1, 10)) {
            return;
        }

        switch (choice) {
        case 1: demoCardinalAndTransfiniteIndex(); break;
        case 2: demoSequenceGenerator(); break;
        case 3: demoRuleGeneratorThroughLazy(); break;
        case 4: demoLazyGetAndSubsequence(); break;
        case 5: demoAppend(); break;
        case 6: demoPrepend(); break;
        case 7: demoInsertItem(); break;
        case 8: demoConcat(); break;
        case 9: demoInsertSequence(); break;
        case 10: demoAppendDoesNotChangeFibonacci(); break;
        }

        pauseConsole();
    }
}

static void demoSequenceReadOnlyStream() {
    printTitle("SequenceReadOnlyStream");

    LazySequence<int>* sequence = nullptr;

    try {
        sequence = readFiniteLazySequence("stream source");

        if (sequence == nullptr) {
            return;
        }

        SequenceReadOnlyStream<int> stream(sequence);
        Stream<int>* base = &stream;
        ReadOnlyStream<int>* reader = &stream;

        base->Open();

        std::cout << "  IsCanSeek: " << (reader->IsCanSeek() ? "true" : "false") << "\n";
        std::cout << "  IsCanGoBack: " << (reader->IsCanGoBack() ? "true" : "false") << "\n";

        while (!reader->IsEndOfStream()) {
            std::cout << "  position=" << base->GetPosition()
                      << ", value=" << reader->Read() << "\n";
        }

        if (reader->IsCanSeek()) {
            int index;

            if (readIntWithPrompt(index, "  Seek index: ")) {
                try {
                    reader->Seek(index);

                    if (!reader->IsEndOfStream()) {
                        std::cout << "  After seek value: " << reader->Read() << "\n";
                    }
                }
                catch (const std::exception& e) {
                    printError(e.what());
                }
            }
        }

        base->Close();
        delete sequence;
    }
    catch (const std::exception& e) {
        delete sequence;
        printError(e.what());
    }
}

static void demoLazyReadOnlyStream() {
    printTitle("LazyReadOnlyStream");

    LazySequence<int>* sequence = nullptr;

    try {
        sequence = chooseLazySequence("lazy stream source");

        if (sequence == nullptr) {
            return;
        }

        LazyReadOnlyStream<int> stream(sequence);
        Stream<int>* base = &stream;
        ReadOnlyStream<int>* reader = &stream;

        int count;

        if (!readIntWithPrompt(count, "  How many values to read: ") || count < 0) {
            printError("Count must be non-negative");
            delete sequence;
            return;
        }

        base->Open();

        for (int i = 0; i < count; ++i) {
            if (reader->IsEndOfStream()) {
                break;
            }

            std::cout << "  position=" << base->GetPosition()
                      << ", value=" << reader->Read() << "\n";
        }

        base->Close();
        delete sequence;
    }
    catch (const std::exception& e) {
        delete sequence;
        printError(e.what());
    }
}

static void demoSequenceWriteOnlyStream() {
    printTitle("SequenceWriteOnlyStream");

    Sequence<int>* sequence = nullptr;

    try {
        sequence = new MutableArraySequence<int>();

        SequenceWriteOnlyStream<int> stream(sequence);

        Stream<int>* base = &stream;
        WriteOnlyStream<int>* writer = &stream;

        base->Open();

        int count;

        if (!readIntWithPrompt(count, "  How many numbers to write: ") || count < 0) {
            printError("Count must be non-negative");
            base->Close();
            delete sequence;
            return;
        }

        for (int i = 0; i < count; ++i) {
            int value;

            while (!readIntWithPrompt(value, "  value[" + std::to_string(i) + "]: ")) {
                handleInputError();
            }

            writer->Write(value);
        }

        std::cout << "  Written count: " << base->GetPosition() << "\n";
        std::cout << "  Sequence content: ";

        for (int i = 0; i < sequence->GetLength(); ++i) {
            if (i > 0) {
                std::cout << ", ";
            }

            std::cout << sequence->Get(i);
        }

        std::cout << "\n";

        base->Close();

        delete sequence;
    }
    catch (const std::exception& e) {
        delete sequence;
        printError(e.what());
    }
}

static void demoFileLineReadOnlyStream() {
    printTitle("FileLineReadOnlyStream");

    std::string filename = readLineWithPrompt("  File path: ");

    try {
        FileLineReadOnlyStream stream(filename);

        Stream<std::string>* base = &stream;
        ReadOnlyStream<std::string>* reader = &stream;

        base->Open();

        std::cout << "  IsCanSeek: " << (reader->IsCanSeek() ? "true" : "false") << "\n";
        std::cout << "  IsCanGoBack: " << (reader->IsCanGoBack() ? "true" : "false") << "\n";

        while (!reader->IsEndOfStream()) {
            std::cout << "  position=" << base->GetPosition()
                      << ", line=\"" << reader->Read() << "\"\n";
        }

        base->Close();
    }
    catch (const std::exception& e) {
        printError(e.what());
    }
}

static void demoFileLineWriteOnlyStream() {
    printTitle("FileLineWriteOnlyStream");

    std::string filename = readLineWithPrompt("  Output file path: ");

    try {
        FileLineWriteOnlyStream stream(filename);

        Stream<std::string>* base = &stream;
        WriteOnlyStream<std::string>* writer = &stream;

        base->Open();

        int count;

        if (!readIntWithPrompt(count, "  How many lines to write: ") || count < 0) {
            printError("Count must be non-negative");
            base->Close();
            return;
        }

        for (int i = 0; i < count; ++i) {
            std::string line = readLineWithPrompt("  line[" + std::to_string(i) + "]: ");
            writer->Write(line);
        }

        std::cout << "  Written lines: " << base->GetPosition() << "\n";
        base->Close();
        std::cout << "  File written.\n";
    }
    catch (const std::exception& e) {
        printError(e.what());
    }
}

static void streamsMenu() {
    int choice;

    while (true) {
        printTitle("Streams");
        std::cout << "  1. SequenceReadOnlyStream\n";
        std::cout << "  2. LazyReadOnlyStream\n";
        std::cout << "  3. SequenceWriteOnlyStream\n";
        std::cout << "  4. FileLineReadOnlyStream\n";
        std::cout << "  5. FileLineWriteOnlyStream\n";
        std::cout << "  0. Back\n";

        if (!readIntInRange(choice, "Choice: ", 1, 5)) {
            return;
        }

        switch (choice) {
        case 1: demoSequenceReadOnlyStream(); break;
        case 2: demoLazyReadOnlyStream(); break;
        case 3: demoSequenceWriteOnlyStream(); break;
        case 4: demoFileLineReadOnlyStream(); break;
        case 5: demoFileLineWriteOnlyStream(); break;
        }

        pauseConsole();
    }
}

static void printEvent(const Event<double>& event) {
    if (event.type == EventType::Start) {
        std::cout << "  Type: START\n";
    }
    else if (event.type == EventType::End) {
        std::cout << "  Type: END\n";
    }
    else if (event.type == EventType::Measure) {
        std::cout << "  Type: MEASURE\n";
        std::cout << "  Value: " << event.value << "\n";
    }
    else if (event.type == EventType::Error) {
        std::cout << "  Type: ERROR\n";
        std::cout << "  Message: " << event.message << "\n";
    }
    else {
        std::cout << "  Type: UNKNOWN\n";
        std::cout << "  Message: " << event.message << "\n";
    }
}

static void demoEventParser() {
    printTitle("EventParser");

    std::cout << "  Examples:\n";
    std::cout << "  START\n";
    std::cout << "  END\n";
    std::cout << "  MEASURE 12.5\n";
    std::cout << "  ERROR sensor disconnected\n";

    std::string line = readLineWithPrompt("  Enter event line: ");

    try {
        Event<double> event = EventParser<double>::ParseLine(line);
        printEvent(event);
    }
    catch (const std::exception& e) {
        printError(e.what());
    }
}

static void demoEventReadOnlyStream() {
    printTitle("EventReadOnlyStream");

    int count;

    if (!readIntWithPrompt(count, "  Number of event lines: ") || count < 0) {
        printError("Count must be non-negative");
        return;
    }

    std::string* data = nullptr;
    MutableArraySequence<std::string>* lines = nullptr;

    try {
        if (count > 0) {
            data = new std::string[count];
        }

        for (int i = 0; i < count; ++i) {
            data[i] = readLineWithPrompt("  line[" + std::to_string(i) + "]: ");
        }

        lines = new MutableArraySequence<std::string>(data, count);
        SequenceReadOnlyStream<std::string> line_stream(lines);
        EventReadOnlyStream<double> event_stream(&line_stream);

        event_stream.Open();

        while (!event_stream.IsEndOfStream()) {
            Event<double> event = event_stream.Read();
            std::cout << "  position=" << event_stream.GetPosition() << "\n";
            printEvent(event);
        }

        event_stream.Close();

        delete lines;
        delete[] data;
    }
    catch (const std::exception& e) {
        delete lines;
        delete[] data;
        printError(e.what());
    }
}

static void demoManualOnlineStatistics() {
    printTitle("OnlineEventStatistics");

    std::cout << "  Enter event lines. Type STOP to finish.\n";
    OnlineEventStatistics<double> stats;

    while (true) {
        std::string line = readLineWithPrompt("  > ");

        if (line == "STOP") {
            break;
        }

        Event<double> event = EventParser<double>::ParseLine(line);
        stats.AddEvent(event);
    }

    printStatistics(stats);
}

static void demoProtocolStatisticsStringStream() {
    printTitle("ProtocolStatisticsTask with string stream");

    int count;

    if (!readIntWithPrompt(count, "  Number of protocol lines: ") || count < 0) {
        printError("Count must be non-negative");
        return;
    }

    std::string* data = nullptr;
    MutableArraySequence<std::string>* lines = nullptr;

    try {
        if (count > 0) {
            data = new std::string[count];
        }

        for (int i = 0; i < count; ++i) {
            data[i] = readLineWithPrompt("  line[" + std::to_string(i) + "]: ");
        }

        lines = new MutableArraySequence<std::string>(data, count);
        SequenceReadOnlyStream<std::string> stream(lines);

        OnlineEventStatistics<double> stats = ProtocolStatisticsTask<double>::Process(stream);
        printStatistics(stats);

        delete lines;
        delete[] data;
    }
    catch (const std::exception& e) {
        delete lines;
        delete[] data;
        printError(e.what());
    }
}

static void demoProtocolStatisticsFile() {
    printTitle("ProtocolStatisticsTask with file");

    std::string filename = readLineWithPrompt("  File path: ");

    try {
        FileLineReadOnlyStream stream(filename);
        OnlineEventStatistics<double> stats = ProtocolStatisticsTask<double>::Process(stream);
        printStatistics(stats);
    }
    catch (const std::exception& e) {
        printError(e.what());
    }
}

static void demoCreateProtocolFile() {
    printTitle("Create protocol file");

    std::string filename = readLineWithPrompt("  Output file path: ");
    int count;

    if (!readIntWithPrompt(count, "  Number of lines: ") || count < 0) {
        printError("Count must be non-negative");
        return;
    }

    FileLineWriteOnlyStream stream(filename);

    try {
        stream.Open();

        for (int i = 0; i < count; ++i) {
            std::string line = readLineWithPrompt("  line[" + std::to_string(i) + "]: ");
            stream.Write(line);
        }

        stream.Close();
        std::cout << "  File created.\n";
    }
    catch (const std::exception& e) {
        printError(e.what());
    }
}

static void eventsMenu() {
    int choice;

    while (true) {
        printTitle("Events and online statistics");
        std::cout << "  1. EventParser\n";
        std::cout << "  2. EventReadOnlyStream\n";
        std::cout << "  3. OnlineEventStatistics manually\n";
        std::cout << "  4. ProtocolStatisticsTask with string stream\n";
        std::cout << "  5. ProtocolStatisticsTask with file\n";
        std::cout << "  6. Create protocol file\n";
        std::cout << "  0. Back\n";

        if (!readIntInRange(choice, "Choice: ", 1, 6)) {
            return;
        }

        switch (choice) {
        case 1: demoEventParser(); break;
        case 2: demoEventReadOnlyStream(); break;
        case 3: demoManualOnlineStatistics(); break;
        case 4: demoProtocolStatisticsStringStream(); break;
        case 5: demoProtocolStatisticsFile(); break;
        case 6: demoCreateProtocolFile(); break;
        }

        pauseConsole();
    }
}

int main() {
    setupConsole();

    int choice;

    while (true) {
        printTitle("Laboratory Work 4");
        std::cout << "  1. Lazy sequences and generators\n";
        std::cout << "  2. Streams\n";
        std::cout << "  3. Events and online statistics\n";
        std::cout << "  0. Exit\n";

        if (!readIntInRange(choice, "Choice: ", 1, 3)) {
            std::cout << "  Goodbye.\n";
            return 0;
        }

        switch (choice) {
        case 1: lazyMenu(); break;
        case 2: streamsMenu(); break;
        case 3: eventsMenu(); break;
        }
    }
}
