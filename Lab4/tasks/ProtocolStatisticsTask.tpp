#pragma once

#include "EventParser.h"
#include "../streams/StreamExceptions.h"
#include "ProtocolStatisticsTask.h"


template <class T>
OnlineEventStatistics<T> ProtocolStatisticsTask<T>::Process(ReadOnlyStream<std::string>& stream) {
    OnlineEventStatistics<T> statistics;

    stream.Open();

    try {
        while (true) {
            std::string line = stream.Read();
            Event<T> event = EventParser<T>::ParseLine(line);
            statistics.AddEvent(event);
        }
    }
    catch (const EndOfStreamException&) {
        stream.Close();
        return statistics;
    }
    catch (...) {
        stream.Close();
        throw;
    }
}