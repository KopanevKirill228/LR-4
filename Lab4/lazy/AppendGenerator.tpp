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
    position_(0) {
}


template <class T>
AppendGenerator<T>::AppendGenerator(const AppendGenerator<T>& other)
    : source_(other.source_ == nullptr ? nullptr : new LazySequence<T>(*other.source_)),
    item_(other.item_ == nullptr ? nullptr : new T(*other.item_)),
    source_length_(other.source_length_),
    result_length_(other.result_length_),
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
T AppendGenerator<T>::GetByTransfiniteIndex(const TransfiniteIndex& index) const {
    if (index.IsFinite()) {
        int finite_index = index.GetFiniteIndex();

        if (source_length_.IsInfinite()) {
            return source_->Get(finite_index);
        }

        int source_length = source_length_.GetFiniteValue();

        if (finite_index < source_length) {
            return source_->Get(finite_index);
        }

        if (finite_index == source_length) {
            return *item_;
        }

        throw std::out_of_range("AppendGenerator: index is out of range");
    }

    if (!source_length_.IsInfinite()) {
        throw std::out_of_range("AppendGenerator: no transfinite tail");
    }

    int tail_index = index.GetFiniteIndex();

    if (index.GetInfinityCount() == 1) {
        int used_by_source = 0;

        while (used_by_source <= tail_index) {
            try {
                T value = source_->Get(
                    TransfiniteIndex::AfterInfinity(used_by_source)
                );

                if (used_by_source == tail_index) {
                    return value;
                }

                ++used_by_source;
            }
            catch (...) {
                if (tail_index == used_by_source) {
                    return *item_;
                }

                throw std::out_of_range("AppendGenerator: transfinite index is out of range");
            }
        }
    }

    try {
        return source_->Get(index);
    }
    catch (...) {
        throw std::out_of_range("AppendGenerator: transfinite index is out of range");
    }
}


template <class T>
Cardinal AppendGenerator<T>::GetResultCardinality() const {
    return result_length_;
}