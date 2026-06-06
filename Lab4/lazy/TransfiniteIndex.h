#pragma once

#include <stdexcept>


class TransfiniteIndex {
private:
    int infinity_count_;
    int finite_index_;

public:
    TransfiniteIndex(int finite_index)
        : infinity_count_(0),
        finite_index_(finite_index)
    {
        if (finite_index_ < 0) {
            throw std::out_of_range("TransfiniteIndex: finite index is negative");
        }
    }

    TransfiniteIndex(int infinity_count, int finite_index)
        : infinity_count_(infinity_count),
        finite_index_(finite_index)
    {
        if (infinity_count_ < 0) {
            throw std::out_of_range("TransfiniteIndex: infinity count is negative");
        }

        if (finite_index_ < 0) {
            throw std::out_of_range("TransfiniteIndex: finite index is negative");
        }
    }

    bool IsFinite() const {
        return infinity_count_ == 0;
    }

    bool IsAfterInfinity() const {
        return infinity_count_ > 0;
    }

    int GetInfinityCount() const {
        return infinity_count_;
    }

    int GetFiniteIndex() const {
        return finite_index_;
    }

    static TransfiniteIndex Finite(int index) {
        return TransfiniteIndex(index);
    }

    static TransfiniteIndex AfterInfinity(int index) {
        return TransfiniteIndex(1, index);
    }

    static TransfiniteIndex AfterInfinities(int infinityCount, int index) {
        if (infinityCount <= 0) {
            throw std::out_of_range("TransfiniteIndex: infinity count must be positive");
        }

        return TransfiniteIndex(infinityCount, index);
    }

    TransfiniteIndex DropInfinities(int count) const {
        if (count < 0) {
            throw std::out_of_range("TransfiniteIndex: drop count is negative");
        }

        if (count > infinity_count_) {
            throw std::out_of_range("TransfiniteIndex: cannot drop more infinities than index has");
        }

        return TransfiniteIndex(infinity_count_ - count, finite_index_);
    }

    bool operator==(const TransfiniteIndex& other) const {
        return infinity_count_ == other.infinity_count_
            && finite_index_ == other.finite_index_;
    }

    bool operator!=(const TransfiniteIndex& other) const {
        return !(*this == other);
    }
};