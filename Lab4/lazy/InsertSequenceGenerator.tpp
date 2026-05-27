#pragma once


static Cardinal InsertAddCardinal(const Cardinal& first, const Cardinal& second) {
    if (first.IsInfinite() || second.IsInfinite()) {
        return Cardinal::Infinity();
    }

    return Cardinal::Finite(first.GetFiniteValue() + second.GetFiniteValue());
}


template <class T>
InsertSequenceGenerator<T>::InsertSequenceGenerator(
    const LazySequence<T>& source,
    const LazySequence<T>& inserted,
    int index
)
    : InsertSequenceGenerator(
        source,
        inserted,
        TransfiniteIndex::Finite(index)
    ) {
}


template <class T>
InsertSequenceGenerator<T>::InsertSequenceGenerator(
    const LazySequence<T>& source,
    const LazySequence<T>& inserted,
    const TransfiniteIndex& index
)
    : source_(new LazySequence<T>(source)),
    inserted_(new LazySequence<T>(inserted)),
    source_length_(source.GetCardinality()),
    inserted_length_(inserted.GetCardinality()),
    result_length_(InsertAddCardinal(source.GetCardinality(), inserted.GetCardinality())),
    index_(index),
    position_(0)
{
    if (index_.IsFinite()) {
        int finite_index = index_.GetFiniteIndex();

        if (source_length_.IsFinite() && finite_index > source_length_.GetFiniteValue()) {
            delete source_;
            delete inserted_;

            source_ = nullptr;
            inserted_ = nullptr;

            throw std::out_of_range("InsertSequenceGenerator: index is out of range");
        }
    }
    else {
        if (source_length_.IsFinite()) {
            delete source_;
            delete inserted_;

            source_ = nullptr;
            inserted_ = nullptr;

            throw std::out_of_range(
                "InsertSequenceGenerator: transfinite index for finite sequence"
            );
        }
    }
}


template <class T>
InsertSequenceGenerator<T>::InsertSequenceGenerator(
    const InsertSequenceGenerator<T>& other
)
    : source_(other.source_ == nullptr ? nullptr : new LazySequence<T>(*other.source_)),
    inserted_(other.inserted_ == nullptr ? nullptr : new LazySequence<T>(*other.inserted_)),
    source_length_(other.source_length_),
    inserted_length_(other.inserted_length_),
    result_length_(other.result_length_),
    index_(other.index_),
    position_(other.position_) {
}


template <class T>
InsertSequenceGenerator<T>& InsertSequenceGenerator<T>::operator=(
    const InsertSequenceGenerator<T>& other
    ) {
    if (this == &other) {
        return *this;
    }

    LazySequence<T>* new_source = other.source_ == nullptr
        ? nullptr
        : new LazySequence<T>(*other.source_);

    LazySequence<T>* new_inserted = other.inserted_ == nullptr
        ? nullptr
        : new LazySequence<T>(*other.inserted_);

    delete source_;
    delete inserted_;

    source_ = new_source;
    inserted_ = new_inserted;

    source_length_ = other.source_length_;
    inserted_length_ = other.inserted_length_;
    result_length_ = other.result_length_;
    index_ = other.index_;
    position_ = other.position_;

    return *this;
}


template <class T>
InsertSequenceGenerator<T>::~InsertSequenceGenerator() {
    delete source_;
    delete inserted_;
}


template <class T>
bool InsertSequenceGenerator<T>::HasNext() const {
    if (result_length_.IsInfinite()) {
        return true;
    }

    return position_ < result_length_.GetFiniteValue();
}


template <class T>
T InsertSequenceGenerator<T>::GetNext() {
    if (!HasNext()) {
        throw std::out_of_range("InsertSequenceGenerator: no next element");
    }

    T value;

    if (index_.IsAfterInfinity()) {
        value = source_->Get(position_);
    }
    else {
        int insert_index = index_.GetFiniteIndex();

        if (position_ < insert_index) {
            value = source_->Get(position_);
        }
        else if (inserted_length_.IsInfinite()) {
            value = inserted_->Get(position_ - insert_index);
        }
        else {
            int inserted_length = inserted_length_.GetFiniteValue();

            if (position_ < insert_index + inserted_length) {
                value = inserted_->Get(position_ - insert_index);
            }
            else {
                value = source_->Get(position_ - inserted_length);
            }
        }
    }

    ++position_;

    return value;
}


template <class T>
int InsertSequenceGenerator<T>::GetPosition() const {
    return position_;
}


template <class T>
void InsertSequenceGenerator<T>::SetSource(const Sequence<T>&) {
}


template <class T>
void InsertSequenceGenerator<T>::Reset() {
    position_ = 0;
}


template <class T>
Generator<T>* InsertSequenceGenerator<T>::Clone() const {
    return new InsertSequenceGenerator<T>(*this);
}


template <class T>
T InsertSequenceGenerator<T>::GetByTransfiniteIndex(
    const TransfiniteIndex& index
) const {
    if (index.IsFinite()) {
        int finite_index = index.GetFiniteIndex();

        if (index_.IsAfterInfinity()) {
            return source_->Get(finite_index);
        }

        int insert_index = index_.GetFiniteIndex();

        if (finite_index < insert_index) {
            return source_->Get(finite_index);
        }

        if (inserted_length_.IsInfinite()) {
            return inserted_->Get(finite_index - insert_index);
        }

        int inserted_length = inserted_length_.GetFiniteValue();

        if (finite_index < insert_index + inserted_length) {
            return inserted_->Get(finite_index - insert_index);
        }

        return source_->Get(finite_index - inserted_length);
    }

    int tail_index = index.GetFiniteIndex();

    if (index_.IsAfterInfinity()) {
        int insert_infinity = index_.GetInfinityCount();
        int insert_offset = index_.GetFiniteIndex();

        if (index.GetInfinityCount() < insert_infinity) {
            return source_->Get(index);
        }

        if (index.GetInfinityCount() == insert_infinity) {
            if (tail_index < insert_offset) {
                return source_->Get(index);
            }

            if (inserted_length_.IsInfinite()) {
                return inserted_->Get(tail_index - insert_offset);
            }

            int inserted_length = inserted_length_.GetFiniteValue();

            if (tail_index < insert_offset + inserted_length) {
                return inserted_->Get(tail_index - insert_offset);
            }

            return source_->Get(
                TransfiniteIndex(insert_infinity, tail_index - inserted_length)
            );
        }

        if (inserted_length_.IsInfinite() &&
            index.GetInfinityCount() == insert_infinity + 1) {
            return source_->Get(
                TransfiniteIndex(insert_infinity, insert_offset + tail_index)
            );
        }

        return source_->Get(index);
    }

    int insert_index = index_.GetFiniteIndex();

    if (inserted_length_.IsInfinite()) {
        if (index.GetInfinityCount() == 1) {
            return source_->Get(insert_index + tail_index);
        }

        return source_->Get(
            TransfiniteIndex(index.GetInfinityCount() - 1, tail_index)
        );
    }

    return source_->Get(index);
}


template <class T>
Cardinal InsertSequenceGenerator<T>::GetResultCardinality() const {
    return result_length_;
}