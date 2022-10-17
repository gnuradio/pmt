/*-*-c++-*-*/
/*
 * Copyright 2021 John Sallay
 *
 * SPDX-License-Identifier: LGPL-3.0
 *
 */
#pragma once

#include <pmtv/type_helpers.hpp>
#include <map>
#include <vector>
#include <variant>
#include <ranges>

namespace pmtv {

class pmt {
public:
    // Constructors
    /*!
    * Default constructor
    *
    * Initialize everything to nullptrs (not null pmt)
    */
    pmt(): _value(nullptr) {}
    pmt(const pmt& other) = default;
    
    template <class T>
    pmt(const T& other);
    // Probably only useful for strings
    template <class T>
    pmt(const T* other);
    pmt& operator=(const pmt& other) = default;
    template <class T>
    pmt& operator=(const T& other);
    std::string type_name() const noexcept { 
        return std::visit([](const auto& arg) -> std::string {
            using T = std::decay_t<decltype(arg)>;
            return type_string<T>(); }
             , _value.base()); }
    //template <class T> pmt(const T& x);

    /*size_t serialize(std::streambuf& sb) const;
    static pmt deserialize(std::streambuf& sb);

    std::string to_base64();
    static pmtv::pmt from_base64(const std::string& encoded_str);
    Data data_type() const;
    size_t elements() const;
    size_t bytes_per_element() const {
*/
    //! Equality Comparisons
    // Declared as class members so that we don't do implicit conversions.
    template <class T>
    bool operator==(const T& other) const;
    // Allows us to cast from a pmt like this: float x = float(mypmt);
    // Must be explicit.
    template <class T>
    explicit operator T() const {
        return std::visit([](const auto& arg) -> T {
            using U = std::decay_t<decltype(arg)>;
            if constexpr(std::constructible_from<T, U>) return T(arg);
        }, _value.base()); }


private:
    _pmt_storage _value;
};

template <class T>
pmt::pmt(const T& other) {
    if constexpr(Scalar<T>) _value = other;
    else if constexpr(std::is_same_v<T, std::string>)
        _value = other;
    else if constexpr(std::is_same_v<T, std::map>)
        _value = std::make_shared<std::vector<typename T::value_type>>(other.begin(), other.end());
    else if constexpr(std::ranges::view<T>) 
        _value = std::make_shared<std::vector<typename T::value_type>>(other.begin(), other.end());
    else _value = nullptr;
}

template <class T, class U>
bool PmtEqual(const T& arg, const U& other) {
    if constexpr(Scalar<T> && std::is_convertible_v<T,U>) {
        // Make sure we don't mess up signs or floating point
        // We don't want uint8_t(255) == int8_t(-1)
        // Or float(1.5) == int(1)
        if constexpr(std::same_as<T, U>) return arg == other;
        if constexpr(std::signed_integral<T> && std::unsigned_integral<U>)
            return (arg > 0) && (U(arg) == other);
        else if constexpr(std::unsigned_integral<T> && std::signed_integral<U>)
            return (other > 0) && (T(other) == arg);
        else if constexpr(Complex<T> && Complex<U>)
            return std::complex<double>(arg) == std::complex<double>(other);
        else if constexpr(Complex<T> && ! Complex<U>) return false;
        else if constexpr(Complex<U> && ! Complex<T>) return false;
        else return arg == other;
    }
    /*if constexpr(std::is_convertible_v<T, U>) return arg == other;
    else if constexpr(std::ranges::view<T> && std::ranges::view<U>) {
        if (std::is_same_v<typename T::value_type, typename U::value_type>) {
            if (arg.size() == other.size()) return std::equal(arg.begin(), arg.end(), other.begin());
            else return false;
        }
    }*/
    return false;
    
}

template <class U>
bool pmt::operator==(const U& other) const {
    return std::visit([other](const auto& arg) -> bool {
        using T = std::decay_t<decltype(arg)>;
        using Ud = std::decay_t<U>;
        return PmtEqual<T,Ud>(arg, other); }
         , _value.base());
}
/*template <class T>
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
};*/

    
/*!
Pmt class is a collection of base_buffers.  This makes it easy for us to work
with collections of pmts like maps and vectors.
*/
/*class pmt {
public:
    // Constructors
    /*!
    * Default constructor
    *
    * Initialize everything to nullptrs (not null pmt)
    */
    /*pmt();
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
    _pmt_value _value;

    size_t serialize(std::streambuf& sb) const;
    static pmt deserialize(std::streambuf& sb);

    std::string to_base64();
    static pmtf::pmt from_base64(const std::string& encoded_str);
    bool empty() { return !(_scalar || _vector || _map); }
    Data data_type() const;
    std::string type_string() const noexcept;
    size_t elements() const;
    size_t bytes_per_element() const;

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
    /*pmt(const std::shared_ptr<base_buffer>& other);
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
}*/


} // namespace pmtv
