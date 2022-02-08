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
#include <ostream>
#include <map>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <type_traits>


namespace pmtf {

template <class T>
inline flatbuffers::Offset<void> CreateScalar(flatbuffers::FlatBufferBuilder& fbb, const T& value);

template <class T> struct scalar_traits;
template <> struct scalar_traits<uint8_t> { using traits = ScalarUInt8::Traits; };
template <> struct scalar_traits<uint16_t> { using traits = ScalarUInt16::Traits; };
template <> struct scalar_traits<uint32_t> { using traits = ScalarUInt32::Traits; };
template <> struct scalar_traits<uint64_t> { using traits = ScalarUInt64::Traits; };
template <> struct scalar_traits<int8_t> { using traits = ScalarInt8::Traits; };
template <> struct scalar_traits<int16_t> { using traits = ScalarInt16::Traits; };
template <> struct scalar_traits<int32_t> { using traits = ScalarInt32::Traits; };
template <> struct scalar_traits<int64_t> { using traits = ScalarInt64::Traits; };
template <> struct scalar_traits<float> { using traits = ScalarFloat32::Traits; };
template <> struct scalar_traits<double> { using traits = ScalarFloat64::Traits; };
template <> struct scalar_traits<std::complex<float>> { using traits = ScalarComplex64::Traits; };
template <> struct scalar_traits<std::complex<double>> { using traits = ScalarComplex128::Traits; };

template <class T>
class scalar {
public:
    using traits = typename scalar_traits<T>::traits;
    using type = typename traits::type;
    using value_type = T;
    scalar() { _Create(T(0)); }
    scalar(const T& value) { _Create(value); }
    scalar(const scalar<T>& other) { _Create(other.value()); }
    scalar(const pmt& other): _buf(other) {} 
    ~scalar() {}
    T value() const {
        std::shared_ptr<base_buffer> scalar = _get_buf();
        if constexpr(is_complex<T>::value)
            return *reinterpret_cast<const T*>(scalar->data_as<type>()->value());
        else
            return scalar->data_as<type>()->value();
    }
    static constexpr Data data_type() { return DataTraits<type>::enum_value; }
    scalar& operator=(const T& value) {
        std::shared_ptr<base_buffer> scalar = _get_buf();
        if constexpr(is_complex<T>::value) {
            auto mv = scalar->data_as<type>()->mutable_value();
            mv->mutate_re(value.real());
            mv->mutate_im(value.imag());
        } else
            scalar->data_as<type>()->mutate_value(value);
        return *this;        
    }
    scalar& operator=(const scalar<T>& value) {
        return this->operator=(value.value());
    }
    const pmt& get_pmt_buffer() const { return _buf; }
    void print(std::ostream& os) const { os << value(); }
    // Cast operators
    //! Cast to a T value.
    //! Explicit means that the user must do something like T(scalar<T>(val));
    //! Implicit conversions can cause lots of problems, so we are avoiding them.
    explicit operator T() const { return value(); }
    //! Cast to another type
    //! Will cause a compilation failure if we can't do the cast.
    template <class U>
    explicit operator U() const { return U(value()); }

    //! Equality Comparisons
    // Declared as class members so that we don't do implicit conversions.
    template <class U>
    bool operator==(const U& x) const;
    template <class U>
    bool operator!=(const U& x) const { return !(operator==(x));}
private:
    void _Create(const T& value) {
        flatbuffers::FlatBufferBuilder fbb(128);
        flatbuffers::Offset<void> offset;
        //auto offset = traits::Create(fbb, value).Union();
        if constexpr(is_complex<T>::value) {
            auto ptr = reinterpret_cast<const typename scalar_type<T>::type*>(&value);
            offset = traits::Create(fbb, ptr).Union();
        } else {
            offset = traits::Create(fbb, value).Union();
        }
        auto pmt = CreatePmt(fbb, data_type(), offset);
        fbb.FinishSizePrefixed(pmt);
        _get_buf() = std::make_shared<base_buffer>(fbb.Release());
    }
    std::shared_ptr<base_buffer>& _get_buf() { return _buf._scalar; }
    const std::shared_ptr<base_buffer> _get_buf() const { return _buf._scalar; }
    pmt _buf;
    
};

template <class T>
struct is_scalar : std::false_type {};

template <class T>
struct is_scalar<scalar<T>> : std::true_type {};

template <class T, class U>
using IsNotScalarT = std::enable_if_t<!std::is_same_v<scalar<T>, U>, bool>;

// In C++20 we can replace this with a concept.
#define IMPLEMENT_SCALAR_PMT(type) \
template <> inline pmt& pmt::operator=<type>(const type& x) \
    { return operator=(scalar(x).get_pmt_buffer()); } \
template <> inline pmt& pmt::operator=<scalar<type>>(const scalar<type>& x) \
    { return operator=(x.get_pmt_buffer()); } \
template <> inline pmt::pmt<type>(const type& x) \
    { operator=(scalar(x).get_pmt_buffer()); } \
template <> inline pmt::pmt<scalar<type>>(const scalar<type>& x) \
    { operator=(x.get_pmt_buffer()); }

IMPLEMENT_SCALAR_PMT(uint8_t)
IMPLEMENT_SCALAR_PMT(uint16_t)
IMPLEMENT_SCALAR_PMT(uint32_t)
IMPLEMENT_SCALAR_PMT(uint64_t)
IMPLEMENT_SCALAR_PMT(int8_t)
IMPLEMENT_SCALAR_PMT(int16_t)
IMPLEMENT_SCALAR_PMT(int32_t)
IMPLEMENT_SCALAR_PMT(int64_t)
IMPLEMENT_SCALAR_PMT(float)
IMPLEMENT_SCALAR_PMT(double)
IMPLEMENT_SCALAR_PMT(std::complex<float>)
IMPLEMENT_SCALAR_PMT(std::complex<double>)



// The catch all case.
template <class T>
template <class U>
bool scalar<T>::operator==(const U& y) const {
    // U is a plain old data type (scalar<float> == float)
    if constexpr(std::is_same_v<T, U>)
        return value() == y;
    else if constexpr(is_scalar<U>::value) {
        if constexpr(std::is_convertible_v<typename U::value_type, T>)
            return value() == T(y.value());
        else return false;
    } else if constexpr(std::is_same_v<U, pmt>)
        return y == value();
    // Can U be converted to T?
    else if constexpr(std::is_convertible_v<U, T>)
        return value() == T(y);
    return false;
}

// Reversed case.  This allows for x == y and y == x
template <class T, class U, IsNotScalarT<T,U> = true>
bool operator==(const U& y, const scalar<T>& x) {
    return x.operator==(y);
}

// Reversed Not equal operator
template <class T, class U, IsNotScalarT<T,U> = true>
bool operator!=(const U& y, const scalar<T>& x) {
    return operator!=(x,y);
}


template <class T>
std::ostream& operator<<(std::ostream& os, const scalar<T>& x) {
    os << x.value();
    return os;
}

template <class T>
scalar<T> get_scalar(const pmt& p) {
    if (p.data_type() == scalar<T>::data_type())
        return scalar<T>(p);
    throw ConversionError(p, "scalar", ctype_string<T>());
}

}

