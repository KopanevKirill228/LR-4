#pragma once

#include "OnlineEventStatistics.h"


template <class T>
bool OnlineEventStatistics<T>::MaxCompare(const T& a, const T& b) {
    return a > b;
}


template <class T>
OnlineEventStatistics<T>::OnlineEventStatistics()
    : total_events_(0),
    start_events_(0),
    end_events_(0),
    measure_events_(0),
    error_events_(0),
    unknown_events_(0),
    measure_sum_(T()),
    min_heap_(),
    max_heap_(MaxCompare) {
}


template <class T>
void OnlineEventStatistics<T>::AddEvent(const Event<T>& event) {
    ++total_events_;

    if (event.type == EventType::Start) {
        ++start_events_;
    }
    else if (event.type == EventType::End) {
        ++end_events_;
    }
    else if (event.type == EventType::Measure) {
        ++measure_events_;
        measure_sum_ = measure_sum_ + event.value;

        min_heap_.Push(event.value);
        max_heap_.Push(event.value);
    }
    else if (event.type == EventType::Error) {
        ++error_events_;
    }
    else {
        ++unknown_events_;
    }
}


template <class T>
void OnlineEventStatistics<T>::Clear() {
    total_events_ = 0;
    start_events_ = 0;
    end_events_ = 0;
    measure_events_ = 0;
    error_events_ = 0;
    unknown_events_ = 0;

    measure_sum_ = T();

    min_heap_.Clear();
    max_heap_.Clear();
}


template <class T>
int OnlineEventStatistics<T>::GetTotalEvents() const {
    return total_events_;
}


template <class T>
int OnlineEventStatistics<T>::GetStartEvents() const {
    return start_events_;
}


template <class T>
int OnlineEventStatistics<T>::GetEndEvents() const {
    return end_events_;
}


template <class T>
int OnlineEventStatistics<T>::GetMeasureEvents() const {
    return measure_events_;
}


template <class T>
int OnlineEventStatistics<T>::GetErrorEvents() const {
    return error_events_;
}


template <class T>
int OnlineEventStatistics<T>::GetUnknownEvents() const {
    return unknown_events_;
}


template <class T>
bool OnlineEventStatistics<T>::HasMeasurements() const {
    return measure_events_ > 0;
}


template <class T>
const T& OnlineEventStatistics<T>::GetMinMeasure() const {
    if (!HasMeasurements()) {
        throw std::out_of_range("OnlineEventStatistics: no measurements");
    }

    return min_heap_.Peek();
}


template <class T>
const T& OnlineEventStatistics<T>::GetMaxMeasure() const {
    if (!HasMeasurements()) {
        throw std::out_of_range("OnlineEventStatistics: no measurements");
    }

    return max_heap_.Peek();
}


template <class T>
T OnlineEventStatistics<T>::GetAverageMeasure() const {
    if (!HasMeasurements()) {
        throw std::out_of_range("OnlineEventStatistics: no measurements");
    }

    return measure_sum_ / measure_events_;
}