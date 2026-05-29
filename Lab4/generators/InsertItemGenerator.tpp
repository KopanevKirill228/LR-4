#pragma once


static Cardinal InsertItemResultCardinal(const Cardinal& source) {
    if (source.IsInfinite()) {
        return Cardinal::Infinity();
    }

    return Cardinal::Finite(source.GetFiniteValue() + 1);
}


template <class T>
InsertItemGenerator<T>::InsertItemGenerator(
    const LazySequence<T>& source,
    const T& item,
    int index
)
    : source_(new LazySequence<T>(source)),
    item_(new T(item)),
    source_length_(source.GetCardinality()),
    result_length_(InsertItemResultCardinal(source.GetCardinality())),
    index_(index),
    position_(0)
{
    if (index_ < 0) {
        delete source_;
        delete item_;

        source_ = nullptr;
        item_ = nullptr;

        throw std::out_of_range("InsertItemGenerator: index is negative");
    }

    if (source_length_.IsFinite() && index_ > source_length_.GetFiniteValue()) {
        delete source_;
        delete item_;

        source_ = nullptr;
        item_ = nullptr;

        throw std::out_of_range("InsertItemGenerator: index is out of range");
    }
}


template <class T>
InsertItemGenerator<T>::InsertItemGenerator(const InsertItemGenerator<T>& other)
    : source_(other.source_ == nullptr ? nullptr : new LazySequence<T>(*other.source_)),
    item_(other.item_ == nullptr ? nullptr : new T(*other.item_)),
    source_length_(other.source_length_),
    result_length_(other.result_length_),
    index_(other.index_),
    position_(other.position_) {
}


template <class T>
InsertItemGenerator<T>& InsertItemGenerator<T>::operator=(
    const InsertItemGenerator<T>& other
    ) {
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
    index_ = other.index_;
    position_ = other.position_;

    return *this;
}


template <class T>
InsertItemGenerator<T>::~InsertItemGenerator() {
    delete source_;
    delete item_;
}


template <class T>
bool InsertItemGenerator<T>::HasNext() const {
    if (result_length_.IsInfinite()) {
        return true;
    }

    return position_ < result_length_.GetFiniteValue();
}


template <class T>
T InsertItemGenerator<T>::GetNext() {
    if (!HasNext()) {
        throw std::out_of_range("InsertItemGenerator: no next element");
    }

    T value;

    if (position_ < index_) {
        value = source_->Get(position_);
    }
    else if (position_ == index_) {
        value = *item_;
    }
    else {
        value = source_->Get(position_ - 1);
    }

    ++position_;

    return value;
}


template <class T>
int InsertItemGenerator<T>::GetPosition() const {
    return position_;
}


template <class T>
void InsertItemGenerator<T>::SetSource(const Sequence<T>&) {
}


template <class T>
void InsertItemGenerator<T>::Reset() {
    position_ = 0;
}


template <class T>
Generator<T>* InsertItemGenerator<T>::Clone() const {
    return new InsertItemGenerator<T>(*this);
}


template <class T>
T InsertItemGenerator<T>::GetAfterInfinite(int index) const {
    if (index < 0) {
        throw std::out_of_range("InsertItemGenerator: after-infinity index is negative");
    }

    return GetByTransfiniteIndex(TransfiniteIndex::AfterInfinity(index));
}


template <class T>
T InsertItemGenerator<T>::GetByTransfiniteIndex(const TransfiniteIndex& index) const {
    if (index.IsFinite()) {
        int finite_index = index.GetFiniteIndex();

        if (finite_index < index_) {
            return source_->Get(finite_index);
        }

        if (finite_index == index_) {
            return *item_;
        }

        return source_->Get(finite_index - 1);
    }

    return source_->Get(index);
}


template <class T>
Cardinal InsertItemGenerator<T>::GetResultCardinality() const {
    return result_length_;
}

template <class T>
TransfiniteLength InsertItemGenerator<T>::GetResultLength() const {
    TransfiniteLength source_length = source_->GetTransfiniteLength();

    if (source_length.IsInfinite()) {
        return source_length;
    }

    return TransfiniteLength::Finite(source_length.GetFiniteCount() + 1);
}