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

#include <pmtf/base.hpp>
#include <pmtf/wrap.hpp>

// What if this was just a map?
// Then I wouldn't have a serialize function in it. and it wouldn't be derived from base.
/*
What should the class hierarchy be???
Presently we have a base and then several classes dervice from that.
I then define wrappers around pointers to those classes that make it 
easy to work with them.
Can I just cut out the middle man and have the wrapper class be the main class?
Then we don't need all of the static make functions.  It handles all of that for
us.  Can I do this in a useful way?
So I have a pmt base class and derive from that scalar, uniform vector, vector, and map.
In the scalar case and uniform vector case I can just store it.  The vector would need to store
variants or pointers.
1) pmt is pointer, classes are wrappers to make it convenient.
    Need one class and one wrapper for each type
2) pmt is class with data.  Polymorphism doesn't buy me anything here, because I am avoiding creating
    pointers.  I have to use variants.

Let's start with polymorphism.
I need the following set of classes.
scalar
    uniform_vector
    vector
    map
I need a wrapper class for each one.
I need a generator class that can produce any one of them.

*/

namespace pmtf {


template <class T>
class map_value : public base
{
public:
    using map_type = std::map<T, wrap>;
    typedef std::shared_ptr<map_value> sptr;
    static sptr make(const map_type& val)
    {
        return std::make_shared<map_value<T>>(val);
    }
    static sptr from_buffer(const uint8_t* buf, size_t size)
    {
        return std::make_shared<map_value<T>>(buf, size);
    }
    // static sptr from_pmt(const pmtf::Pmt* fb_pmt)
    // {
    //     return std::make_shared<map_value<T>>(fb_pmt);
    // }

    /**************************************************************************
    * Constructors
    **************************************************************************/

    map_value();
    map_value(const map_type& val);
    map_value(const map_value& val);
    map_value(const uint8_t* buf, size_t size);
    // map_value(const pmtf::Pmt* fb_pmt);

    flatbuffers::Offset<void> rebuild_data(flatbuffers::FlatBufferBuilder& fbb);

    void print(std::ostream& os) const {
        os << "{";
        for (const auto& [k, v]: _map) {
            os << k << ": " << v << ", "; 
        }
        os << "}";
    } 

    map_type& value() { return _map; }
private:
    // This stores the actual data.
    map_type _map;

    void fill_flatbuffer();
    virtual void serialize_setup();


};

template <class T>
class map {
public:
    using map_type = std::map<T, wrap>;
    using key_type = T;
    using mapped_type = wrap;
    using value_type = std::pair<const key_type, mapped_type>;
    using reference = value_type&;
    using const_reference = const value_type&;
    
    using sptr = typename map_value<T>::sptr;
    map() : d_ptr(map_value<T>::make({})) {}
    //! Construct a map from a std::map
    map(const std::map<T,wrap>& val): d_ptr(map_value<T>::make(val)) {}
    //! Construct a map from a map_value pointer.
    map(sptr ptr):
        d_ptr(ptr) {}
    //! Copy constructor.
    map(const map<T>& x):
        d_ptr(x.d_ptr) {}
   
    //! Get at the smart pointer.
    sptr ptr() const { return d_ptr; }
    // bool operator==(const map<T>& val) const { return *d_ptr == *val.d_ptr; }
    auto data_type() { return d_ptr->data_type(); }
    auto value() const { return d_ptr->value(); }
   
    /**************************************************************************
    * Copy Assignment
    **************************************************************************/
    map& operator=(const map& other);
    map& operator=(map&& other) noexcept;

    /**************************************************************************
    * Element Access
    **************************************************************************/
    mapped_type& at(const key_type& key);
    const mapped_type& at(const key_type& key ) const;
    mapped_type& operator[]( const key_type& key);

    /**************************************************************************
    * Iterators
    **************************************************************************/
    typename map_type::iterator begin() noexcept { return d_ptr->value().begin(); }
    typename map_type::const_iterator begin() const noexcept { return d_ptr->value().begin(); }
    //typename std::map<T, pmt_sptr>::const_iterator begin() const noexcept { return _map.begin(); }
    typename map_type::iterator end() noexcept { return d_ptr->value().end(); }
    typename map_type::const_iterator end() const noexcept { return d_ptr->value().end(); }
    //typename const std::map<T, pmt_sptr>::iterator end() const noexcept { return _map.end(); }

    /**************************************************************************
    * Capacity
    **************************************************************************/
    bool empty() const noexcept { return d_ptr->value().empty(); }
    size_t size() const noexcept { return d_ptr->value().size(); }
    size_t max_size() const noexcept { return d_ptr->value().max_size(); }

    /**************************************************************************
    * Modifiers
    **************************************************************************/
    

private:
    sptr d_ptr;
};


template<class T>
map<T> get_map(const wrap& x) {
    // Make sure that this is the right type.
    switch(x.ptr()->data_type()) {
        case Data::MapString :
             return map<T>(std::dynamic_pointer_cast<map_value<T>>(x.ptr()));
        default:
            throw std::runtime_error("Cannot convert non string keyed pmt map.");
    }
}

template <class T>
auto get_map_value(const wrap& x) {
    return get_map<T>(x).ptr()->value();
}

/*map<std::string> get_map(const wrap& x) {
    if (x.ptr()->data_type() == Data::PmtMap)
        return map<std::string>(std::dynamic_pointer_cast<map<std::string>>(x.ptr()));
    else
        throw std::runtime_error("Cannot convert to map");
}*/

}
