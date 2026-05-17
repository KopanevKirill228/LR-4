#pragma once

#include "Event.h"
#include "../lib/BinaryHeap.h"

#include <stdexcept>


template <class T>
class OnlineEventStatistics {
private:
    int total_events_;
    int start_events_;
    int end_events_;
    int measure_events_;
    int error_events_;
    int unknown_events_;

    T measure_sum_;

    BinaryHeap<T> min_heap_;
    BinaryHeap<T> max_heap_;

    static bool MaxCompare(const T& a, const T& b);

public:
    OnlineEventStatistics();

    void AddEvent(const Event<T>& event);
    void Clear();

    int GetTotalEvents() const;
    int GetStartEvents() const;
    int GetEndEvents() const;
    int GetMeasureEvents() const;
    int GetErrorEvents() const;
    int GetUnknownEvents() const;

    bool HasMeasurements() const;

    const T& GetMinMeasure() const;
    const T& GetMaxMeasure() const;
    T GetAverageMeasure() const;
};

#include "OnlineEventStatistics.tpp"