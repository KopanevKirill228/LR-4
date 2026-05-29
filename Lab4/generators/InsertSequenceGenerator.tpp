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
    source_transfinite_length_(source.GetTransfiniteLength()),
    inserted_transfinite_length_(inserted.GetTransfiniteLength()),
    result_transfinite_length_(TransfiniteLength::Finite(0)),
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

    result_transfinite_length_ = GetResultLength();
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
    source_transfinite_length_(other.source_transfinite_length_),
    inserted_transfinite_length_(other.inserted_transfinite_length_),
    result_transfinite_length_(other.result_transfinite_length_),
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
    source_transfinite_length_ = other.source_transfinite_length_;
    inserted_transfinite_length_ = other.inserted_transfinite_length_;
    result_transfinite_length_ = other.result_transfinite_length_;

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
T InsertSequenceGenerator<T>::GetAfterInfinite(int index) const {
    if (index < 0) {
        throw std::out_of_range("InsertSequenceGenerator: after-infinity index is negative");
    }

    return GetByTransfiniteIndex(
        TransfiniteIndex::AfterInfinity(index)
    );
}


template <class T>
T InsertSequenceGenerator<T>::GetByTransfiniteIndex(
    const TransfiniteIndex& index
) const {
    if (index_.IsFinite()) {
        int insert_index = index_.GetFiniteIndex();

        if (index.IsFinite()) {
            int finite_index = index.GetFiniteIndex();

            if (finite_index < insert_index) {
                return source_->Get(finite_index);
            }

            int inserted_finite_index = finite_index - insert_index;

            if (inserted_transfinite_length_.IsFinite()) {
                int inserted_count = inserted_transfinite_length_.GetFiniteCount();

                if (inserted_finite_index < inserted_count) {
                    return inserted_->Get(inserted_finite_index);
                }

                return source_->Get(finite_index - inserted_count);
            }

            return inserted_->Get(inserted_finite_index);
        }

        if (inserted_transfinite_length_.Contains(index)) {
            if (index.IsFinite()) {
                return inserted_->Get(index.GetFiniteIndex());
            }

            return inserted_->Get(index);
        }

        TransfiniteIndex after_inserted =
            inserted_transfinite_length_.SubtractFrom(index);

        if (after_inserted.IsFinite()) {
            return source_->Get(
                insert_index + after_inserted.GetFiniteIndex()
            );
        }

        return source_->Get(after_inserted);
    }

    if (index.IsFinite()) {
        return source_->Get(index.GetFiniteIndex());
    }

    int insert_infinity = index_.GetInfinityCount();
    int insert_offset = index_.GetFiniteIndex();

    int query_infinity = index.GetInfinityCount();
    int query_offset = index.GetFiniteIndex();

    if (query_infinity < insert_infinity) {
        return source_->Get(index);
    }

    if (query_infinity == insert_infinity && query_offset < insert_offset) {
        return source_->Get(index);
    }

    TransfiniteIndex inserted_index =
        query_infinity == insert_infinity
        ? TransfiniteIndex::Finite(query_offset - insert_offset)
        : TransfiniteIndex(
            query_infinity - insert_infinity,
            query_offset
        );

    if (inserted_transfinite_length_.Contains(inserted_index)) {
        if (inserted_index.IsFinite()) {
            return inserted_->Get(inserted_index.GetFiniteIndex());
        }

        return inserted_->Get(inserted_index);
    }

    TransfiniteIndex after_inserted =
        inserted_transfinite_length_.SubtractFrom(inserted_index);

    if (after_inserted.IsFinite()) {
        return source_->Get(
            TransfiniteIndex(
                insert_infinity,
                insert_offset + after_inserted.GetFiniteIndex()
            )
        );
    }

    return source_->Get(
        TransfiniteIndex(
            insert_infinity + after_inserted.GetInfinityCount(),
            after_inserted.GetFiniteIndex()
        )
    );
}


template <class T>
Cardinal InsertSequenceGenerator<T>::GetResultCardinality() const {
    return result_length_;
}

template <class T>
TransfiniteLength InsertSequenceGenerator<T>::GetResultLength() const {
    if (index_.IsFinite()) {
        int insert_index = index_.GetFiniteIndex();

        if (source_transfinite_length_.IsFinite()) {
            if (inserted_transfinite_length_.IsFinite()) {
                return TransfiniteLength::Finite(
                    source_transfinite_length_.GetFiniteCount()
                    + inserted_transfinite_length_.GetFiniteCount()
                );
            }

            int tail_count =
                source_transfinite_length_.GetFiniteCount() - insert_index;

            return TransfiniteLength::Add(
                inserted_transfinite_length_,
                TransfiniteLength::Finite(tail_count)
            );
        }

        if (inserted_transfinite_length_.IsFinite()) {
            return source_transfinite_length_;
        }

        return TransfiniteLength::Add(
            inserted_transfinite_length_,
            source_transfinite_length_
        );
    }

    return TransfiniteLength::Add(
        source_transfinite_length_,
        inserted_transfinite_length_
    );
}