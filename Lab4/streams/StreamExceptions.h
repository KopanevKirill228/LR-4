#pragma once

#include <stdexcept>


class EndOfStreamException : public std::runtime_error {
public:
    EndOfStreamException()
        : std::runtime_error("End of stream") {
    }
};


class StreamIsClosedException : public std::runtime_error {
public:
    StreamIsClosedException()
        : std::runtime_error("Stream is closed") {
    }
};


class StreamSeekException : public std::runtime_error {
public:
    StreamSeekException()
        : std::runtime_error("Cannot seek stream") {
    }
};