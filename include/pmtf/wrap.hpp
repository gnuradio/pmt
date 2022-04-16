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
#include <pmtf/tag.hpp>
#include <variant>

namespace pmtf {


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
        case Data::ScalarBool: return scalar<bool>(*this).operator==(other);
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
        case Data::Tag: return tag(*this).operator==(other);
        default:
            throw std::runtime_error("Unknown pmt type passed to operator==");
    }
}

template <class T>
inline T get_as(const pmt& value) {
    if constexpr(is_map_like_container<T>::value) {
        if (value.data_type() == Data::MapHeaderString) {
            return T(map(value).begin(), map(value).end());
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
            case Data::ScalarFloat32: return static_cast<T>(scalar<float>(value));
            case Data::ScalarFloat64: return static_cast<T>(scalar<double>(value));
            case Data::ScalarInt8: return static_cast<T>(scalar<int8_t>(value));
            case Data::ScalarInt16: return static_cast<T>(scalar<int16_t>(value));
            case Data::ScalarInt32: return static_cast<T>(scalar<int32_t>(value));
            case Data::ScalarInt64: return static_cast<T>(scalar<int64_t>(value));
            case Data::ScalarUInt8: return static_cast<T>(scalar<uint8_t>(value));
            case Data::ScalarUInt16: return static_cast<T>(scalar<uint16_t>(value));
            case Data::ScalarUInt32: return static_cast<T>(scalar<uint32_t>(value));
            case Data::ScalarUInt64: return static_cast<T>(scalar<uint64_t>(value));
            case Data::ScalarComplex64: return static_cast<T>(scalar<std::complex<float>>(value));
            case Data::ScalarComplex128: return static_cast<T>(scalar<std::complex<double>>(value));
            default: throw ConversionError(value, "scalar", "complex");
        }
    } else {
        switch (value.data_type()) {
            case Data::ScalarBool: return static_cast<T>(scalar<bool>(value));
            case Data::ScalarFloat32: return static_cast<T>(scalar<float>(value));
            case Data::ScalarFloat64: return static_cast<T>(scalar<double>(value));
            case Data::ScalarInt8: return static_cast<T>(scalar<int8_t>(value));
            case Data::ScalarInt16: return static_cast<T>(scalar<int16_t>(value));
            case Data::ScalarInt32: return static_cast<T>(scalar<int32_t>(value));
            case Data::ScalarInt64: return static_cast<T>(scalar<int64_t>(value));
            case Data::ScalarUInt8: return static_cast<T>(scalar<uint8_t>(value));
            case Data::ScalarUInt16: return static_cast<T>(scalar<uint16_t>(value));
            case Data::ScalarUInt32: return static_cast<T>(scalar<uint32_t>(value));
            case Data::ScalarUInt64: return static_cast<T>(scalar<uint64_t>(value));
            //case Data::ScalarBool: return operator==(scalar<bool>(value), other);
            default:
                throw ConversionError(value, "scalar", "type T");
        }
    }

}
 
void pmt::pre_serial_update() const {
    // If other is a pmt, then we will convert the first arg to its type
    // Then we will call this again to convert the second arg.
    std::cout << (data_type() == Data::MapHeaderString) << " " <<  (data_type() == Data::Tag) << std::endl;
    switch (data_type()) {
        case Data::MapHeaderString:
            map(*this).pre_serial_update();
            break;
        case Data::Tag:
            tag(*this).pre_serial_update();
            break;
        default:
            return;
    }
}

}

