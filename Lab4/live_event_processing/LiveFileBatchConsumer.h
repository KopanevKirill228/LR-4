#pragma once

#include "../tasks/Event.h"
#include "../tasks/EventType.h"
#include "../tasks/EventParser.h"
#include "../tasks/OnlineEventStatistics.h"

#include <chrono>
#include <fstream>
#include <stdexcept>
#include <string>
#include <thread>


enum class LiveConsumerMessageType {
    WaitingForFile,
    FileOpened,
    LineRead,
    BatchProcessed,
    Finished
};


template <class T>
struct LiveConsumerMessage {
    LiveConsumerMessageType type;

    std::string filename;
    std::string line;

    int totalEvents;
    int currentBatchSize;

    bool hasMeasurements;
    T currentMax;
};


template <class T>
class LiveFileBatchConsumer {
private:
    static bool TryOpenFile(
        std::ifstream& input,
        const std::string& filename
    );

    static void ProcessEvent(
        const Event<T>& event,
        OnlineEventStatistics<T>& statistics,
        T(*valueMapper)(const T&)
    );

    static void Notify(
        void (*messageHandler)(const LiveConsumerMessage<T>&),
        const LiveConsumerMessage<T>& message
    );

    static LiveConsumerMessage<T> MakeMessage(
        LiveConsumerMessageType type,
        const std::string& filename,
        const std::string& line,
        int currentBatchSize,
        const OnlineEventStatistics<T>& statistics
    );

public:
    static OnlineEventStatistics<T> Run(
        const std::string& liveFilename,
        int batchSize,
        T(*valueMapper)(const T&),
        void (*messageHandler)(const LiveConsumerMessage<T>&)
    );
};

#include "LiveFileBatchConsumer.tpp"