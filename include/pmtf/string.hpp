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
#include <pmtf/wrap.hpp>
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
    using value_type = char;
    using reference = char&;
    using const_reference = const char&;
    using size_type = size_t;
    string(const std::string& value) {
        _MakeString(value.data(), value.size());
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
    
    static constexpr Data data_type() { return DataTraits<type>::enum_value; }
    void print(std::ostream& os) const { os << value(); }
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

// When we switch to c++20, make this a concept.
/*template <class U>
bool operator==(const string& x, const U& other) {
    if (other.size() != x.size()) return false;
    auto my_val = x.begin();
    for (auto&& val : other) {
        if (*my_val != val) return false;
        my_val++;
    }
    return true;
}*/

inline bool operator==(const string& x, const char other[]) {
    size_t index = 0;
    while (other[index] != 0 && index < x.size()) {
        if (other[index] != x[index]) return false;
        index++;
    }
    return index == x.size() || x[index] == 0;
}



inline std::ostream& operator<<(std::ostream& os, const string& value) {
    os << value.value();
    return os;
}

/*string get_string(const wrap& x);*/
inline string get_string(const pmt& p) {
    if (p.data_type() == string::data_type())
        return string(p);
    // This error message stinks.  Fix it.
    throw std::runtime_error("Can't convert pmt to this string");
}


} // namespace pmtf
