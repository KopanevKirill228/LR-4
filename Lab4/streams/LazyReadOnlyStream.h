#pragma once

#include "ReadOnlyStream.h"
#include "StreamExceptions.h"
#include "../lib/Sequence.h"

#include <stdexcept>


template <class T>
class LazyReadOnlyStream : public ReadOnlyStream<T> {
private:
    const LazySequence<T>* sequence_;
    int position_;
    bool is_open_;

public:
    LazyReadOnlyStream(const LazySequence<T>* sequence);

    void Open() override;
    void Close() override;

    bool IsEndOfStream() const override;
    T Read() override;

    int GetPosition() const override;

    bool IsCanSeek() const override;
    int Seek(int index) override;

    bool IsCanGoBack() const override;
};

#include "LazyReadOnlyStream.tpp"