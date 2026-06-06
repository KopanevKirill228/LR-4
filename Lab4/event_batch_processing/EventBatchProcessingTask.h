#pragma once

#include "../streams/ReadOnlyStream.h"

#include "../tasks/Event.h"
#include "../tasks/EventReadOnlyStream.h"
#include "../tasks/EventMapReadOnlyStream.h"
#include "../tasks/OnlineEventStatistics.h"

#include <stdexcept>
#include <string>


template <class T>
class EventBatchProcessingTask {
private:
    static void AddBatchToStatistics(
        ReadOnlyStream<Event<T>>& source,
        int batchSize,
        OnlineEventStatistics<T>& statistics
    );

public:
    static OnlineEventStatistics<T> Process(
        ReadOnlyStream<std::string>& source,
        int batchSize,
        T(*valueMapper)(const T&)
    );

    static OnlineEventStatistics<T> Process(
        ReadOnlyStream<Event<T>>& source,
        int batchSize,
        T(*valueMapper)(const T&)
    );

    static OnlineEventStatistics<T> Process(
        ReadOnlyStream<std::string>& source,
        int batchSize
    );

    static OnlineEventStatistics<T> Process(
        ReadOnlyStream<Event<T>>& source,
        int batchSize
    );
};

#include "EventBatchProcessingTask.tpp"