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
    using span = typename gsl::span<T>;
    using const_span = typename gsl::span<const T>;
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using size_type = size_t;

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
    vector(const pmt& other): _buf(other) {}
        
    ~vector() {}
    span value() {
        std::shared_ptr<base_buffer> vector = _get_buf();
        auto buf = vector->data_as<type>()->value();
        if constexpr(is_complex<T>::value) {
            auto tbuf = reinterpret_cast<const T*>(buf->data());
            return gsl::span<T>(const_cast<T*>(tbuf), buf->size());
        } else {
            return gsl::span<T>(const_cast<T*>(buf->data()), buf->size());
        }
    }
    const_span value() const {
        std::shared_ptr<base_buffer> vector = _get_buf();
        auto buf = vector->data_as<type>()->value();
        if constexpr(is_complex<T>::value) {
            return gsl::span<const T>(reinterpret_cast<const T*>(buf->data()), buf->size());
        } else {
            return gsl::span<const T>(buf->data(), buf->size());
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
    typename span::iterator begin() { return value().begin(); }
    typename span::iterator end() { return value().end(); }
    typename span::const_iterator begin() const { return value().begin(); }
    typename span::const_iterator end() const { return value().end(); }
    reference operator[] (size_type n) {
        // operator[] doesn't do bounds checking, use at for that
        // TODO: implement at
        return data()[n];
    }
    const reference operator[] (size_type n) const {
        return data()[n];
    }
    void print(std::ostream& os) const {
        os << "[";
        for (auto& e : value()) {
            os << e << ", ";
        }
        os << "]";
    }
private:
    void _MakeVector(const T* data, size_t size) {
        flatbuffers::FlatBufferBuilder fbb;
        if constexpr(is_complex<T>::value) {
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
        if constexpr(is_complex<T>::value) {
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
    std::shared_ptr<base_buffer>& _get_buf() { return _buf._scalar; }
    const std::shared_ptr<base_buffer> _get_buf() const { return _buf._scalar; }
    pmt _buf;
};

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

// Compare against a span
template <class T>
bool operator==(const vector<T>& x, const gsl::span<const T>& other) {
    if (other.size() != x.size()) return false;
    auto my_val = x.begin();
    for (auto&& val : other) {
        if (*my_val != val) return false;
        my_val++;
    }
    return true;
}

template <class T>
bool operator==(const gsl::span<const T>& other, const vector<T>& x) {
    return operator==(x, other);
}

// Compare against another vector (std or pmt)
template <class T>
bool operator==(const vector<T>& x, const std::vector<T>& other) {
    return x == gsl::span(other);
}
template <class T>
bool operator==(const std::vector<T>& other, const vector<T>& x) {
    return x == gsl::span(other);
}

template <class T>
bool operator==(const vector<T>& x, const vector<T>& other) {
    return x == gsl::span(other);
}

// Blanket other case.
template <class T, class U>
bool operator==(const vector<T>& x, const U& other) {
    return false;
}

template <class T, class U>
bool operator!=(const vector<T>& x, const U& other) {
    return !(x == other);
}

template <class T, class U>
bool operator!=(const U& other, const vector<T>& x) {
    return !(x == other);
}

// Pmt == cases
template <class T>
bool operator==(const pmt& x, const gsl::span<const T>& y) {
    switch(x.data_type()) {
        case Data::VectorFloat32: return vector<float>(x) == y;
        case Data::VectorFloat64: return vector<double>(x) == y;
        case Data::VectorComplex64: return vector<std::complex<float>>(x) == y;
        case Data::VectorComplex128: return vector<std::complex<double>>(x) == y;
        case Data::VectorInt8: return vector<int8_t>(x) == y;
        case Data::VectorInt16: return vector<int16_t>(x) == y;
        case Data::VectorInt32: return vector<int32_t>(x) == y;
        case Data::VectorInt64: return vector<int64_t>(x) == y;
        case Data::VectorUInt8: return vector<uint8_t>(x) == y;
        case Data::VectorUInt16: return vector<uint16_t>(x) == y;
        case Data::VectorUInt32: return vector<uint32_t>(x) == y;
        case Data::VectorUInt64: return vector<uint64_t>(x) == y;
        default: return false;
    }
}

template <class T>
bool operator==(const gsl::span<const T>& y, const pmt& x) {
    return operator==(x, y);
}
template <class T>
bool operator==(const pmt& x, const std::vector<T>& other) {
    return x == gsl::span(other);
}
template <class T>
bool operator==(const std::vector<T>& other, const pmt& x) {
    return x == gsl::span(other);
}

template <class T>
bool operator==(const pmt& x, const vector<T>& other) {
    return x == gsl::span(other);
}
template <class T>
bool operator==(const vector<T>& other, const pmt& x) {
    return x == gsl::span(other);
}

// When we switch to c++20, make this a concept.
/*template <class T, class U>
bool operator==(const vector<T>& x, const U& other) {
    if (other.size() != x.size()) return false;
    auto my_val = x.begin();
    for (auto&& val : other) {
        if (*my_val != val) return false;
        my_val++;
    }
    return true;
}*/

template <class T>
vector<T> get_vector(const pmt& p) {
    if (p.data_type() == vector<T>::data_type())
        return vector<T>(p);
    // This error message stinks.  Fix it.
    throw std::runtime_error("Can't convert pmt to this type");
}

template <class T>
std::vector<T> get_std_vector(const pmt& p) {
    auto vec = get_vector<T>(p);
    return std::vector(vec.begin(), vec.end());
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
func(std::complex<double>)

#define VectorPmt(T) \
template <> inline pmt::pmt<std::vector<T>>(const std::vector<T>& x) \
    { *this = vector<T>(x).get_pmt_buffer(); }
Apply(VectorPmt)
#undef VectorPmt
#undef Apply
} // namespace pmtf
