/*-*-c++-*-*/
/*
 * Copyright 2021 John Sallay
 *
 * SPDX-License-Identifier: LGPL-3.0
 *
 */
#pragma once

#include <pmtf/pmtf_generated.h>
#include <cstdlib>
#include <complex>
#include <iostream>
#include <map>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <vector>

namespace pmtf {
/*!
Class is a wrapper around a flatbuffers buffer that contains a Pmt.
It offers several convenience functions.
*/
class base_buffer {
public:
    base_buffer() {}
    base_buffer(flatbuffers::DetachedBuffer&& buf): _buf(std::move(buf)) {}
    Data data_type() { return data()->data_type(); }
    const Pmt* data() const { return GetSizePrefixedPmt(_buf.data()); }
    Pmt* data() { return const_cast<Pmt*>(GetSizePrefixedPmt(_buf.data())); }
    template <class type>
    const type* data_as() const { return data()->data_as<type>(); }
    template <class type>
    type* data_as() { return const_cast<type*>(data()->data_as<type>()); }
    size_t size() { return _buf.size(); }
    const uint8_t* raw() const { return _buf.data(); }
private:
    flatbuffers::DetachedBuffer _buf;

};

class AlignedAllocator : public flatbuffers::Allocator {
  public:
    uint8_t* allocate(size_t size) FLATBUFFERS_OVERRIDE {
        return reinterpret_cast<uint8_t*>(std::aligned_alloc(64, size));
    }

    void deallocate(uint8_t *p, size_t) FLATBUFFERS_OVERRIDE { free(p); }
    static void dealloc(void *p, size_t) { free(p); }
};

/*!
Pmt class is a collection of base_buffers.  This makes it easy for us to work
with collections of pmts like maps and vectors.

Do I want to have shared pointers to pmts or pmts have shared pointers??
A map needs to have shared pointers to its values.
*/
struct pmt {
public:
    pmt(): _scalar(nullptr), _map(nullptr) {}
    pmt(const std::shared_ptr<base_buffer>& other) {
        _scalar = other;
        _map = nullptr;
    }
    pmt(const pmt& other) {
        _scalar = other._scalar;
        _vector = other._vector;
        _map = other._map;
    }
    template <class T>
    pmt(const T& other);
    // Probably only useful for strings
    template <class T>
    pmt(const T* other);
    pmt& operator=(const pmt& other) {
        _scalar = other._scalar;
        _vector = other._vector;
        _map = other._map;
        return *this;
    }
    template <class T>
    pmt& operator=(const T& other);
    //template <class T> pmt(const T& x);
    std::shared_ptr<base_buffer> _scalar;
    std::shared_ptr<std::vector<pmt>> _vector;
    std::shared_ptr<std::map<std::string, pmt>> _map;

    bool empty() { return !(_scalar || _vector || _map); }

    size_t serialize(std::streambuf& sb) const {
        size_t length = 0;
        length += sb.sputn(reinterpret_cast<const char*>(_scalar->raw()), _scalar->size());
        if (_vector) {
            for (const auto& v: *_vector) {
                length += v.serialize(sb);
            }
        }
        if (_map) {
            uint32_t size;
            for (const auto& [k, v]: *_map) {
                // For right now just prefix the size to the key and send it
                size = k.size();
                length += sb.sputn(reinterpret_cast<const char*>(&size), sizeof(uint32_t));
                length += sb.sputn(k.c_str(), size);
                length += v.serialize(sb);
            }
        }
        return length;
    }
    static pmt deserialize(std::streambuf& sb) {
        uint32_t size;
        sb.sgetn(reinterpret_cast<char*>(&size), sizeof(size));
        AlignedAllocator* aa = new AlignedAllocator;
        char* x = reinterpret_cast<char*>(aa->allocate(size + sizeof(uint32_t)));
        *reinterpret_cast<uint32_t*>(x) = size;
        sb.sgetn(x + sizeof(uint32_t), size);
        flatbuffers::DetachedBuffer buf(aa, true, reinterpret_cast<uint8_t*>(x), size + sizeof(uint32_t), reinterpret_cast<uint8_t*>(x), size+sizeof(uint32_t));
        pmt cur(std::make_shared<base_buffer>(std::move(buf)));
        if (cur.data_type() == Data::VectorPmtHeader) {
            uint32_t count = cur._scalar->data_as<VectorPmtHeader>()->count();
            cur._vector = std::make_shared<std::vector<pmt>>(count);
            for (size_t i = 0; i < count; i++) {
                (*cur._vector)[i] = deserialize(sb);
            }
        } else if (cur.data_type() == Data::MapHeaderString) {
            cur._map = std::make_shared<std::map<std::string, pmt>>();
            uint32_t count = cur._scalar->data_as<MapHeaderString>()->count();
            std::vector<char> data;
            for (size_t i = 0; i < count; i++) {
                // Read length then string
                sb.sgetn(reinterpret_cast<char*>(&size), sizeof(uint32_t));
                data.resize(size);
                sb.sgetn(data.data(), size);
                // Deserialize the pmt map value
                (*cur._map)[std::string(data.begin(), data.end())] = deserialize(sb);
            }
        }
        return cur;
    }

    std::string to_base64();
    static pmtf::pmt from_base64(const std::string& encoded_str);

    Data data_type() const {
        if (_scalar != nullptr) {
            return _scalar->data_type();
        }
        throw std::runtime_error("Cannot get data type for unitialized pmt");
    }

    std::string type_string() const noexcept {
        if (_scalar != nullptr)
            return std::string(EnumNameData(data_type()));
        else return "Uninitialized";
    }
};

class ConversionError: public std::exception {
public:
    ConversionError(const pmt& pmt, const std::string& base_type, const std::string& c_type) {
        _msg = "Can't convert pmt of type " + pmt.type_string() + " to " + base_type + "<" + c_type + ">";
    }
    ConversionError(const pmt& pmt, const std::string& base_type) {
        _msg = "Can't convert pmt of type " + pmt.type_string() + " to " + base_type;
    }

    const char* what() const noexcept {
        return _msg.c_str();
    }
private:
    std::string _msg;
};

// Define some SFINAE templates.  Since we can create pmts from the various classes,
// We want to ensure that we really do or do not have one when we call a function.
template <typename T>
using IsPmt = std::enable_if_t<std::is_same_v<T, pmt>, bool>;
template <typename T>
using IsNotPmt = std::enable_if_t<!std::is_same_v<T, pmt>, bool>;

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

template <typename T>
using IsContainer = std::enable_if_t<is_container<T>::value, bool>;

template <class T>
struct is_complex : std::false_type {};

template <class T>
struct is_complex<std::complex<T>> : std::true_type {};

// We need to know the struct type for complex values
template <class T> struct scalar_type;
template <> struct scalar_type<std::complex<float>> { using type = Complex64; };
template <> struct scalar_type<std::complex<double>> { using type = Complex128; };

template <Data T>
struct cpp_type;

template <class T> inline std::string ctype_string();
template <> inline std::string ctype_string<char>() { return "char"; }
template <> inline std::string ctype_string<uint8_t>() { return "uint8_t"; }
template <> inline std::string ctype_string<uint16_t>() { return "uint16_t"; }
template <> inline std::string ctype_string<uint32_t>() { return "uint32_t"; }
template <> inline std::string ctype_string<uint64_t>() { return "uint64_t"; }
template <> inline std::string ctype_string<int8_t>() { return "int8_t"; }
template <> inline std::string ctype_string<int16_t>() { return "int16_t"; }
template <> inline std::string ctype_string<int32_t>() { return "int32_t"; }
template <> inline std::string ctype_string<int64_t>() { return "int64_t"; }
template <> inline std::string ctype_string<float>() { return "float"; }
template <> inline std::string ctype_string<double>() { return "double"; }
template <> inline std::string ctype_string<std::complex<float>>() { return "complex<float>"; }
template <> inline std::string ctype_string<std::complex<double>>() { return "complex<double>"; }
template <> inline std::string ctype_string<pmt>() { return "pmt"; }

} // namespace pmtf
