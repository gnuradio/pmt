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
#include <initializer_list>

#include <pmtf/base.hpp>

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

class map {
public:
    using traits = MapHeaderString::Traits;
    using entryTraits = MapEntryString::Traits;
    using type = typename traits::type;

    using key_type = std::string;
    using mapped_type = pmt;
    using value_type = std::pair<const key_type, mapped_type>;
    using reference = value_type&;
    using const_reference = const value_type&;
    using map_type = std::map<key_type, mapped_type>;
    using size_type = size_t;

    using iterator = map_type::iterator;
    using const_iterator = map_type::const_iterator;

    // Construct empty map
    map() {
        _MakeEmptyMap();
    }
    // Copy from std map
    map(const std::map<std::string, pmt>& other) {
        _MakeEmptyMap();
        for (auto& [k, v]: other)
            this->operator[](k) = v;
    }
    template <class T, typename = IsPmt<T>>
    map(const T& other) {
        if (other.data_type() != data_type())
            throw ConversionError(other, "map");
        _map = other;
    }
    map(std::initializer_list<value_type> il) {
        _MakeEmptyMap();
        for (auto& [k, v]: il)
            this->operator[](k) = v;
    }
    //template <class T>
    //map(std::map<string
    ~map() {}

    /**************************************************************************
    * Iterators
    **************************************************************************/
    typename map_type::iterator begin() noexcept { return _get_map()->begin(); }
    typename map_type::const_iterator begin() const noexcept { return _get_map()->begin(); }
    typename map_type::iterator end() noexcept { return _get_map()->end(); }
    typename map_type::const_iterator end() const noexcept { return _get_map()->end(); }

    /**************************************************************************
    * Element Access
    **************************************************************************/
    mapped_type& at(const key_type& key) { return _get_map()->at(key); }
    const mapped_type& at(const key_type& key ) const { return _get_map()->at(key); }
    mapped_type& operator[]( const key_type& key) {
        return _get_map()->operator[](key);
    }

    size_t size() const { return _get_map()->size(); }
    size_t count(const key_type& key) const { return _get_map()->count(key); }

    static constexpr Data data_type() { return DataTraits<type>::enum_value; }
    const pmt& get_pmt_buffer() const { return _map; }

    //! Equality Comparisons
    // Declared as class members so that we don't do implicit conversions.
    template <class U>
    bool operator==(const U& x) const;
    template <class U>
    bool operator!=(const U& x) const { return !(operator==(x));}
    void pre_serial_update() const {
        // It may look odd to declare this function as const when it modifies
        // count.  But count is part of the internal interface, so to the
        // user, this is a const function.
        std::shared_ptr<base_buffer> scalar = _map._scalar;
        scalar->data_as<type>()->mutate_count(_get_map()->size());
    }
protected:
    std::shared_ptr<map_type> _get_map() { return _map._map; }
    const std::shared_ptr<map_type> _get_map() const { return _map._map; }
    std::shared_ptr<base_buffer> _get_header() { return _map._scalar; }
    const std::shared_ptr<base_buffer> _get_header() const { return _map._scalar; }
    // This stores the actual data.
    pmt _map;
    void _MakeEmptyMap() {
        flatbuffers::FlatBufferBuilder fbb;
        fbb.ForceDefaults(true);
        auto offset = traits::Create(fbb, 0).Union();
        auto pmt = CreatePmt(fbb, data_type(), offset);
        fbb.FinishSizePrefixed(pmt);
        _map._scalar = std::make_shared<base_buffer>(fbb.Release());
        _map._map = std::make_shared<std::map<std::string, pmtf::pmt>>();
    }
};
    //virtual void serialize_setup();
template <class T>
using IsMap = std::enable_if_t<std::is_same_v<map, T>, bool>;
template <class T>
using IsNotMap = std::enable_if_t<!std::is_same_v<map, T>, bool>;

template <> inline pmt::pmt(const map& x) { *this = x.get_pmt_buffer(); }

template <> inline pmt::pmt(const std::map<std::string, pmt>& x) {
    *this = map(x).get_pmt_buffer();
}

template <class T>
bool map::operator==(const T& other) const {
    if constexpr(is_map_like_container<T>::value) {
        if (size() != other.size()) return false;
        for (const auto& [k, v]: *this) {
            if (other.count(k) == 0) return false;
            else if (!(other.at(k) == v)) return false;
        }
        return true;
    } else if constexpr(std::is_same_v<T, pmt>) {
        return other.operator==(*this);
    } else
        return false;
}

// Reversed case.  This allows for x == y and y == x
template <class T, class U, IsMap<T> = true, IsNotMap<U> = true>
bool operator==(const U& y, const T& x) {
    return x.operator==(y);
}

// Reversed Not equal operator
template <class T, class U, IsMap<T> = true, IsNotMap<U> = true>
bool operator!=(const U& y, const T& x) {
    return operator!=(x,y);
}

// Need to have map operator here because it has pmts in it.
template <class T, IsMap<T> = true>
std::ostream& operator<<(std::ostream& os, const T& value) {
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

}
