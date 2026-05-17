#pragma once

#include "OnlineEventStatistics.h"
#include "../streams/ReadOnlyStream.h"

#include <string>


template <class T>
class ProtocolStatisticsTask {
public:
    static OnlineEventStatistics<T> Process(ReadOnlyStream<std::string>& stream);
};

#include "ProtocolStatisticsTask.tpp"