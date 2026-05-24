#pragma once

#include <cstdlib>
#include <cerrno>


static bool IsParserSpace(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
}


static std::string GetFirstToken(const std::string& line) {
    int start = 0;

    while (start < static_cast<int>(line.size()) && IsParserSpace(line[start])) {
        ++start;
    }

    int end = start;

    while (end < static_cast<int>(line.size()) && !IsParserSpace(line[end])) {
        ++end;
    }

    return line.substr(start, end - start);
}


static std::string GetRestAfterFirstToken(const std::string& line) {
    int index = 0;

    while (index < static_cast<int>(line.size()) && IsParserSpace(line[index])) {
        ++index;
    }

    while (index < static_cast<int>(line.size()) && !IsParserSpace(line[index])) {
        ++index;
    }

    while (index < static_cast<int>(line.size()) && IsParserSpace(line[index])) {
        ++index;
    }

    return line.substr(index);
}


template <class T>
static bool TryParseValue(const std::string& text, T& value) {
    const char* begin = text.c_str();
    char* end = nullptr;

    errno = 0;

    double parsed = std::strtod(begin, &end);

    if (begin == end || errno == ERANGE) {
        return false;
    }

    while (*end != '\0') {
        if (!IsParserSpace(*end)) {
            return false;
        }

        ++end;
    }

    value = static_cast<T>(parsed);

    return true;
}


template <class T>
Event<T> EventParser<T>::ParseLine(const std::string& line) {
    std::string command = GetFirstToken(line);

    if (command == "START") {
        return Event<T>(EventType::Start, T(), "");
    }

    if (command == "END") {
        return Event<T>(EventType::End, T(), "");
    }

    if (command == "MEASURE") {
        std::string value_text = GetRestAfterFirstToken(line);

        T value;

        if (TryParseValue(value_text, value)) {
            return Event<T>(EventType::Measure, value, "");
        }

        return Event<T>(EventType::Unknown, T(), line);
    }

    if (command == "ERROR") {
        std::string message = GetRestAfterFirstToken(line);

        return Event<T>(EventType::Error, T(), message);
    }

    return Event<T>(EventType::Unknown, T(), line);
}