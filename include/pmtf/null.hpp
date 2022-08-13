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
#include <ostream>
#include <memory>
#include <type_traits>


namespace pmtf {

class null {
public:
    using traits = PmtNull::Traits;
    using type = typename traits::type;
    using value_type = void;
    null() { _Create(); }
    template <class U, typename = IsPmt<U>>
    null(const U& other) {
        if (other.data_type() != data_type())
            throw ConversionError(other, "null");
        _buf = other;
    }
    ~null() {}
    static constexpr Data data_type() { return DataTraits<type>::enum_value; }
    const pmt& get_pmt_buffer() const { return _buf; }

    //! Equality Comparisons
    // Declared as class members so that we don't do implicit conversions.
    template <class U>
    bool operator==(const U& x) const;
    template <class U>
    bool operator!=(const U& x) const { return !(operator==(x));}
private:
    void _Create() {
        flatbuffers::FlatBufferBuilder fbb(128);
        fbb.ForceDefaults(true);
        auto offset = traits::Create(fbb).Union();
        auto pmt = CreatePmt(fbb, data_type(), offset);
        fbb.FinishSizePrefixed(pmt);
        _get_buf() = std::make_shared<base_buffer>(fbb.Release());
    }
    std::shared_ptr<base_buffer>& _get_buf() { return _buf._scalar; }
    const std::shared_ptr<base_buffer> _get_buf() const { return _buf._scalar; }
    pmt _buf;
    
};

template <class U>
using IsNotPmtNull = std::enable_if_t<!std::is_same_v<null, U>, bool>;

template <> inline pmt::pmt(const null& x)
    { operator=(x.get_pmt_buffer()); }


// The catch all case.
template <class U>
bool null::operator==(const U& y) const {
    // U is a plain old data type (scalar<float> == float)
    if constexpr(std::is_same_v<null, U> || std::is_same_v<std::nullptr_t, U>)
        return true;
    else if constexpr(std::is_same_v<U, pmt>)
        return y.data_type() == data_type();
    return false;
}

// Reversed case.  This allows for x == y and y == x
template <class U, IsNotPmtNull<U> = true>
bool operator==(const U& y, const null& x) {
    return x.operator==(y);
}

// Reversed Not equal operator
template <class U, IsNotPmtNull<U> = true>
bool operator!=(const U& y, const null& x) {
    return operator!=(x,y);
}


inline std::ostream& operator<<(std::ostream& os, const null& x) {
    os << "null";
    return os;
}

// Declare a static const null_pmt so we can reuse it.
static const null null_pmt;

}

