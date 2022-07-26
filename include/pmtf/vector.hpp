/*-*-c++-*-*/
/*
 * Copyright 2021 John Sallay
 * Copyright 2021 Josh Morman
 *
 * SPDX-License-Identifier: LGPL-3.0
 *
 */
#pragma once

#include <pmtf/pmtf_generated.h>
#include <complex>
#include <iostream>
#include <map>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <vector>

#include <pmtf/base.hpp>
#include <pmtf/type_helpers.hpp>
#include <pmtf/gsl-lite.hpp>

// Need to define packing functions for complex<double>
// flatbuffers knows how to pack the other types that we use.
namespace flatbuffers {
    pmtf::Complex128 Pack(std::complex<double> const& obj);

    std::complex<double> UnPack(const pmtf::Complex128& obj);
}

namespace pmtf {

template <class T> struct vector_traits;
template <> struct vector_traits<uint8_t> { using traits = VectorUInt8::Traits; };
template <> struct vector_traits<uint16_t> { using traits = VectorUInt16::Traits; };
template <> struct vector_traits<uint32_t> { using traits = VectorUInt32::Traits; };
template <> struct vector_traits<uint64_t> { using traits = VectorUInt64::Traits; };
template <> struct vector_traits<int8_t> { using traits = VectorInt8::Traits; };
template <> struct vector_traits<int16_t> { using traits = VectorInt16::Traits; };
template <> struct vector_traits<int32_t> { using traits = VectorInt32::Traits; };
template <> struct vector_traits<int64_t> { using traits = VectorInt64::Traits; };
template <> struct vector_traits<float> { using traits = VectorFloat32::Traits; };
template <> struct vector_traits<double> { using traits = VectorFloat64::Traits; };
template <> struct vector_traits<std::complex<float>> { using traits = VectorComplex64::Traits; };
template <> struct vector_traits<std::complex<double>> { using traits = VectorComplex128::Traits; };
template <> struct vector_traits<pmt> { using traits = VectorPmtHeader::Traits; };


// There are two vector constructors that take two arguments.
// For integer types, they can get confused.  Need to use SFINAE to
// disambiguate them.
template <typename T>
using IsNotInteger = std::enable_if_t<!std::is_integral_v<T>>;

template <class T>
class vector {
public:
    using traits = typename vector_traits<T>::traits;
    using type = typename traits::type;
    using span_type = typename gsl::span<T>;
    using const_span_type = typename gsl::span<const T>;
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using size_type = size_t;

    using iterator = typename span_type::iterator;
    using const_iterator = typename span_type::const_iterator;

    // Default constructor
    vector() {
        _MakeVector(0);
    }
    // Constuct with unitialized memory
    vector(size_t size) {
        _MakeVector(size);
    }
    // Fill Constructor
    explicit vector(size_t size, const T& set_value) {
        _MakeVector(size);
        std::fill(value().begin(), value().end(), set_value);
    }
    // Range Constuctor
    // Need the IsNotInteger to not conflict with the fill constructor
    template <class InputIterator, typename = IsNotInteger<InputIterator>>
    vector(InputIterator first, InputIterator last) {
        _MakeVector(&(*first), std::distance(first, last));
    }
    // Copy from vector Constructor
    vector(const std::vector<T>& value) {
        _MakeVector(value.data(), value.size());
    }
    // Copy Constructor
    vector(const vector<T>& value) {
        _MakeVector(value.data(), value.size());
    }
    // Initializer list Constructor
    vector(std::initializer_list<value_type> il) {
        _MakeVector(il.begin(), il.size());
    }

    // From a pmt buffer
    template <class U, typename = IsPmt<U>>
    vector(const U& other) {
        if (other.data_type() != data_type())
            throw ConversionError(other, "vector", ctype_string<T>());
        _buf = other;
    }
        
    ~vector() {}
    span_type value() {
        if constexpr(std::is_same_v<T, pmt>)
            return span_type(_get_buf()->data(), _get_buf()->size());
        else {
            std::shared_ptr<base_buffer> vector = _get_buf();
            auto buf = vector->data_as<type>()->value();
            if constexpr(is_complex<T>::value) {
                auto tbuf = reinterpret_cast<const T*>(buf->data());
                return gsl::span<T>(const_cast<T*>(tbuf), buf->size());
            } else {
                return gsl::span<T>(const_cast<T*>(buf->data()), buf->size());
            }
        }
    }
    const_span_type value() const {
        if constexpr(std::is_same_v<T, pmt>)
            return const_span_type(_get_buf()->data(), _get_buf()->size());
        else {
            std::shared_ptr<base_buffer> vector = _get_buf();
            auto buf = vector->data_as<type>()->value();
            if constexpr(is_complex<T>::value) {
                return gsl::span<const T>(reinterpret_cast<const T*>(buf->data()), buf->size());
           } else {
                return gsl::span<const T>(buf->data(), buf->size());
            }
        }
    }
    static constexpr Data data_type() { return DataTraits<type>::enum_value; }
    vector& operator=(const T& value) {
        _MakeVector(value.begin(), value.size()); 
    }
    T* data() { return value().data(); }
    const T* data() const { return value().data(); }
    size_t size() const { return value().size(); }
    const pmt& get_pmt_buffer() const { return _buf; }
    typename span_type::iterator begin() { return value().begin(); }
    typename span_type::iterator end() { return value().end(); }
    typename span_type::const_iterator begin() const { return value().begin(); }
    typename span_type::const_iterator end() const { return value().end(); }
    reference operator[] (size_type n) {
        // operator[] doesn't do bounds checking, use at for that
        // TODO: implement at
        return data()[n];
    }
    const reference operator[] (size_type n) const {
        return data()[n];
    }
    void resize(size_type n) {
        if constexpr(std::is_same_v<T, pmt>) {
            _get_buf()->resize(n);
            std::shared_ptr<base_buffer> scalar = _buf._scalar;
            scalar->data_as<type>()->mutate_count(_get_buf()->size());
        } else throw std::runtime_error("Not implemented");
    }
    //! Equality Comparisons
    // Declared as class members so that we don't do implicit conversions.
    template <class U>
    bool operator==(const U& x) const;
    template <class U>
    bool operator!=(const U& x) const { return !(operator==(x));}
private:
    void _MakeVector(const T* data, size_t size) {
        flatbuffers::FlatBufferBuilder fbb;
        if constexpr(std::is_same_v<T, pmt>) {
            fbb.ForceDefaults(true);
            auto offset = traits::Create(fbb, size).Union();
            auto pmt = CreatePmt(fbb, data_type(), offset);
            fbb.FinishSizePrefixed(pmt);
            _buf._scalar = std::make_shared<base_buffer>(fbb.Release());
            _buf._vector = std::make_shared<std::vector<pmtf::pmt>>(data, data + size);
        } else if constexpr(is_complex<T>::value) {
            using stype = typename scalar_type<T>::type;
            //auto offset = fbb.CreateVectorOfNativeStructs<stype>(reinterpret_cast<const stype*>(data), size);
            auto offset = fbb.CreateVectorOfNativeStructs<stype>(data, size);
            _Create(fbb, traits::Create(fbb, offset).Union());
        } else {
            auto offset = fbb.CreateVector(data, size);
            _Create(fbb, traits::Create(fbb, offset).Union());
        }
    }
    void _MakeVector(size_t size) {
        flatbuffers::FlatBufferBuilder fbb;
        if constexpr(std::is_same_v<T, pmt>) {
            fbb.ForceDefaults(true);
            auto offset = traits::Create(fbb, size).Union();
            auto pmt = CreatePmt(fbb, data_type(), offset);
            fbb.FinishSizePrefixed(pmt);
            _buf._scalar = std::make_shared<base_buffer>(fbb.Release());
            _buf._vector = std::make_shared<std::vector<pmtf::pmt>>(size);
        } else if constexpr(is_complex<T>::value) {
            uint8_t* ignore;
            auto offset = fbb.CreateUninitializedVector(size, sizeof(T), &ignore);
            _Create(fbb, traits::Create(fbb, offset).Union());
        } else {
            T* ignore;
            auto offset = fbb.CreateUninitializedVector(size, &ignore);
            _Create(fbb, traits::Create(fbb, offset).Union());
        }
    }
    void _Create(flatbuffers::FlatBufferBuilder& fbb, flatbuffers::Offset<void> offset) {
        PmtBuilder pb(fbb);
        pb.add_data_type(this->data_type());
        pb.add_data(offset);
        auto blob = pb.Finish();
        fbb.FinishSizePrefixed(blob);
        _get_buf() = std::make_shared<base_buffer>(fbb.Release());
    }
    auto& _get_buf() { 
        if constexpr(std::is_same_v<T, pmt>) return _buf._vector;
        else return _buf._scalar;
    }
    const auto& _get_buf() const {
        if constexpr(std::is_same_v<T, pmt>) return _buf._vector;
        else return _buf._scalar;
    }
    pmt _buf;
};

template <class T>
struct is_vector : std::false_type {};

template <class T>
struct is_vector<vector<T>> : std::true_type {};

template <class T, class U>
using IsNotVectorT = std::enable_if_t<!std::is_same_v<vector<T>, U>, bool>;

template <class T>
std::ostream& operator<<(std::ostream& os, const vector<T>& value) {
    os << "[ ";
    bool first = true;
    for (auto& v: value) {
        if (!first) os << ", ";
        first = false;
        os << v;
    }
    os << " ]";
    return os;
}

template <class T>
template <class U>
bool vector<T>::operator==(const U& other) const {
    // We can only compare true against containers
    if constexpr(is_vector_like_container<U>::value) {
        if constexpr(std::is_same_v<typename U::value_type, T>) {
            if (size() != other.size()) return false;
            return std::equal(begin(), end(), other.begin());
        }
        return false;
    } else if constexpr(std::is_same_v<U, pmt>) {
        return other.operator==(*this);
    } else {
        return false;
    }
}

// Reversed case.  This allows for x == y and y == x
template <class T, class U, IsNotVectorT<T, U> = true>
bool operator==(const U& y, const vector<T>& x) {
    return x.operator==(y);
}

// Reversed Not equal operator
template <class T, class U, IsNotVectorT<T, U> = true>
bool operator!=(const U& y, const vector<T>& x) {
    return operator!=(x,y);
}

#define Apply(func) \
func(uint8_t) \
func(uint16_t) \
func(uint32_t) \
func(uint64_t) \
func(int8_t) \
func(int16_t) \
func(int32_t) \
func(int64_t) \
func(float) \
func(double) \
func(std::complex<float>)\
func(std::complex<double>)\
func(pmtf::pmt)

#define VectorPmt(T) \
template <> inline pmt::pmt<std::vector<T>>(const std::vector<T>& x) \
    { *this = vector<T>(x).get_pmt_buffer(); } \
template <> inline pmt::pmt<vector<T>>(const vector<T>& x) { *this = x.get_pmt_buffer(); } \
template <> inline pmt::pmt<gsl::span<T>>(const gsl::span<T>& x) { *this = vector<T>(x.begin(), x.end()).get_pmt_buffer(); } \
template <> inline pmt& pmt::operator=<vector<T>>(const vector<T>& x) \
    { return operator=(x.get_pmt_buffer()); } \
template <> inline pmt& pmt::operator=<std::vector<T>>(const std::vector<T>& x) \
    { return operator=(vector(x).get_pmt_buffer()); } 

Apply(VectorPmt)
#undef VectorPmt
#undef Apply

template <class T, class type>
inline T _ConstructVectorLike(const pmt& value) {
    if constexpr(std::is_same_v<typename T::value_type, type>) {
        using iter = decltype(vector<type>(value).begin());
        // Vector like containers like this
        if constexpr(std::is_constructible_v<T, iter, iter>) {
            return T(vector<type>(value).begin(), vector<type>(value).end());
        } else {
            throw ConversionError(value, "vector", ctype_string<type>());
        }
    } else {
        throw ConversionError(value, "vector", ctype_string<type>());
    }
}

/* Wrapper class that stores a vector of arithmetic data (not pmts)
   This is a convenience class */
class vector_wrap {
  public:
    // From a pmt buffer
    template <class U, typename = IsPmt<U>>
    vector_wrap(const U& other) {
        switch(other.data_type()) {
            case Data::VectorFloat32:
            case Data::VectorFloat64:
            case Data::VectorComplex64:
            case Data::VectorComplex128:
            case Data::VectorInt8:
            case Data::VectorInt16:
            case Data::VectorInt32:
            case Data::VectorInt64:
            case Data::VectorUInt8:
            case Data::VectorUInt16:
            case Data::VectorUInt32:
            case Data::VectorUInt64:
                _buf = other;
                break;
            default:
                throw ConversionError(other, "vector", ctype_string<U>());
        }
    }
    template <class U, typename = IsVectorLikeContainer<U>, typename = IsNotPmt<U>>
    vector_wrap(const U& other) {
        _buf = other;
    }
    const pmt& get_pmt_buffer() const { return _buf; }
    size_t size() const { return _buf.elements(); }
    const size_t bytes() const { return size() * _buf.bytes_per_element(); }
    const size_t bytes_per_element() const { return _buf.bytes_per_element(); }
    template <class U>
    bool operator==(const U& other) const { return _buf ==  other; }
    template <class U>
    bool operator!=(const U& other) const { return !operator==(other); }

  private:
    pmt _buf;
};

std::ostream& operator<<(std::ostream& os, const vector_wrap& value);

template <class U>
using IsNotVectorWrap = std::enable_if_t<!std::is_same_v<vector_wrap, U>, bool>;


// Reversed case.  This allows for x == y and y == x
template <class U, IsNotVectorWrap<U> = true>
bool operator==(const U& y, const vector_wrap& x) {
    return x.operator==(y);
}

// Reversed Not equal operator
template <class U, IsNotVectorWrap<U> = true>
bool operator!=(const U& y, const vector_wrap& x) {
    return x.operator!=(y);
}

template <> inline pmt::pmt<vector_wrap>(const vector_wrap& x) { *this = x.get_pmt_buffer(); }
} // namespace pmtf
