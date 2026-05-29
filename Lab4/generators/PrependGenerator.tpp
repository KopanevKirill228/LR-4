#pragma once


static Cardinal PrependResultCardinal(const Cardinal& source) {
    if (source.IsInfinite()) {
        return Cardinal::Infinity();
    }

    return Cardinal::Finite(source.GetFiniteValue() + 1);
}


template <class T>
PrependGenerator<T>::PrependGenerator(
    const LazySequence<T>& source,
    const T& item
)
    : source_(new LazySequence<T>(source)),
    item_(new T(item)),
    source_length_(source.GetCardinality()),
    result_length_(PrependResultCardinal(source.GetCardinality())),
    position_(0) {
}


template <class T>
PrependGenerator<T>::PrependGenerator(const PrependGenerator<T>& other)
    : source_(other.source_ == nullptr ? nullptr : new LazySequence<T>(*other.source_)),
    item_(other.item_ == nullptr ? nullptr : new T(*other.item_)),
    source_length_(other.source_length_),
    result_length_(other.result_length_),
    position_(other.position_) {
}


template <class T>
PrependGenerator<T>& PrependGenerator<T>::operator=(const PrependGenerator<T>& other) {
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

    return *this;
}


template <class T>
PrependGenerator<T>::~PrependGenerator() {
    delete source_;
    delete item_;
}


template <class T>
bool PrependGenerator<T>::HasNext() const {
    if (result_length_.IsInfinite()) {
        return true;
    }

    return position_ < result_length_.GetFiniteValue();
}


template <class T>
T PrependGenerator<T>::GetNext() {
    if (!HasNext()) {
        throw std::out_of_range("PrependGenerator: no next element");
    }

    T value;

    if (position_ == 0) {
        value = *item_;
    }
    else {
        value = source_->Get(position_ - 1);
    }

    ++position_;

    return value;
}


template <class T>
int PrependGenerator<T>::GetPosition() const {
    return position_;
}


template <class T>
void PrependGenerator<T>::SetSource(const Sequence<T>&) {
}


template <class T>
void PrependGenerator<T>::Reset() {
    position_ = 0;
}


template <class T>
Generator<T>* PrependGenerator<T>::Clone() const {
    return new PrependGenerator<T>(*this);
}


template <class T>
T PrependGenerator<T>::GetAfterInfinite(int index) const {
    if (index < 0) {
        throw std::out_of_range("PrependGenerator: after-infinity index is negative");
    }

    return GetByTransfiniteIndex(TransfiniteIndex::AfterInfinity(index));
}


template <class T>
T PrependGenerator<T>::GetByTransfiniteIndex(const TransfiniteIndex& index) const {
    if (index.IsFinite()) {
        int finite_index = index.GetFiniteIndex();

        if (finite_index == 0) {
            return *item_;
        }

        return source_->Get(finite_index - 1);
    }

    return source_->Get(index);
}

template <class T>
Cardinal PrependGenerator<T>::GetResultCardinality() const {
    return result_length_;
}

template <class T>
TransfiniteLength PrependGenerator<T>::GetResultLength() const {
    TransfiniteLength source_length = source_->GetTransfiniteLength();

    if (source_length.IsInfinite()) {
        return source_length;
    }

    return TransfiniteLength::Finite(source_length.GetFiniteCount() + 1);
}