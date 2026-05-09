#pragma once

#include "lib/Sequence.h"

#include <functional>
#include <stdexcept>


template <class T>
class Generator {
private:
    std::function<T(const Sequence<T>*)> rule_;
    const Sequence<T>* source_;
    int position_;
    bool has_next_;

public:
    Generator();

    Generator(
        std::function<T(const Sequence<T>*)> rule,
        const Sequence<T>* source
    );

    Generator(const Generator<T>& other);

    Generator<T>& operator=(const Generator<T>& other);

    bool HasNext() const;
    T GetNext();

    int GetPosition() const;

    void SetSource(const Sequence<T>* source);
    void Reset();
};

#include "Generator.tpp"