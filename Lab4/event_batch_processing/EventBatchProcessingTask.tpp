#pragma once


template <class T>
void EventBatchProcessingTask<T>::AddBatchToStatistics(
    ReadOnlyStream<Event<T>>& source,
    int batchSize,
    OnlineEventStatistics<T>& statistics
) {
    int currentBatchSize = 0;

    while (!source.IsEndOfStream() && currentBatchSize < batchSize) {
        Event<T> event = source.Read();

        statistics.AddEvent(event);

        ++currentBatchSize;
    }
}


template <class T>
OnlineEventStatistics<T> EventBatchProcessingTask<T>::Process(
    ReadOnlyStream<std::string>& source,
    int batchSize,
    T(*valueMapper)(const T&)
) {
    if (batchSize <= 0) {
        throw std::invalid_argument("EventBatchProcessingTask: batch size must be positive");
    }

    if (valueMapper == nullptr) {
        throw std::invalid_argument("EventBatchProcessingTask: mapper is nullptr");
    }

    EventReadOnlyStream<T> eventStream(&source);

    return Process(eventStream, batchSize, valueMapper);
}


template <class T>
OnlineEventStatistics<T> EventBatchProcessingTask<T>::Process(
    ReadOnlyStream<Event<T>>& source,
    int batchSize,
    T(*valueMapper)(const T&)
) {
    if (batchSize <= 0) {
        throw std::invalid_argument("EventBatchProcessingTask: batch size must be positive");
    }

    if (valueMapper == nullptr) {
        throw std::invalid_argument("EventBatchProcessingTask: mapper is nullptr");
    }

    EventMapReadOnlyStream<T> mappedStream(
        &source,
        valueMapper
    );

    OnlineEventStatistics<T> statistics;

    mappedStream.Open();

    try {
        while (!mappedStream.IsEndOfStream()) {
            AddBatchToStatistics(
                mappedStream,
                batchSize,
                statistics
            );
        }

        mappedStream.Close();

        return statistics;
    }
    catch (...) {
        mappedStream.Close();
        throw;
    }
}


template <class T>
OnlineEventStatistics<T> EventBatchProcessingTask<T>::Process(
    ReadOnlyStream<std::string>& source,
    int batchSize
) {
    if (batchSize <= 0) {
        throw std::invalid_argument("EventBatchProcessingTask: batch size must be positive");
    }

    EventReadOnlyStream<T> eventStream(&source);

    return Process(eventStream, batchSize);
}


template <class T>
OnlineEventStatistics<T> EventBatchProcessingTask<T>::Process(
    ReadOnlyStream<Event<T>>& source,
    int batchSize
) {
    if (batchSize <= 0) {
        throw std::invalid_argument("EventBatchProcessingTask: batch size must be positive");
    }

    OnlineEventStatistics<T> statistics;

    source.Open();

    try {
        while (!source.IsEndOfStream()) {
            AddBatchToStatistics(
                source,
                batchSize,
                statistics
            );
        }

        source.Close();

        return statistics;
    }
    catch (...) {
        source.Close();
        throw;
    }
}