#pragma once

#include "../lib/Sequence.h"
#include "../lib/ArraySequence.h"
#include "../lib/MapReduce.h"

#include "../streams/ReadOnlyStream.h"
#include "../streams/SequenceReadOnlyStream.h"

#include "../tasks/Event.h"
#include "../tasks/EventType.h"
#include "../tasks/EventParser.h"
#include "../tasks/OnlineEventStatistics.h"

#include <functional>
#include <stdexcept>
#include <string>


template <class T>
class EventBatchProcessingTask {
private:
    static Sequence<std::string>* ReadBatch(
        ReadOnlyStream<std::string>& source,
        int batchSize
    );

    static Sequence<Event<T>>* ParseEvents(
        const Sequence<std::string>* lines
    );

    static Sequence<Event<T>>* MapMeasureValues(
        const Sequence<Event<T>>* events,
        std::function<T(const T&)> valueMapper
    );

    static void AddEventsToStatistics(
        Sequence<Event<T>>* events,
        OnlineEventStatistics<T>& statistics
    );

public:
    static OnlineEventStatistics<T> Process(
        ReadOnlyStream<std::string>& source,
        int batchSize,
        std::function<T(const T&)> valueMapper
    );

    static OnlineEventStatistics<T> Process(
        ReadOnlyStream<std::string>& source,
        int batchSize
    );
};

#include "EventBatchProcessingTask.tpp"