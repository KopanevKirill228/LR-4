#pragma once

#include "TransfiniteIndex.h"
#include "Cardinal.h"

#include <stdexcept>


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

    Cardinal ToCardinal() const {
        if (IsInfinite()) {
            return Cardinal::Infinity();
        }

        return Cardinal::Finite(finite_count_);
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

        int index_infinity = index.GetInfinityCount();
        int index_finite = index.GetFiniteIndex();

        if (index_infinity < infinity_count_) {
            return true;
        }

        if (index_infinity == infinity_count_) {
            return index_finite < finite_count_;
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

        int index_infinity = index.GetInfinityCount();
        int index_finite = index.GetFiniteIndex();

        if (index_infinity == infinity_count_) {
            return TransfiniteIndex::Finite(index_finite - finite_count_);
        }

        return TransfiniteIndex(
            index_infinity - infinity_count_,
            index_finite
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