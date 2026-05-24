#pragma once


#include "ProtocolStatisticsTask.h"


template <class T>
OnlineEventStatistics<T> ProtocolStatisticsTask<T>::Process(
    ReadOnlyStream<Event<T>>& stream
) {
    OnlineEventStatistics<T> statistics;

    stream.Open();

    try {
        while (!stream.IsEndOfStream()) {
            Event<T> event = stream.Read();
            statistics.AddEvent(event);
        }

        stream.Close();

        return statistics;
    }
    catch (...) {
        stream.Close();
        throw;
    }
}


template <class T>
OnlineEventStatistics<T> ProtocolStatisticsTask<T>::Process(
    ReadOnlyStream<std::string>& stream
) {
    EventReadOnlyStream<T> event_stream(&stream);

    return Process(event_stream);
}