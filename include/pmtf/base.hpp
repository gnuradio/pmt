/*-*-c++-*-*/
/*
 * Copyright 2021 John Sallay
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
        _map = other._map;
    }
    template <class T>
    pmt(const T& other);
    pmt& operator=(const pmt& other) {
        _scalar = other._scalar;
        _map = other._map;
        return *this;
    }
    template <class T>
    pmt& operator=(const T& other);
    //template <class T> pmt(const T& x);
    std::shared_ptr<base_buffer> _scalar;
    std::shared_ptr<std::map<std::string, pmt>> _map;

    size_t serialize(std::streambuf& sb) const {
        std::cout << "Write: " << _scalar->size() << std::endl;
        size_t length = 0;
        length += sb.sputn(reinterpret_cast<const char*>(_scalar->raw()), _scalar->size());
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
        std::cout << size << std::endl;
        char* x = new char[size + sizeof(uint32_t)];
        *reinterpret_cast<uint32_t*>(x) = size;
        sb.sgetn(x + sizeof(uint32_t), size);
        // This will not free when done...
        flatbuffers::DetachedBuffer buf(nullptr, false, nullptr, 0, reinterpret_cast<uint8_t*>(x), size);
        pmt cur(std::make_shared<base_buffer>(std::move(buf)));
        if (cur.data_type() == Data::MapHeaderString) {
            cur._map = std::make_shared<std::map<std::string, pmt>>();
            uint32_t count = cur._scalar->data_as<MapHeaderString>()->count();
            std::cout << "count = " << count << std::endl;
            std::vector<char> data;
            for (size_t i = 0; i < count; i++) {
                // Read length then string
                sb.sgetn(reinterpret_cast<char*>(&size), sizeof(uint32_t));
                data.resize(size);
                sb.sgetn(data.data(), size);
                std::cout << "key = " << std::string(data.begin(), data.end()) << std::endl;
                // Deserialize the pmt map value
                (*cur._map)[std::string(data.begin(), data.end())] = deserialize(sb);
            }
        }
        return cur;
    }

    Data data_type() const {
        if (_scalar != nullptr) {
            return _scalar->data_type();
        }
        throw std::runtime_error("Cannot get data type for unitialized pmt");
    }
};

// Define some SFINAE templates.  Since we can create pmts from the various classes,
// We want to ensure that we really do or do not have one when we call a function.
template <typename T>
using IsPmt = std::enable_if_t<std::is_same_v<T, pmt>, bool>;
template <typename T>
using IsNotPmt = std::enable_if_t<!std::is_same_v<T, pmt>, bool>;



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


} // namespace pmtf
