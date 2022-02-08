/*-*-c++-*-*/
/*
 * Copyright 2021 John Sallay
 *
 * SPDX-License-Identifier: LGPL-3.0
 *
 */
#pragma once

#include <pmtf/pmtf_generated.h>
#include <pmtf/base.hpp>
#include <complex>
#include <iostream>
#include <map>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <pmtf/gsl-lite.hpp>

namespace pmtf {

class string {
public:
    using traits = PmtString::Traits;
    using type = typename traits::type;
    using span = typename gsl::span<char>;
    using value_type = char;
    using reference = char&;
    using const_reference = const char&;
    using size_type = size_t;

    using iterator = typename span::iterator;
    using const_iterator = typename span::const_iterator;

    string(const std::string& value) {
        _MakeString(value.data(), value.size());
    }
    string(const char* value) {
        _MakeString(value, strlen(value));
    }
    template <class T, typename = IsPmt<T>>
    string(const T& other): _buf(other) {} 
    ~string() {}
    gsl::span<char> value() {
        std::shared_ptr<base_buffer> scalar = _get_buf();
        auto buf = scalar->data_as<type>()->value();
        return gsl::span<char>(const_cast<char*>(buf->data()), buf->size());
    }
    std::string_view value() const {
        std::shared_ptr<base_buffer> scalar = _get_buf();
        auto buf = scalar->data_as<type>()->value();
        return std::string_view(buf->data(), buf->size());
    }
    string& operator=(const std::string& value) {
        _MakeString(value.data(), value.size());
        return *this;
    }
    string& operator=(const char value[]) {
        _MakeString(&value[0], std::string(value).size());
        return *this;
    }
    char* data() { return value().data(); }
    const char* data() const { return value().data(); }
    size_t size() const { return value().size(); }
    char& operator[] (size_type n) {
        // operator[] doesn't do bounds checking, use at for that
        // TODO: implement at
        return data()[n];
    }
    const char& operator[] (size_type n) const {
        return data()[n];
    }
    
    typename span::iterator begin() { return value().begin(); }
    typename span::const_iterator begin() const { return value().begin(); }
    typename span::iterator end() { return value().end(); }
    typename span::const_iterator end() const { return value().end(); }
    static constexpr Data data_type() { return DataTraits<type>::enum_value; }
    void print(std::ostream& os) const { os << value(); }
    const pmt& get_pmt_buffer() const { return _buf; }
    //! Equality Comparisons
    // Declared as class members so that we don't do implicit conversions.
    template <class U>
    bool operator==(const U& x) const;
    template <class U>
    bool operator!=(const U& x) const { return !(operator==(x));}
private:
    pmt _buf;
    std::shared_ptr<base_buffer>& _get_buf() { return _buf._scalar; }
    const std::shared_ptr<base_buffer> _get_buf() const { return _buf._scalar; }
    void _MakeString(const char* data, size_t size) {
        flatbuffers::FlatBufferBuilder fbb;
        auto offset = traits::Create(fbb, fbb.CreateString(data, size)).Union();
        auto pmt = CreatePmt(fbb, data_type(), offset);
        fbb.FinishSizePrefixed(pmt);
        _get_buf() = std::make_shared<base_buffer>(fbb.Release());
    }
    
};

template <> inline pmt::pmt<string>(const string& x) { *this = x.get_pmt_buffer(); }
template <> inline pmt::pmt<std::string>(const std::string& x) { *this = string(x).get_pmt_buffer(); }
template <> inline pmt::pmt<char>(const char* x) { *this = string(x).get_pmt_buffer(); }

template <typename T>
using IsPmtString = std::enable_if_t<std::is_same_v<T, string>, bool>;
template <class T>
using IsString = std::enable_if_t<std::is_same_v<string, T>, bool>;
template <class T>
using IsNotString = std::enable_if_t<!std::is_same_v<string, T>, bool>;

template <class T>
bool string::operator==(const T& other) const {
    if constexpr(std::is_same_v<std::decay_t<T>, char*>) {
        size_t index = 0;
        while (other[index] != 0 && index < size()) {
            if (other[index] != (*this)[index]) return false;
            index++;
        }
        return index == size() || (*this)[index] == 0;
    } else if constexpr(is_container<T>::value) {
        if constexpr( std::is_same_v<typename T::value_type, char>) {
            if (other.size() != size()) return false;
            return std::equal(std::begin(*this), std::end(*this), other.begin());
        } else {
            return false;
        }
    } else if constexpr(std::is_same_v<T, pmt>) {
        return other.operator==(*this);
    } else
        return false;
}

// Reversed case.  This allows for x == y and y == x
template <class T, class U, IsString<T> = true, IsNotString<U> = true>
bool operator==(const U& y, const T& x) {
    return x.operator==(y);
}

// Reversed Not equal operator
template <class T, class U, IsString<T> = true, IsNotString<U> = true>
bool operator!=(const U& y, const T& x) {
    return operator!=(x,y);
}

template <class T, IsString<T> = true>
inline std::ostream& operator<<(std::ostream& os, const T& value) {
    os << value.value();
    return os;
}

inline string get_string(const pmt& p) {
    if (p.data_type() == string::data_type())
        return string(p);
    throw ConversionError(p, "string");
}



} // namespace pmtf
