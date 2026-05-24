#pragma once


template <class T>
bool OnlineEventStatistics<T>::GreaterPriority(const T& first, const T& second) {
    return first > second;
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
    measure_square_sum_(T()),
    min_measure_(T()),
    max_measure_(T()),
    has_measurements_(false),
    lower_half_(GreaterPriority),
    upper_half_() {
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
        AddMeasure(event.value);
    }
    else if (event.type == EventType::Error) {
        ++error_events_;
    }
    else {
        ++unknown_events_;
    }
}


template <class T>
void OnlineEventStatistics<T>::AddMeasure(const T& value) {
    measure_sum_ = measure_sum_ + value;
    measure_square_sum_ = measure_square_sum_ + value * value;

    if (!has_measurements_) {
        min_measure_ = value;
        max_measure_ = value;
        has_measurements_ = true;
    }
    else {
        if (value < min_measure_) {
            min_measure_ = value;
        }

        if (value > max_measure_) {
            max_measure_ = value;
        }
    }

    if (lower_half_.IsEmpty() || value <= lower_half_.Peek()) {
        lower_half_.Push(value);
    }
    else {
        upper_half_.Push(value);
    }

    RebalanceMedianHeaps();
}


template <class T>
void OnlineEventStatistics<T>::RebalanceMedianHeaps() {
    if (lower_half_.GetCount() > upper_half_.GetCount() + 1) {
        upper_half_.Push(lower_half_.Pop());
    }
    else if (upper_half_.GetCount() > lower_half_.GetCount() + 1) {
        lower_half_.Push(upper_half_.Pop());
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
    measure_square_sum_ = T();

    min_measure_ = T();
    max_measure_ = T();
    has_measurements_ = false;

    lower_half_ = BinaryHeap<T>(GreaterPriority);
    upper_half_ = BinaryHeap<T>();
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
    return has_measurements_;
}


template <class T>
T OnlineEventStatistics<T>::GetMinMeasure() const {
    if (!has_measurements_) {
        throw std::logic_error("OnlineEventStatistics: no measurements");
    }

    return min_measure_;
}


template <class T>
T OnlineEventStatistics<T>::GetMaxMeasure() const {
    if (!has_measurements_) {
        throw std::logic_error("OnlineEventStatistics: no measurements");
    }

    return max_measure_;
}


template <class T>
T OnlineEventStatistics<T>::GetAverageMeasure() const {
    if (!has_measurements_) {
        throw std::logic_error("OnlineEventStatistics: no measurements");
    }

    return measure_sum_ / static_cast<T>(measure_events_);
}


template <class T>
T OnlineEventStatistics<T>::GetVarianceMeasure() const {
    if (!has_measurements_) {
        throw std::logic_error("OnlineEventStatistics: no measurements");
    }

    T average = GetAverageMeasure();
    T square_average = measure_square_sum_ / static_cast<T>(measure_events_);

    return square_average - average * average;
}


template <class T>
T OnlineEventStatistics<T>::GetMedianMeasure() const {
    if (!has_measurements_) {
        throw std::logic_error("OnlineEventStatistics: no measurements");
    }

    if (lower_half_.GetCount() > upper_half_.GetCount()) {
        return lower_half_.Peek();
    }

    if (upper_half_.GetCount() > lower_half_.GetCount()) {
        return upper_half_.Peek();
    }

    return (lower_half_.Peek() + upper_half_.Peek()) / static_cast<T>(2);
}