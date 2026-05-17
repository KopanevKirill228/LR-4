#pragma once

#include "Event.h"

#include <string>


template <class T>
class EventParser {
public:
    static Event<T> ParseLine(const std::string& line);
};

#include "EventParser.tpp"