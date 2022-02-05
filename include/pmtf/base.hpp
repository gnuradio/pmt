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
*/
struct pmt {
public:
    // Constructors
    /*!
    * Default constructor
    *
    * Initialize everything to nullptrs (not null pmt)
    */
    pmt(): _scalar(nullptr), _vector(nullptr), _map(nullptr) {}
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


} // namespace pmtf
