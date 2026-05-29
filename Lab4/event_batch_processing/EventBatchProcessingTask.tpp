#pragma once


template <class T>
Sequence<std::string>* EventBatchProcessingTask<T>::ReadBatch(
    ReadOnlyStream<std::string>& source,
    int batchSize)
{
    Sequence<std::string>* lines = new MutableArraySequence<std::string>();

    try {
        int count = 0;

        while (count < batchSize && !source.IsEndOfStream()) {
            AppendToResult(lines, source.Read());
            ++count;
        }

        return lines;
    }
    catch (...) {
        delete lines;
        throw;
    }
}


template <class T>
Sequence<Event<T>>* EventBatchProcessingTask<T>::ParseEvents(
    const Sequence<std::string>* lines)
{
    return Map<std::string, Event<T>>(
        lines,
        [](const std::string& line) {
            return EventParser<T>::ParseLine(line);
        },
        []() -> Sequence<Event<T>>*{
            return new MutableArraySequence<Event<T>>();
        }
    );
}


template <class T>
Sequence<Event<T>>* EventBatchProcessingTask<T>::MapMeasureValues(
    const Sequence<Event<T>>* events,
    std::function<T(const T&)> valueMapper)
{
    if (!valueMapper) {
        throw std::invalid_argument("EventBatchProcessingTask: value mapper is empty");
    }

    return Map<Event<T>, Event<T>>(
        events,
        [valueMapper](const Event<T>& event) {
            if (event.type != EventType::Measure) {
                return event;
            }

            return Event<T>(
                EventType::Measure,
                valueMapper(event.value),
                event.message
            );
        },
        []() -> Sequence<Event<T>>*{
            return new MutableArraySequence<Event<T>>();
        }
    );
}


template <class T>
void EventBatchProcessingTask<T>::AddEventsToStatistics(
    Sequence<Event<T>>* events,
    OnlineEventStatistics<T>& statistics)
{
    SequenceReadOnlyStream<Event<T>> stream(events);
    stream.Open();

    try {
        while (!stream.IsEndOfStream()) {
            statistics.AddEvent(stream.Read());
        }

        stream.Close();
    }
    catch (...) {
        stream.Close();
        throw;
    }
}


template <class T>
OnlineEventStatistics<T> EventBatchProcessingTask<T>::Process(
    ReadOnlyStream<std::string>& source,
    int batchSize,
    std::function<T(const T&)> valueMapper)
{
    if (batchSize <= 0) {
        throw std::invalid_argument("EventBatchProcessingTask: batch size must be positive");
    }

    if (!valueMapper) {
        throw std::invalid_argument("EventBatchProcessingTask: value mapper is empty");
    }

    OnlineEventStatistics<T> statistics;

    source.Open();

    try {
        while (!source.IsEndOfStream()) {
            Sequence<std::string>* lines = ReadBatch(source, batchSize);
            Sequence<Event<T>>* events = nullptr;
            Sequence<Event<T>>* mappedEvents = nullptr;

            try {
                events = ParseEvents(lines);
                mappedEvents = MapMeasureValues(events, valueMapper);

                AddEventsToStatistics(mappedEvents, statistics);

                delete mappedEvents;
                delete events;
                delete lines;
            }
            catch (...) {
                delete mappedEvents;
                delete events;
                delete lines;
                throw;
            }
        }

        source.Close();
        return statistics;
    }
    catch (...) {
        source.Close();
        throw;
    }
}


template <class T>
OnlineEventStatistics<T> EventBatchProcessingTask<T>::Process(
    ReadOnlyStream<std::string>& source,
    int batchSize)
{
    return Process(
        source,
        batchSize,
        [](const T& value) {
            return value;
        }
    );
}