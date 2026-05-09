#pragma once

#include "WriteOnlyStream.h"
#include "StreamExceptions.h"
#include "lib/Sequence.h"

#include <stdexcept>


template <class T>
class SequenceWriteOnlyStream : public WriteOnlyStream<T> {
private:
    Sequence<T>*& sequence_;
    int position_;
    bool is_open_;
    bool delete_replaced_;

public:
    SequenceWriteOnlyStream(Sequence<T>*& sequence, bool deleteReplaced = true);

    SequenceWriteOnlyStream(const SequenceWriteOnlyStream<T>& other) = delete;
    SequenceWriteOnlyStream<T>& operator=(const SequenceWriteOnlyStream<T>& other) = delete;

    void Open() override;
    void Close() override;

    int GetPosition() const override;
    int Write(const T& item) override;
};

#include "SequenceWriteOnlyStream.tpp"