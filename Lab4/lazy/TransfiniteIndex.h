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


class TransfiniteLength {
private:
    int infinity_count_;
    int finite_count_;

public:
    TransfiniteLength()
        : infinity_count_(0),
        finite_count_(0) {
    }

    TransfiniteLength(int infinityCount, int finiteCount)
        : infinity_count_(infinityCount),
        finite_count_(finiteCount)
    {
        if (infinity_count_ < 0) {
            throw std::out_of_range("TransfiniteLength: infinity count is negative");
        }

        if (finite_count_ < 0) {
            throw std::out_of_range("TransfiniteLength: finite count is negative");
        }
    }

    static TransfiniteLength Finite(int value) {
        return TransfiniteLength(0, value);
    }

    static TransfiniteLength Omega() {
        return TransfiniteLength(1, 0);
    }

    static TransfiniteLength Infinite(int infinityCount, int finiteTail) {
        if (infinityCount <= 0) {
            throw std::out_of_range("TransfiniteLength: infinity count must be positive");
        }

        return TransfiniteLength(infinityCount, finiteTail);
    }

    bool IsFinite() const {
        return infinity_count_ == 0;
    }

    bool IsInfinite() const {
        return infinity_count_ > 0;
    }

    int GetInfinityCount() const {
        return infinity_count_;
    }

    int GetFiniteCount() const {
        return finite_count_;
    }

    bool Contains(const TransfiniteIndex& index) const {
        if (index.IsFinite()) {
            if (IsInfinite()) {
                return true;
            }

            return index.GetFiniteIndex() < finite_count_;
        }

        if (IsFinite()) {
            return false;
        }

        int indexInfinity = index.GetInfinityCount();
        int indexFinite = index.GetFiniteIndex();

        if (indexInfinity < infinity_count_) {
            return true;
        }

        if (indexInfinity == infinity_count_) {
            return indexFinite < finite_count_;
        }

        return false;
    }

    TransfiniteIndex SubtractFrom(const TransfiniteIndex& index) const {
        if (Contains(index)) {
            throw std::logic_error("TransfiniteLength: index is inside length");
        }

        if (IsFinite()) {
            if (index.IsFinite()) {
                return TransfiniteIndex::Finite(
                    index.GetFiniteIndex() - finite_count_
                );
            }

            return index;
        }

        int indexInfinity = index.GetInfinityCount();
        int indexFinite = index.GetFiniteIndex();

        if (indexInfinity == infinity_count_) {
            return TransfiniteIndex::Finite(indexFinite - finite_count_);
        }

        return TransfiniteIndex(
            indexInfinity - infinity_count_,
            indexFinite
        );
    }

    static TransfiniteLength Add(
        const TransfiniteLength& first,
        const TransfiniteLength& second)
    {
        if (second.IsFinite()) {
            return TransfiniteLength(
                first.infinity_count_,
                first.finite_count_ + second.finite_count_
            );
        }

        return TransfiniteLength(
            first.infinity_count_ + second.infinity_count_,
            second.finite_count_
        );
    }
};