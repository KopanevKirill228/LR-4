#pragma once

#include "LazySequence.h"


template <class T>
LazySequence<T>::LazySequence()
    : materialized_(),
    generator_(nullptr),
    is_infinite_(false),
    finite_length_(0) {
}


template <class T>
LazySequence<T>::LazySequence(const T* items, int count)
    : materialized_(),
    generator_(nullptr),
    is_infinite_(false),
    finite_length_(0)
{
    if (count < 0) {
        throw std::invalid_argument("LazySequence: count must be non-negative");
    }

    if (items == nullptr && count > 0) {
        throw std::invalid_argument("LazySequence: items is nullptr");
    }

    try {
        for (int i = 0; i < count; ++i) {
            AppendMaterialized(items[i]);
        }

        finite_length_ = count;
    }
    catch (...) {
        delete generator_;
        generator_ = nullptr;
        finite_length_ = 0;
        throw;
    }
}


template <class T>
LazySequence<T>::LazySequence(const Sequence<T>* seq)
    : materialized_(),
    generator_(nullptr),
    is_infinite_(false),
    finite_length_(0)
{
    if (seq == nullptr) {
        throw std::invalid_argument("LazySequence: sequence is nullptr");
    }

    IEnumerator<T>* en = nullptr;

    try {
        en = seq->GetEnumerator();

        if (en == nullptr) {
            throw std::runtime_error("LazySequence: enumerator is nullptr");
        }

        while (en->MoveNext()) {
            AppendMaterialized(en->GetCurrent());
            ++finite_length_;
        }

        delete en;
    }
    catch (...) {
        delete en;
        finite_length_ = 0;
        throw;
    }
}


template <class T>
LazySequence<T>::LazySequence(
    std::function<T(const Sequence<T>*)> rule,
    const Sequence<T>* initialValues)
    : materialized_(),
    generator_(nullptr),
    is_infinite_(true),
    finite_length_(-1)
{
    if (!rule) {
        throw std::invalid_argument("LazySequence: rule is empty");
    }

    if (initialValues == nullptr) {
        throw std::invalid_argument("LazySequence: initial values is nullptr");
    }

    IEnumerator<T>* en = nullptr;

    try {
        en = initialValues->GetEnumerator();

        if (en == nullptr) {
            throw std::runtime_error("LazySequence: enumerator is nullptr");
        }

        while (en->MoveNext()) {
            AppendMaterialized(en->GetCurrent());
        }

        delete en;
        en = nullptr;

        if (materialized_.GetLength() == 0) {
            throw std::invalid_argument("LazySequence: initial values must not be empty");
        }

        generator_ = new Generator<T>(rule, &materialized_);
    }
    catch (...) {
        delete en;
        delete generator_;
        generator_ = nullptr;
        throw;
    }
}


template <class T>
LazySequence<T>::LazySequence(const LazySequence<T>& other)
    : materialized_(other.materialized_),
    generator_(nullptr),
    is_infinite_(other.is_infinite_),
    finite_length_(other.finite_length_)
{
    try {
        if (other.generator_ != nullptr) {
            generator_ = new Generator<T>(*other.generator_);
            generator_->SetSource(&materialized_);
        }
    }
    catch (...) {
        delete generator_;
        generator_ = nullptr;
        throw;
    }
}


template <class T>
LazySequence<T>& LazySequence<T>::operator=(const LazySequence<T>& other) {
    if (this == &other) {
        return *this;
    }

    MutableArraySequence<T> newMaterialized(other.materialized_);
    Generator<T>* newGenerator = nullptr;

    try {
        if (other.generator_ != nullptr) {
            newGenerator = new Generator<T>(*other.generator_);
        }

        materialized_ = newMaterialized;

        if (newGenerator != nullptr) {
            newGenerator->SetSource(&materialized_);
        }

        delete generator_;

        generator_ = newGenerator;
        is_infinite_ = other.is_infinite_;
        finite_length_ = other.finite_length_;

        return *this;
    }
    catch (...) {
        delete newGenerator;
        throw;
    }
}


template <class T>
LazySequence<T>::~LazySequence() {
    delete generator_;
}


template <class T>
void LazySequence<T>::AppendMaterialized(const T& item) const {
    Sequence<T>* result = materialized_.Append(item);

    if (result == nullptr) {
        throw std::runtime_error("LazySequence: append returned nullptr");
    }

    if (result != &materialized_) {
        delete result;
        throw std::runtime_error("LazySequence: materialized storage must be mutable");
    }
}


template <class T>
void LazySequence<T>::CheckIndex(int index) const {
    if (index < 0) {
        throw std::out_of_range("LazySequence: index is negative");
    }

    if (!is_infinite_ && index >= finite_length_) {
        throw std::out_of_range("LazySequence: index is out of range");
    }
}


template <class T>
void LazySequence<T>::MaterializeTo(int index) const {
    CheckIndex(index);

    if (index < materialized_.GetLength()) {
        return;
    }

    if (generator_ == nullptr) {
        throw std::out_of_range("LazySequence: cannot materialize element");
    }

    while (materialized_.GetLength() <= index) {
        if (!generator_->HasNext()) {
            throw std::out_of_range("LazySequence: generator has no next element");
        }

        T next = generator_->GetNext();
        AppendMaterialized(next);
    }
}


template <class T>
const T& LazySequence<T>::GetFirst() const {
    if (!is_infinite_ && finite_length_ == 0) {
        throw std::out_of_range("LazySequence is empty");
    }

    return Get(0);
}


template <class T>
const T& LazySequence<T>::GetLast() const {
    if (is_infinite_) {
        throw std::logic_error("LazySequence: infinite sequence has no last element");
    }

    if (finite_length_ == 0) {
        throw std::out_of_range("LazySequence is empty");
    }

    return Get(finite_length_ - 1);
}


template <class T>
const T& LazySequence<T>::Get(int index) const {
    MaterializeTo(index);
    return materialized_.Get(index);
}


template <class T>
int LazySequence<T>::GetLength() const {
    if (is_infinite_) {
        throw std::logic_error("LazySequence: infinite sequence has no finite length");
    }

    return finite_length_;
}


template <class T>
int LazySequence<T>::GetMaterializedCount() const {
    return materialized_.GetLength();
}


template <class T>
bool LazySequence<T>::IsInfinite() const {
    return is_infinite_;
}


template <class T>
Sequence<T>* LazySequence<T>::GetSubsequence(int startIndex, int endIndex) const {
    if (startIndex < 0 || endIndex < 0) {
        throw std::out_of_range("LazySequence: subsequence index is negative");
    }

    if (endIndex < startIndex) {
        throw std::out_of_range("LazySequence: end index is less than start index");
    }

    if (!is_infinite_ && endIndex >= finite_length_) {
        throw std::out_of_range("LazySequence: subsequence index is out of range");
    }

    LazySequence<T>* result = new LazySequence<T>();

    try {
        for (int i = startIndex; i <= endIndex; ++i) {
            result->AppendMaterialized(Get(i));
            ++result->finite_length_;
        }

        return result;
    }
    catch (...) {
        delete result;
        throw;
    }
}


template <class T>
Sequence<T>* LazySequence<T>::Append(const T& item) {
    if (is_infinite_) {
        throw std::logic_error("LazySequence: cannot append to infinite sequence in this implementation");
    }

    LazySequence<T>* result = new LazySequence<T>(*this);

    try {
        result->AppendMaterialized(item);
        ++result->finite_length_;

        return result;
    }
    catch (...) {
        delete result;
        throw;
    }
}


template <class T>
Sequence<T>* LazySequence<T>::Prepend(const T& item) {
    if (is_infinite_) {
        throw std::logic_error("LazySequence: cannot prepend to infinite sequence in this implementation");
    }

    LazySequence<T>* result = new LazySequence<T>();

    try {
        result->AppendMaterialized(item);
        ++result->finite_length_;

        for (int i = 0; i < finite_length_; ++i) {
            result->AppendMaterialized(Get(i));
            ++result->finite_length_;
        }

        return result;
    }
    catch (...) {
        delete result;
        throw;
    }
}


template <class T>
Sequence<T>* LazySequence<T>::InsertAt(const T& item, int index) {
    if (is_infinite_) {
        throw std::logic_error("LazySequence: cannot insert into infinite sequence in this implementation");
    }

    if (index < 0 || index > finite_length_) {
        throw std::out_of_range("LazySequence: insert index is out of range");
    }

    LazySequence<T>* result = new LazySequence<T>();

    try {
        for (int i = 0; i < index; ++i) {
            result->AppendMaterialized(Get(i));
            ++result->finite_length_;
        }

        result->AppendMaterialized(item);
        ++result->finite_length_;

        for (int i = index; i < finite_length_; ++i) {
            result->AppendMaterialized(Get(i));
            ++result->finite_length_;
        }

        return result;
    }
    catch (...) {
        delete result;
        throw;
    }
}


template <class T>
Sequence<T>* LazySequence<T>::Concat(const Sequence<T>& other) const {
    if (is_infinite_) {
        throw std::logic_error("LazySequence: cannot concat after infinite sequence in this implementation");
    }

    LazySequence<T>* result = new LazySequence<T>(*this);
    IEnumerator<T>* en = nullptr;

    try {
        en = other.GetEnumerator();

        if (en == nullptr) {
            throw std::runtime_error("LazySequence: enumerator is nullptr");
        }

        while (en->MoveNext()) {
            result->AppendMaterialized(en->GetCurrent());
            ++result->finite_length_;
        }

        delete en;
        return result;
    }
    catch (...) {
        delete en;
        delete result;
        throw;
    }
}


template <class T>
T LazySequence<T>::operator[](int index) const {
    return Get(index);
}


template <class T>
Sequence<T>* LazySequence<T>::operator+(const Sequence<T>& other) const {
    return Concat(other);
}


template <class T>
IEnumerator<T>* LazySequence<T>::GetEnumerator() const {
    return new Enumerator(this);
}