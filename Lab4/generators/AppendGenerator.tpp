#pragma once


static Cardinal AppendResultCardinal(const Cardinal& source) {
    if (source.IsInfinite()) {
        return Cardinal::Infinity();
    }

    return Cardinal::Finite(source.GetFiniteValue() + 1);
}


template <class T>
AppendGenerator<T>::AppendGenerator(
    const LazySequence<T>& source,
    const T& item
)
    : source_(new LazySequence<T>(source)),
    item_(new T(item)),
    source_length_(source.GetCardinality()),
    result_length_(AppendResultCardinal(source.GetCardinality())),
    source_transfinite_length_(source.GetTransfiniteLength()),
    result_transfinite_length_(
        TransfiniteLength::Add(
            source.GetTransfiniteLength(),
            TransfiniteLength::Finite(1)
        )
    ),
    position_(0) {
}


template <class T>
AppendGenerator<T>::AppendGenerator(const AppendGenerator<T>& other)
    : source_(other.source_ == nullptr ? nullptr : new LazySequence<T>(*other.source_)),
    item_(other.item_ == nullptr ? nullptr : new T(*other.item_)),
    source_length_(other.source_length_),
    result_length_(other.result_length_),
    source_transfinite_length_(other.source_transfinite_length_),
    result_transfinite_length_(other.result_transfinite_length_),
    position_(other.position_) {
}

template <class T>
AppendGenerator<T>& AppendGenerator<T>::operator=(const AppendGenerator<T>& other) {
    if (this == &other) {
        return *this;
    }

    LazySequence<T>* new_source = other.source_ == nullptr
        ? nullptr
        : new LazySequence<T>(*other.source_);

    T* new_item = other.item_ == nullptr
        ? nullptr
        : new T(*other.item_);

    delete source_;
    delete item_;

    source_ = new_source;
    item_ = new_item;

    source_length_ = other.source_length_;
    result_length_ = other.result_length_;
    position_ = other.position_;
    source_transfinite_length_ = other.source_transfinite_length_;
    result_transfinite_length_ = other.result_transfinite_length_;

    return *this;
}


template <class T>
AppendGenerator<T>::~AppendGenerator() {
    delete source_;
    delete item_;
}


template <class T>
bool AppendGenerator<T>::HasNext() const {
    if (result_length_.IsInfinite()) {
        return true;
    }

    return position_ < result_length_.GetFiniteValue();
}


template <class T>
T AppendGenerator<T>::GetNext() {
    if (!HasNext()) {
        throw std::out_of_range("AppendGenerator: no next element");
    }

    T value;

    if (source_length_.IsInfinite()) {
        value = source_->Get(position_);
    }
    else {
        int source_length = source_length_.GetFiniteValue();

        if (position_ < source_length) {
            value = source_->Get(position_);
        }
        else {
            value = *item_;
        }
    }

    ++position_;

    return value;
}


template <class T>
int AppendGenerator<T>::GetPosition() const {
    return position_;
}


template <class T>
void AppendGenerator<T>::SetSource(const Sequence<T>&) {
}


template <class T>
void AppendGenerator<T>::Reset() {
    position_ = 0;
}


template <class T>
Generator<T>* AppendGenerator<T>::Clone() const {
    return new AppendGenerator<T>(*this);
}


template <class T>
T AppendGenerator<T>::GetAfterInfinite(int index) const {
    if (index < 0) {
        throw std::out_of_range("AppendGenerator: after-infinity index is negative");
    }

    return GetByTransfiniteIndex(TransfiniteIndex::AfterInfinity(index));
}


template <class T>
T AppendGenerator<T>::GetByTransfiniteIndex(const TransfiniteIndex& index) const {
    if (source_transfinite_length_.Contains(index)) {
        if (index.IsFinite()) {
            return source_->Get(index.GetFiniteIndex());
        }

        return source_->Get(index);
    }

    TransfiniteIndex appended_index =
        source_transfinite_length_.SubtractFrom(index);

    if (appended_index.IsFinite() && appended_index.GetFiniteIndex() == 0) {
        return *item_;
    }

    throw std::out_of_range("AppendGenerator: transfinite index is out of range");
}


template <class T>
Cardinal AppendGenerator<T>::GetResultCardinality() const {
    return result_length_;
}

template <class T>
TransfiniteLength AppendGenerator<T>::GetResultLength() const {
    return TransfiniteLength::Add(
        source_->GetTransfiniteLength(),
        TransfiniteLength::Finite(1)
    );
}