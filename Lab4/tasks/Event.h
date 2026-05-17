#pragma once

#include "EventType.h"

#include <string>


template <class T>
struct Event {
    EventType type;
    T value;
    std::string message;

    Event()
        : type(EventType::Unknown), value(T()), message("") {
    }

    Event(EventType eventType, const T& eventValue, const std::string& eventMessage)
        : type(eventType), value(eventValue), message(eventMessage) {
    }
};