#pragma once

#include "Sequence.h"
#include "Pair.h"

#include <stdexcept>
#include <functional>


// Добавляет элемент с учетом mutable / immutable семантики
template <class T>
void AppendToResult(Sequence<T>*& seq, const T& item) {
    Sequence<T>* old = seq;
    Sequence<T>* next = seq->Append(item);

    if (next == nullptr) {
        throw std::runtime_error("Append returned nullptr");
    }

    if (next != old) {
        delete old;
        seq = next;
    }
}


// MAP — применяет func к каждому элементу
template <class T, class U>
Sequence<U>* Map(
    const Sequence<T>* seq,
    std::function<U(const T&)> func,
    std::function<Sequence<U>* ()> createResult)
{
    if (seq == nullptr) {
        throw std::invalid_argument("Map: sequence is nullptr");
    }

    if (!func) {
        throw std::invalid_argument("Map: function is empty");
    }

    if (!createResult) {
        throw std::invalid_argument("Map: result factory is empty");
    }

    Sequence<U>* result = createResult();

    if (result == nullptr) {
        throw std::runtime_error("Map: result factory returned nullptr");
    }

    IEnumerator<T>* en = nullptr;

    try {
        en = seq->GetEnumerator();

        if (en == nullptr) {
            throw std::runtime_error("Map: enumerator is nullptr");
        }

        while (en->MoveNext()) {
            AppendToResult(result, func(en->GetCurrent()));
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


// WHERE — оставляет только элементы где func = true
template <class T>
Sequence<T>* Where(
    const Sequence<T>* seq,
    std::function<bool(const T&)> func,
    std::function<Sequence<T>* ()> createResult)
{
    if (seq == nullptr) {
        throw std::invalid_argument("Where: sequence is nullptr");
    }

    if (!func) {
        throw std::invalid_argument("Where: function is empty");
    }

    if (!createResult) {
        throw std::invalid_argument("Where: result factory is empty");
    }

    Sequence<T>* result = createResult();

    if (result == nullptr) {
        throw std::runtime_error("Where: result factory returned nullptr");
    }

    IEnumerator<T>* en = nullptr;

    try {
        en = seq->GetEnumerator();

        if (en == nullptr) {
            throw std::runtime_error("Where: enumerator is nullptr");
        }

        while (en->MoveNext()) {
            const T& val = en->GetCurrent();

            if (func(val)) {
                AppendToResult(result, val);
            }
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


// REDUCE — сворачивает последовательность в одно значение
template <class T, class U>
U Reduce(
    const Sequence<T>* seq,
    std::function<U(const U&, const T&)> func,
    const U& initial)
{
    if (seq == nullptr) {
        throw std::invalid_argument("Reduce: sequence is nullptr");
    }

    if (!func) {
        throw std::invalid_argument("Reduce: function is empty");
    }

    U accum = initial;
    IEnumerator<T>* en = nullptr;

    try {
        en = seq->GetEnumerator();

        if (en == nullptr) {
            throw std::runtime_error("Reduce: enumerator is nullptr");
        }

        while (en->MoveNext()) {
            accum = func(accum, en->GetCurrent());
        }

        delete en;
        return accum;
    }
    catch (...) {
        delete en;
        throw;
    }
}


// ZIP — объединяет две последовательности в одну попарно
template <class T, class U>
Sequence<Pair<T, U>>* Zip(
    const Sequence<T>* first,
    const Sequence<U>* second,
    std::function<Sequence<Pair<T, U>>* ()> createResult)
{
    if (first == nullptr) {
        throw std::invalid_argument("Zip: first sequence is nullptr");
    }

    if (second == nullptr) {
        throw std::invalid_argument("Zip: second sequence is nullptr");
    }

    if (!createResult) {
        throw std::invalid_argument("Zip: result factory is empty");
    }

    Sequence<Pair<T, U>>* result = createResult();

    if (result == nullptr) {
        throw std::runtime_error("Zip: result factory returned nullptr");
    }

    IEnumerator<T>* en1 = nullptr;
    IEnumerator<U>* en2 = nullptr;

    try {
        en1 = first->GetEnumerator();
        en2 = second->GetEnumerator();

        if (en1 == nullptr || en2 == nullptr) {
            throw std::runtime_error("Zip: enumerator is nullptr");
        }

        while (en1->MoveNext() && en2->MoveNext()) {
            AppendToResult(result, Pair<T, U>(en1->GetCurrent(), en2->GetCurrent()));
        }

        delete en1;
        delete en2;

        return result;
    }
    catch (...) {
        delete en1;
        delete en2;
        delete result;
        throw;
    }
}


// UNZIP — разбивает последовательность пар на две
template <class T, class U>
void Unzip(
    const Sequence<Pair<T, U>>* seq,
    Sequence<T>*& outFirst,
    Sequence<U>*& outSecond,
    std::function<Sequence<T>* ()> createFirst,
    std::function<Sequence<U>* ()> createSecond)
{
    if (seq == nullptr) {
        throw std::invalid_argument("Unzip: sequence is nullptr");
    }

    if (!createFirst) {
        throw std::invalid_argument("Unzip: first result factory is empty");
    }

    if (!createSecond) {
        throw std::invalid_argument("Unzip: second result factory is empty");
    }

    Sequence<T>* first = createFirst();
    Sequence<U>* second = nullptr;
    IEnumerator<Pair<T, U>>* en = nullptr;

    try {
        if (first == nullptr) {
            throw std::runtime_error("Unzip: first result factory returned nullptr");
        }

        second = createSecond();

        if (second == nullptr) {
            throw std::runtime_error("Unzip: second result factory returned nullptr");
        }

        en = seq->GetEnumerator();

        if (en == nullptr) {
            throw std::runtime_error("Unzip: enumerator is nullptr");
        }

        while (en->MoveNext()) {
            const Pair<T, U>& p = en->GetCurrent();

            AppendToResult(first, p.first);
            AppendToResult(second, p.second);
        }

        delete en;

        outFirst = first;
        outSecond = second;
    }
    catch (...) {
        delete en;
        delete first;
        delete second;

        outFirst = nullptr;
        outSecond = nullptr;

        throw;
    }
}