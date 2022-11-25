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
template <class T, IsMap<T> = true>
std::ostream& operator<<(std::ostream& os, const T& value);
template <class T, typename = IsPmt<T>>
std::ostream& operator<<(std::ostream& os, const T& value);


typedef std::variant<
        std::string, bool, int8_t, uint8_t, int16_t, uint16_t, int32_t,
        uint32_t, int64_t, uint64_t, float, double, std::complex<float>,
        std::complex<double>, std::vector<bool>, std::vector<int8_t>,
        std::vector<uint8_t>, std::vector<int16_t>, std::vector<uint16_t>,
        std::vector<int32_t>, std::vector<uint32_t>, std::vector<int64_t>,
        std::vector<uint64_t>, std::vector<float>, std::vector<double>,
        std::vector<std::complex<float>>, std::vector<std::complex<double>>>
        pmt_variant_t;


// Reversed case.  This allows for x == y and y == x
template <class T, class U, IsPmt<T> = true, IsNotPmt<U> = true, IsNotPmtDerived<U> = true>
bool operator==(const U& y, const T& x) {
    return x.operator==(y);
}

// Reversed Not equal operator
template <class T, class U, IsMap<T> = true, IsNotPmt<U> = true, IsNotPmtDerived<U> = true>
bool operator!=(const U& y, const T& x) {
    return operator!=(x,y);
}

template <class T>
bool pmt::operator==(const T& other) const {
    // If other is a pmt, then we will convert the first arg to its type
    // Then we will call this again to convert the second arg.
    switch (data_type()) {
        case Data::PmtString: return string(*this).operator==(other);
        case Data::ScalarFloat32: return scalar<float>(*this).operator==(other);
        case Data::ScalarFloat64: return scalar<double>(*this).operator==(other);
        case Data::ScalarComplex64: return scalar<std::complex<float>>(*this).operator==(other);
        case Data::ScalarComplex128: return scalar<std::complex<double>>(*this).operator==(other);
        case Data::ScalarInt8: return scalar<int8_t>(*this).operator==(other);
        case Data::ScalarInt16: return scalar<int16_t>(*this).operator==(other);
        case Data::ScalarInt32: return scalar<int32_t>(*this).operator==(other);
        case Data::ScalarInt64: return scalar<int64_t>(*this).operator==(other);
        case Data::ScalarUInt8: return scalar<uint8_t>(*this).operator==(other);
        case Data::ScalarUInt16: return scalar<uint16_t>(*this).operator==(other);
        case Data::ScalarUInt32: return scalar<uint32_t>(*this).operator==(other);
        case Data::ScalarUInt64: return scalar<uint64_t>(*this).operator==(other);
        //case Data::ScalarBool: return scalar<bool>(*this).operator==(other);
        case Data::VectorFloat32: return vector<float>(*this).operator==(other);
        case Data::VectorFloat64: return vector<double>(*this).operator==(other);
        case Data::VectorComplex64: return vector<std::complex<float>>(*this).operator==(other);
        case Data::VectorComplex128: return vector<std::complex<double>>(*this).operator==(other);
        case Data::VectorInt8: return vector<int8_t>(*this).operator==(other);
        case Data::VectorInt16: return vector<int16_t>(*this).operator==(other);
        case Data::VectorInt32: return vector<int32_t>(*this).operator==(other);
        case Data::VectorInt64: return vector<int64_t>(*this).operator==(other);
        case Data::VectorUInt8: return vector<uint8_t>(*this).operator==(other);
        case Data::VectorUInt16: return vector<uint16_t>(*this).operator==(other);
        case Data::VectorUInt32: return vector<uint32_t>(*this).operator==(other);
        case Data::VectorUInt64: return vector<uint64_t>(*this).operator==(other);
        case Data::VectorPmtHeader: return vector<pmt>(*this).operator==(other);
        case Data::MapHeaderString: return map(*this).operator==(other);
        default:
            throw std::runtime_error("Unknown pmt type passed to operator==");
    }
}

// Need to have map operator here because it has pmts in it.
template <class T, IsMap<T> = true>
std::ostream& operator<<(std::ostream& os, const T& value) {
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

template <class T, typename = IsPmt<T>>
std::ostream& operator<<(std::ostream& os, const T& value) {
    switch(value.data_type()) {
        case Data::PmtString: return operator<<(os, string(value));
        case Data::ScalarFloat32: return operator<<(os, scalar<float>(value));
        case Data::ScalarFloat64: return operator<<(os, scalar<double>(value));
        case Data::ScalarComplex64: return operator<<(os, scalar<std::complex<float>>(value));
        case Data::ScalarComplex128: return operator<<(os, scalar<std::complex<double>>(value));
        case Data::ScalarInt8: return operator<<(os, scalar<int8_t>(value));
        case Data::ScalarInt16: return operator<<(os, scalar<int16_t>(value));
        case Data::ScalarInt32: return operator<<(os, scalar<int32_t>(value));
        case Data::ScalarInt64: return operator<<(os, scalar<int64_t>(value));
        case Data::ScalarUInt8: return operator<<(os, scalar<uint8_t>(value));
        case Data::ScalarUInt16: return operator<<(os, scalar<uint16_t>(value));
        case Data::ScalarUInt32: return operator<<(os, scalar<uint32_t>(value));
        case Data::ScalarUInt64: return operator<<(os, scalar<uint64_t>(value));
        //case Data::ScalarBool: return operator<<(os, scalar<bool>(value));
        case Data::VectorFloat32: return operator<<(os, vector<float>(value));
        case Data::VectorFloat64: return operator<<(os, vector<double>(value));
        case Data::VectorComplex64: return operator<<(os, vector<std::complex<float>>(value));
        case Data::VectorComplex128: return operator<<(os, vector<std::complex<double>>(value));
        case Data::VectorInt8: return operator<<(os, vector<int8_t>(value));
        case Data::VectorInt16: return operator<<(os, vector<int16_t>(value));
        case Data::VectorInt32: return operator<<(os, vector<int32_t>(value));
        case Data::VectorInt64: return operator<<(os, vector<int64_t>(value));
        case Data::VectorUInt8: return operator<<(os, vector<uint8_t>(value));
        case Data::VectorUInt16: return operator<<(os, vector<uint16_t>(value));
        case Data::VectorUInt32: return operator<<(os, vector<uint32_t>(value));
        case Data::VectorUInt64: return operator<<(os, vector<uint64_t>(value));
        case Data::VectorPmtHeader: return operator<<(os, vector<pmt>(value));
        case Data::MapHeaderString: return operator<<(os, map(value));
        default:
            throw std::runtime_error("Unknown pmt type passed to operator<<");
    }
}

template <typename Container>
using begin_func_t = decltype(*std::begin(std::declval<Container>()));


template <class T, class type>
inline T _ConstructVectorLike(const pmt& value) {
    // This doesn't work because I have to be able to run every line of code instantiated.
    // Can't do _ConstructVectorLike<std::vector<int>, float>
    if constexpr(std::is_same_v<typename T::value_type, type>) {
        using iter = decltype(get_vector<type>(value).begin());
        // Vector like containers like this
        if constexpr(std::is_constructible_v<T, iter, iter>) {
            return T(get_vector<type>(value).begin(), get_vector<type>(value).end());
        } else {
            throw ConversionError(value, "vector", ctype_string<type>());
        }
    } else {
        throw ConversionError(value, "vector", ctype_string<type>());
    }
}

template <class T, class type>
inline T _ConstructStringLike(const pmt& value) {
    if constexpr(std::is_same_v<typename T::value_type, type>) {
        if constexpr(std::is_constructible_v<T, const type*, size_t>) {
            return T(get_string(value).data(), get_string(value).size());
        } else {
            throw ConversionError(value, "string", ctype_string<type>());
        }
    } else {
        throw ConversionError(value, "string", ctype_string<type>());
    }
}
template <class T>
inline T get_as(const pmt& value) {
    if constexpr(is_map_like_container<T>::value) {
        if (value.data_type() == Data::MapHeaderString) {
            return T(get_map(value).begin(), get_map(value).end());
        } else
            throw ConversionError(value, "map", "map-like container");
    } else if constexpr(is_container<T>::value) {
        switch (value.data_type()) {
            case Data::VectorFloat32: return _ConstructVectorLike<T, float>(value);
            case Data::VectorFloat64: return _ConstructVectorLike<T, double>(value);
            case Data::VectorComplex64: return _ConstructVectorLike<T, std::complex<float>>(value);
            case Data::VectorComplex128: return _ConstructVectorLike<T, std::complex<double>>(value);
            case Data::VectorInt8: return _ConstructVectorLike<T, int8_t>(value);
            case Data::VectorInt16: return _ConstructVectorLike<T, int16_t>(value);
            case Data::VectorInt32: return _ConstructVectorLike<T, int32_t>(value);
            case Data::VectorInt64: return _ConstructVectorLike<T, int64_t>(value);
            case Data::VectorUInt8: return _ConstructVectorLike<T, uint8_t>(value);
            case Data::VectorUInt16: return _ConstructVectorLike<T, uint16_t>(value);
            case Data::VectorUInt32: return _ConstructVectorLike<T, uint32_t>(value);
            case Data::VectorUInt64: return _ConstructVectorLike<T, uint64_t>(value);
            case Data::VectorPmtHeader: return _ConstructVectorLike<T, pmt>(value);
            // Need to detect if this is string like or not.
            case Data::PmtString: return _ConstructStringLike<T, char>(value);
            default: throw ConversionError(value, "vector", "vector-like container");
        }
    } else if constexpr(is_complex<T>::value) {
        // We can convert scalars to complex, but not the other way around.
        switch (value.data_type()) {
            case Data::ScalarFloat32: return static_cast<T>(get_scalar<float>(value));
            case Data::ScalarFloat64: return static_cast<T>(get_scalar<double>(value));
            case Data::ScalarInt8: return static_cast<T>(get_scalar<int8_t>(value));
            case Data::ScalarInt16: return static_cast<T>(get_scalar<int16_t>(value));
            case Data::ScalarInt32: return static_cast<T>(get_scalar<int32_t>(value));
            case Data::ScalarInt64: return static_cast<T>(get_scalar<int64_t>(value));
            case Data::ScalarUInt8: return static_cast<T>(get_scalar<uint8_t>(value));
            case Data::ScalarUInt16: return static_cast<T>(get_scalar<uint16_t>(value));
            case Data::ScalarUInt32: return static_cast<T>(get_scalar<uint32_t>(value));
            case Data::ScalarUInt64: return static_cast<T>(get_scalar<uint64_t>(value));
            case Data::ScalarComplex64: return static_cast<T>(get_scalar<std::complex<float>>(value));
            case Data::ScalarComplex128: return static_cast<T>(get_scalar<std::complex<double>>(value));
            default: throw ConversionError(value, "scalar", "complex");
        }
    } else {
        switch (value.data_type()) {
            case Data::ScalarBool: return static_cast<T>(get_scalar<bool>(value));
            case Data::ScalarFloat32: return static_cast<T>(get_scalar<float>(value));
            case Data::ScalarFloat64: return static_cast<T>(get_scalar<double>(value));
            case Data::ScalarInt8: return static_cast<T>(get_scalar<int8_t>(value));
            case Data::ScalarInt16: return static_cast<T>(get_scalar<int16_t>(value));
            case Data::ScalarInt32: return static_cast<T>(get_scalar<int32_t>(value));
            case Data::ScalarInt64: return static_cast<T>(get_scalar<int64_t>(value));
            case Data::ScalarUInt8: return static_cast<T>(get_scalar<uint8_t>(value));
            case Data::ScalarUInt16: return static_cast<T>(get_scalar<uint16_t>(value));
            case Data::ScalarUInt32: return static_cast<T>(get_scalar<uint32_t>(value));
            case Data::ScalarUInt64: return static_cast<T>(get_scalar<uint64_t>(value));
            //case Data::ScalarBool: return operator==(scalar<bool>(value), other);
            default:
                throw ConversionError(value, "scalar", "type T");
        }
    }

}
 

}

