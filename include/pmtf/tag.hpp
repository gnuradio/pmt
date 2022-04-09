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
#include <pmtf/map.hpp>

/* How do we do this???
  We have two cases that I think cover everything.
1) Fixed length vector of pmts.  This is really easy.
    I just remove the ability to resize a pmt vector after construction.
    There is a problem.  How do conversions and equality work?
    Is a fixed vector == to a resizable vector?
    Are they convertible?? (Yes)
2) A tuple.  The idea is that I want a combination of a set of scalar data, a vector, and a dict.
    How do we specify it.
    A tag is a start, duration, and dictionary.  Or it could be start, duration, key, pmt (vector len 1)
    A pmt is a data vector and a dictionary.
    I could specify it as scalar tuple, has vec, has dict
      Or as tuple<start, dur, dict>.  How would I specify dict in there?
      I could do it from an initializer list or a pmt map or a std map.  That doesn't make for a solid tuple.
      Also a map or a vector isn't stored directly in there.
      How about <start, dur, has_vector, has_map>.  Need to know if it is a fixed length pmt vector as well.
      
*/
namespace pmtf {

class tag : public map {
public:
    using traits = Tag::Traits;
    using type = typename traits::type;
    // Construct empty map
    tag() {
        _MakeEmptyTag(0, 0);
    }
    tag(uint64_t start, uint64_t duration, const std::string_view& key, const pmt& value) {
        _MakeEmptyTag(start, duration);
        this->operator[](std::string(key)) = value;
    }
    tag(uint64_t start, uint64_t duration, const map& other) {
        _MakeEmptyTag(start, duration);
        for (auto& [k, v]: other)
            this->operator[](k) = v;

    }
    template <class T, typename = IsPmt<T>>
    tag(const T& other) {
        if (other.data_type() != data_type())
            throw ConversionError(other, "tag");
        _map = other;
    }
    tag(uint64_t start, uint64_t duration, std::initializer_list<value_type> il) {
        _MakeEmptyTag(start, duration);
        for (auto& [k, v]: il)
            this->operator[](k) = v;
    }
    ~tag() {}
    tag& operator=(const tag& value) {
        std::shared_ptr<base_buffer> scalar = _get_header();
        _MakeEmptyTag(value.start(), value.duration());
        for (auto& [k, v]: value)
            this->operator[](k) = v;
        return *this;
    }

    // Accessor functions
    uint64_t start() const { return _get_header()->data_as<type>()->start(); }
    uint64_t duration() const { return _get_header()->data_as<type>()->duration(); }
    uint64_t map_size() const { return _get_map()->size(); }


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
private:
    std::shared_ptr<map_type> _get_map() { return _map._map; }
    const std::shared_ptr<map_type> _get_map() const { return _map._map; }
    std::shared_ptr<base_buffer> _get_header() { return _map._scalar; }
    const std::shared_ptr<base_buffer> _get_header() const { return _map._scalar; }
    //void _MakeTag(std::tuple<uint64_t, uint64_t> )
    void _MakeEmptyTag(uint64_t start, uint64_t duration) {
        flatbuffers::FlatBufferBuilder fbb;
        fbb.ForceDefaults(true);
        auto offset = traits::Create(fbb, start, duration, 0).Union();
        auto pmt = CreatePmt(fbb, data_type(), offset);
        fbb.FinishSizePrefixed(pmt);
        _map._scalar = std::make_shared<base_buffer>(fbb.Release());
        _map._map = std::make_shared<std::map<std::string, pmtf::pmt>>();
    }
};
    //virtual void serialize_setup();
template <class T>
using IsTag = std::enable_if_t<std::is_same_v<tag, T>, bool>;
template <class T>
using IsNotTag = std::enable_if_t<!std::is_same_v<tag, T>, bool>;

template <> inline pmt::pmt<tag>(const tag& x) { *this = x.get_pmt_buffer(); }

template <class T>
bool tag::operator==(const T& other) const {
    // Add in a tuple case
    if constexpr(std::is_same_v<T, tag>) {
        if (start() != other.start()) return false;
        if (duration() != other.duration()) return false;
        if (map_size() != other.map_size()) return false;
        for (const auto& [k, v]: *this) {
            if (other.count(k) == 0) return false;
            else if (!(other.at(k) == v)) return false;
        }
        return true;
    } else return false;
}

// Reversed case.  This allows for x == y and y == x
template <class T, class U, IsTag<T> = true, IsNotTag<U> = true>
bool operator==(const U& y, const T& x) {
    return x.operator==(y);
}

// Reversed Not equal operator
template <class T, class U, IsTag<T> = true, IsNotTag<U> = true>
bool operator!=(const U& y, const T& x) {
    return operator!=(x,y);
}

// Need to have map operator here because it has pmts in it.
template <class T, IsTag<T> = true>
std::ostream& operator<<(std::ostream& os, const T& value) {
    os << "sample: " << value.start() << " duration: " << value.duration() << " { ";
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
