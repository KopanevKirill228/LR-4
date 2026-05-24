#pragma once

#include "OnlineEventStatistics.h"
#include "EventReadOnlyStream.h"
#include "../streams/ReadOnlyStream.h"

#include <string>


template <class T>
class ProtocolStatisticsTask {
public:
    static OnlineEventStatistics<T> Process(ReadOnlyStream<Event<T>>& stream);
    static OnlineEventStatistics<T> Process(ReadOnlyStream<std::string>& stream);
};

#include "ProtocolStatisticsTask.tpp"