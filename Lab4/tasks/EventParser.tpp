#pragma once

#include <sstream>
#include "EventParser.h"

template <class T>
Event<T> EventParser<T>::ParseLine(const std::string& line) {
    std::istringstream input(line);

    std::string command;
    input >> command; // использовать readonlystream

    if (command == "START") {
        return Event<T>(EventType::Start, T(), "");
    }

    if (command == "END") {
        return Event<T>(EventType::End, T(), "");
    }

    if (command == "MEASURE") {
        T value;

        if (input >> value) {
            return Event<T>(EventType::Measure, value, "");
        }

        return Event<T>(EventType::Unknown, T(), line);
    }

    if (command == "ERROR") {
        std::string message;
        std::getline(input, message);

        if (!message.empty() && message[0] == ' ') {
            message.erase(0, 1);
        }

        return Event<T>(EventType::Error, T(), message);
    }

    return Event<T>(EventType::Unknown, T(), line);
}