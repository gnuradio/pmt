/*-*-c++-*-*/
/*
 * Copyright 2021 John Sallay
 *
 * SPDX-License-Identifier: LGPL-3.0
 *
 */
#pragma once

#include <pmtf/pmtf_generated.h>
#include <pmtf/type_helpers.hpp>
#include <map>
#include <vector>

namespace pmtf {
/*!
Class is a wrapper around a flatbuffers buffer that contains a Pmt.
It offers several convenience functions.
*/

template <class T>
class BaseConversionError;

class base_buffer {
public:
    base_buffer() {}
    base_buffer(flatbuffers::DetachedBuffer&& buf): _buf(std::move(buf)) {}
    Data data_type() const { return data()->data_type(); }
    const Pmt* data() const { return GetSizePrefixedPmt(_buf.data()); }
    Pmt* data() { return const_cast<Pmt*>(GetSizePrefixedPmt(_buf.data())); }
    template <class type>
    const type* data_as() const {
        auto ptr = data()->data_as<type>();
        if (ptr == nullptr) throw BaseConversionError<type>(*this);
        return ptr;
    }
    template <class type>
    type* data_as() {
        auto ptr = const_cast<type*>(data()->data_as<type>());
        if (ptr == nullptr) throw BaseConversionError<type>(*this);
        return ptr;
    }
    size_t size() { return _buf.size(); }
    const uint8_t* raw() const { return _buf.data(); }
private:
    flatbuffers::DetachedBuffer _buf;

};

template <class T>
class BaseConversionError: public std::exception {
public:
    BaseConversionError(const base_buffer& buf) {
        _msg = "Can't convert base_buffer of type " + std::string(EnumNameData(buf.data_type())) + " to " + std::string(EnumNameData(DataTraits<T>::enum_value));
    }

    const char* what() const noexcept {
        return _msg.c_str();
    }
private:
    std::string _msg;
};
/*!
Pmt class is a collection of base_buffers.  This makes it easy for us to work
with collections of pmts like maps and vectors.
*/
struct pmt {
public:
    // Constructors
    /*!
    * Default constructor
    *
    * Initialize everything to nullptrs (not null pmt)
    */
    pmt();
    pmt(const pmt& other);
    
    template <class T>
    pmt(const T& other);
    // Probably only useful for strings
    template <class T>
    pmt(const T* other);
    pmt& operator=(const pmt& other);
    template <class T>
    pmt& operator=(const T& other);
    //template <class T> pmt(const T& x);
    std::shared_ptr<base_buffer> _scalar;
    std::shared_ptr<std::vector<pmt>> _vector;
    std::shared_ptr<std::map<std::string, pmt>> _map;

    size_t serialize(std::streambuf& sb) const;
    static pmt deserialize(std::streambuf& sb);

    std::string to_base64();
    static pmtf::pmt from_base64(const std::string& encoded_str);
    bool empty() { return !(_scalar || _vector || _map); }
    Data data_type() const;
    std::string type_string() const noexcept;

    //! Equality Comparisons
    // Declared as class members so that we don't do implicit conversions.
    template <class T>
    bool operator==(const T& other) const;
    template <class T>
    bool operator!=(const T& x) const { return !(operator==(x));}

private:
    void pre_serial_update() const;
    /*!
    * Initialize from a shared_ptr to a base_buffer
    * Used in deserialize.  Shouldn't be used elsewhere.
    */
    pmt(const std::shared_ptr<base_buffer>& other);
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

template <class T, IsPmt<T> = true>
std::ostream& operator<<(std::ostream& os, const T& value);

// Reversed case.  This allows for x == y and y == x
template <class T, class U, IsPmt<T> = true, IsNotPmt<U> = true, IsNotPmtDerived<U> = true>
bool operator==(const U& y, const T& x) {
    return x.operator==(y);
}

// Reversed Not equal operator
template <class T, class U, IsPmt<T> = true, IsNotPmt<U> = true, IsNotPmtDerived<U> = true>
bool operator!=(const U& y, const T& x) {
    return operator!=(x,y);
}


} // namespace pmtf
