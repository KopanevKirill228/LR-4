#pragma once

#include <stdexcept>


class Cardinal {
private:
    int value_;
    bool infinite_;

    Cardinal(int value, bool infinite)
        : value_(value), infinite_(infinite) {
    }

public:
    static Cardinal Finite(int value) {
        if (value < 0) {
            throw std::invalid_argument("Cardinal: finite value must be non-negative");
        }

        return Cardinal(value, false);
    }

    static Cardinal Infinity() {
        return Cardinal(0, true);
    }

    bool IsInfinite() const {
        return infinite_;
    }

    bool IsFinite() const {
        return !infinite_;
    }

    int GetFiniteValue() const {
        if (infinite_) {
            throw std::logic_error("Cardinal: infinity has no finite value");
        }

        return value_;
    }
};