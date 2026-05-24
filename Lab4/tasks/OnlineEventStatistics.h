#pragma once

#include "../lib/BinaryHeap.h"
#include "Event.h"

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
    T measure_square_sum_;

    T min_measure_;
    T max_measure_;
    bool has_measurements_;

    BinaryHeap<T> lower_half_;
    BinaryHeap<T> upper_half_;

    static bool GreaterPriority(const T& first, const T& second);

    void AddMeasure(const T& value);
    void RebalanceMedianHeaps();

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

    T GetMinMeasure() const;
    T GetMaxMeasure() const;
    T GetAverageMeasure() const;
    T GetVarianceMeasure() const;
    T GetMedianMeasure() const;
};

#include "OnlineEventStatistics.tpp"