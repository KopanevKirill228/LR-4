#pragma once


template <class T>
bool LiveFileBatchConsumer<T>::TryOpenFile(
    std::ifstream& input,
    const std::string& filename
) {
    input.open(filename.c_str());

    return input.is_open();
}


template <class T>
void LiveFileBatchConsumer<T>::ProcessEvent(
    const Event<T>& event,
    OnlineEventStatistics<T>& statistics,
    T(*valueMapper)(const T&)
) {
    if (event.type == EventType::Measure) {
        Event<T> mappedEvent(
            EventType::Measure,
            valueMapper(event.value),
            event.message
        );

        statistics.AddEvent(mappedEvent);
    }
    else {
        statistics.AddEvent(event);
    }
}


template <class T>
void LiveFileBatchConsumer<T>::Notify(
    void (*messageHandler)(const LiveConsumerMessage<T>&),
    const LiveConsumerMessage<T>& message
) {
    if (messageHandler != nullptr) {
        messageHandler(message);
    }
}


template <class T>
LiveConsumerMessage<T> LiveFileBatchConsumer<T>::MakeMessage(
    LiveConsumerMessageType type,
    const std::string& filename,
    const std::string& line,
    int currentBatchSize,
    const OnlineEventStatistics<T>& statistics
) {
    LiveConsumerMessage<T> message;

    message.type = type;
    message.filename = filename;
    message.line = line;
    message.totalEvents = statistics.GetTotalEvents();
    message.currentBatchSize = currentBatchSize;
    message.hasMeasurements = statistics.HasMeasurements();
    message.currentMax = statistics.HasMeasurements()
        ? statistics.GetMaxMeasure()
        : T();

    return message;
}


template <class T>
OnlineEventStatistics<T> LiveFileBatchConsumer<T>::Run(
    const std::string& liveFilename,
    int batchSize,
    T(*valueMapper)(const T&),
    void (*messageHandler)(const LiveConsumerMessage<T>&)
) {
    if (batchSize <= 0) {
        throw std::invalid_argument("LiveFileBatchConsumer: batch size must be positive");
    }

    if (valueMapper == nullptr) {
        throw std::invalid_argument("LiveFileBatchConsumer: mapper is nullptr");
    }

    OnlineEventStatistics<T> statistics;

    Notify(
        messageHandler,
        MakeMessage(
            LiveConsumerMessageType::WaitingForFile,
            liveFilename,
            "",
            0,
            statistics
        )
    );

    std::ifstream input;

    while (!TryOpenFile(input, liveFilename)) {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(200)
        );
    }

    Notify(
        messageHandler,
        MakeMessage(
            LiveConsumerMessageType::FileOpened,
            liveFilename,
            "",
            0,
            statistics
        )
    );

    int currentBatchSize = 0;
    bool finished = false;

    while (!finished) {
        std::string line;

        if (std::getline(input, line)) {
            if (line.empty()) {
                continue;
            }

            Notify(
                messageHandler,
                MakeMessage(
                    LiveConsumerMessageType::LineRead,
                    liveFilename,
                    line,
                    currentBatchSize,
                    statistics
                )
            );

            Event<T> event = EventParser<T>::ParseLine(line);

            ProcessEvent(
                event,
                statistics,
                valueMapper
            );

            ++currentBatchSize;

            if (currentBatchSize == batchSize || event.type == EventType::End) {
                Notify(
                    messageHandler,
                    MakeMessage(
                        LiveConsumerMessageType::BatchProcessed,
                        liveFilename,
                        "",
                        currentBatchSize,
                        statistics
                    )
                );

                currentBatchSize = 0;
            }

            if (event.type == EventType::End) {
                finished = true;
            }
        }
        else {
            input.clear();

            std::this_thread::sleep_for(
                std::chrono::milliseconds(200)
            );
        }
    }

    input.close();

    Notify(
        messageHandler,
        MakeMessage(
            LiveConsumerMessageType::Finished,
            liveFilename,
            "",
            currentBatchSize,
            statistics
        )
    );

    return statistics;
}