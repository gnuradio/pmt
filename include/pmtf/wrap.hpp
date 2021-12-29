/*-*-c++-*-*/
/*
 * Copyright 2021 John Sallay
 *
 * SPDX-License-Identifier: LGPL-3.0
 *
 */
#pragma once

#include <pmtf/base.hpp>
#include <pmtf/vector.hpp>
#include <pmtf/scalar.hpp>
#include <pmtf/map.hpp>
#include <pmtf/string.hpp>
#include <variant>

namespace pmtf {

// Forward declare so that we can use it in the function.
inline std::ostream& operator<<(std::ostream& os, const map& value);
bool operator==(const pmt& x, const map& y);
inline bool operator==(const map& y, const pmt& x) { return operator==(x,y); }
std::ostream& operator<<(std::ostream& os, const pmt& value);

inline bool operator==(const pmt& value, const pmt& other) {
    switch (value.data_type()) {
        case Data::ScalarFloat32: return operator==(scalar<float>(value), other);
        case Data::ScalarFloat64: return operator==(scalar<double>(value), other);
        case Data::ScalarComplex64: return operator==(scalar<std::complex<float>>(value), other);
        case Data::ScalarComplex128: return operator==(scalar<std::complex<double>>(value), other);
        case Data::ScalarInt8: return operator==(scalar<int8_t>(value), other);
        case Data::ScalarInt16: return operator==(scalar<int16_t>(value), other);
        case Data::ScalarInt32: return operator==(scalar<int32_t>(value), other);
        case Data::ScalarInt64: return operator==(scalar<int64_t>(value), other);
        case Data::ScalarUInt8: return operator==(scalar<uint8_t>(value), other);
        case Data::ScalarUInt16: return operator==(scalar<uint16_t>(value), other);
        case Data::ScalarUInt32: return operator==(scalar<uint32_t>(value), other);
        case Data::ScalarUInt64: return operator==(scalar<uint64_t>(value), other);
        //case Data::ScalarBool: return operator==(scalar<bool>(value), other);
        case Data::VectorFloat32: return operator==(vector<float>(value), other);
        case Data::VectorFloat64: return operator==(vector<double>(value), other);
        case Data::VectorComplex64: return operator==(vector<std::complex<float>>(value), other);
        case Data::VectorComplex128: return operator==(vector<std::complex<double>>(value), other);
        case Data::VectorInt8: return operator==(vector<int8_t>(value), other);
        case Data::VectorInt16: return operator==(vector<int16_t>(value), other);
        case Data::VectorInt32: return operator==(vector<int32_t>(value), other);
        case Data::VectorInt64: return operator==(vector<int64_t>(value), other);
        case Data::VectorUInt8: return operator==(vector<uint8_t>(value), other);
        case Data::VectorUInt16: return operator==(vector<uint16_t>(value), other);
        case Data::VectorUInt32: return operator==(vector<uint32_t>(value), other);
        case Data::VectorUInt64: return operator==(vector<uint64_t>(value), other);
        case Data::MapHeaderString: return operator==(map(value), other);
        default:
            throw std::runtime_error("Unknown pmt type passed to operator==");
    }
}

typedef std::variant<
        std::string, bool, int8_t, uint8_t, int16_t, uint16_t, int32_t,
        uint32_t, int64_t, uint64_t, float, double, std::complex<float>,
        std::complex<double>, std::vector<bool>, std::vector<int8_t>,
        std::vector<uint8_t>, std::vector<int16_t>, std::vector<uint16_t>,
        std::vector<int32_t>, std::vector<uint32_t>, std::vector<int64_t>,
        std::vector<uint64_t>, std::vector<float>, std::vector<double>,
        std::vector<std::complex<float>>, std::vector<std::complex<double>>>
        pmt_variant_t;


template <class T>
inline bool operator!=(const pmt& value, const T& other) {
    return !(value == other);
}

inline bool operator==(const map& x, const std::map<std::string, pmt>& y) {
    if (x.size() != y.size()) return false;
    for (const auto& [k, v]: x) {
        if (y.count(k) == 0) return false;
        else if (!(y.at(k) == v)) return false;
    }
    return true;
}

inline bool operator==(const map& x, const map& y) {
    if (x.size() != y.size()) return false;
    for (const auto& [k, v]: x) {
        if (y.count(k) == 0) return false;
        else if (!(y.at(k) == v)) return false;
    }
    return true;
}

inline bool operator==(const pmt& x, const std::map<std::string, pmt>& y) {
    if (x.data_type() == Data::MapHeaderString)
        return map(x) == y;
    return false;
}

template <class T>
inline bool operator!=(const map& x, const T& y) {
    return !(x == y);
}
// Need to have map operator here because it has pmts in it.
inline std::ostream& operator<<(std::ostream& os, const map& value) {
    os << "{ ";
    bool first = true;
    for (const auto& [k, v]: value) {
        if (!first) os << ", ";
        first = false;
        os << k << ": " << v;
    }
    os << " }";
    return os;
}
 


}

