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

template <typename Container>
using begin_func_t = decltype(*std::begin(std::declval<Container>()));

template<typename T, typename _ = void>
struct is_container : std::false_type {};

template<typename T>
struct is_container<
        T,
        std::void_t<
                typename T::value_type,
                typename T::size_type,
                typename T::iterator,
                typename T::const_iterator,
                decltype(std::declval<T>().size()),
                decltype(std::declval<T>().begin()),
                decltype(std::declval<T>().end())
            >
        > : public std::true_type {};

template <typename Container, typename _ = void>
struct is_map_like_container: std::false_type {};

template <typename T>
struct is_map_like_container<
        T,
        std::void_t<
                typename T::value_type,
                typename T::mapped_type,
                typename T::size_type,
                typename T::allocator_type,
                typename T::iterator,
                typename T::const_iterator,
                decltype(std::declval<T>().size()),
                decltype(std::declval<T>().begin()),
                decltype(std::declval<T>().end())
            >
        > : public std::true_type {};

template <class T, class type>
inline T _ConstructVectorLike(const pmt& value) {
    // This doesn't work because I have to be able to run every line of code instantiated.
    // Can't do _ConstructVectorLike<std::vector<int>, float>
    if constexpr(std::is_same_v<typename T::value_type, type>) {
        using iter = decltype(get_vector<type>(value).begin());
        // Vector like containers like this
        if constexpr(std::is_constructible_v<T, iter, iter>) 
            return T(get_vector<type>(value).begin(), get_vector<type>(value).end());
        else
            // String like containers like this
            return T(get_string(value).data(), get_vector<type>(value).size());
    } else {
        throw ConversionError(value, "vector", ctype_string<type>());
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
            // Need to detect if this is string like or not.
            case Data::PmtString: return _ConstructVectorLike<T, int8_t>(value);
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

