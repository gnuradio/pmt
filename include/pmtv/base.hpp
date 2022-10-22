/*-*-c++-*-*/
/*
 * Copyright 2021 John Sallay
 *
 * SPDX-License-Identifier: LGPL-3.0
 *
 */
#pragma once

#include <pmtv/type_helpers.hpp>
#include <pmtv/version.hpp>
#include <map>
#include <vector>
#include <variant>
#include <ranges>
#include "base64/base64.h"

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
    pmt& operator=(const T& other)
    {
        _value = other;
        return *this;
    }
    std::string type_name() const noexcept { 
        return std::visit([](const auto& arg) -> std::string {
            using T = std::decay_t<decltype(arg)>;
            return type_string<T>(); }
             , _value.base()); }
    //template <class T> pmt(const T& x);

    // template <class T>
    size_t serialize(std::streambuf& sb) const;
    static pmt deserialize(std::streambuf& sb);

    
    std::string to_base64() const;
    static pmtv::pmt from_base64(const std::string& encoded_str);
    // Data data_type() const;
    // size_t elements() const;
    // size_t bytes_per_element() const {

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
            else throw std::runtime_error("Invalid PMT Cast");
        }, _value.base()); }

protected:
    _pmt_storage _value;
};

template <class T>
pmt::pmt(const T& other) {
    if constexpr(Scalar<T>) { 
        _value = other;
    }
    else if constexpr(std::is_same_v<T, std::string>) {
        _value = other;
    }
    // else if constexpr(std::is_same_v<T, std::map>) {
    //     _value = std::make_shared<std::vector<typename T::value_type>>(other.begin(), other.end());
    // }
    else if constexpr(UniformVector<T>) {
        _value = std::make_shared<std::vector<typename T::value_type>>(other.begin(), other.end());
    }
    else {
        _value = nullptr;
    } 
}

template <class T, class U>
bool PmtEqual(const T& arg, const U& other) {

    if constexpr(PmtNull<T> && std::same_as<T, U>) {
        return true;
    }
    else if constexpr(std::is_same_v<U, pmt>) {
        // If we are comparing to a pmt, another call to ==
        // will peel the value out
        
        return arg == other;;
    }
    else if constexpr(Scalar<T> && std::is_convertible_v<T,U>) {
        // Make sure we don't mess up signs or floating point
        // We don't want uint8_t(255) == int8_t(-1)
        // Or float(1.5) == int(1)
        if constexpr(std::same_as<T, U>) {
            return arg == other;
        }
        if constexpr(std::signed_integral<T> && std::unsigned_integral<U>){
            return (arg > 0) && (U(arg) == other);
        }
        else if constexpr(std::unsigned_integral<T> && std::signed_integral<U>){
            return (other > 0) && (T(other) == arg);
        }
        else if constexpr(Complex<T> && Complex<U>) {
            return std::complex<double>(arg) == std::complex<double>(other);
        }
        else if constexpr(Complex<T> && ! Complex<U>) {
            return false;
        }
        else if constexpr(Complex<U> && ! Complex<T>) { 
            return false;
        }
        else { 
            return arg == other;
        }
    }
    else if constexpr(IsSharedPtr<T> && UniformVector<U>) {
        return std::visit([&arg, &other]() -> bool {
            return PmtEqual(*arg, other); }
            );
    }
    else if constexpr(UniformVector<T> && UniformVector<U>) {
        if constexpr(std::is_same_v<T, U>) {
            if (arg.size() == other.size()) {
                return std::equal(arg.begin(), arg.end(), other.begin());
            }
            else 
            {
                return false;
            }
        }
        else {
            return false;
        }
    }
    else {
        std::cerr << typeid(T).name() << " " << typeid(U).name() << std::endl;
        return false;
    }
    // else if constexpr(std::is_convertible_v<T, U>) return arg == other;
    // else if constexpr(std::ranges::view<T> && std::ranges::view<U>) {
    //     if (std::is_same_v<typename T::value_type, typename U::value_type>) {
    //         if (arg.size() == other.size()) return std::equal(arg.begin(), arg.end(), other.begin());
    //         else return false;
    //     }
    // }
    
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


size_t pmt::serialize(std::streambuf& sb) const {
    size_t length = 0;
    length += sb.sputn(reinterpret_cast<const char*>(&pmt_version), 2);
    
    pmt_container_type container;
    std::visit([&container](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        container = container_type<T>();        
    }, _value.base());

    length += sb.sputn(reinterpret_cast<const char*>(&container), 2);

    if (container == pmt_container_type::EMPTY) {
        // do nothing
    }
    else if (container == pmt_container_type::SCALAR) {

        std::visit([&length, &sb](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            auto id = element_type<T>();
            length += sb.sputn(reinterpret_cast<const char*>(&id), 1);
            auto v = arg;
            length += sb.sputn(reinterpret_cast<const char*>(&v), sizeof(v));
            
        }, _value.base());
    
    }
    else if (container == pmt_container_type::UNIFORM_VECTOR) {
        std::visit([&length, &sb](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr(UniformVector<T>) {
                auto id = element_type<T>();
                length += sb.sputn(reinterpret_cast<const char*>(&id), 1);
                length += sb.sputn(reinterpret_cast<const char*>(arg.data()), arg.size()*sizeof(arg[0]));
            }
        }, _value.base());
    
    }

    return length;
}

pmt pmt::deserialize(std::streambuf& sb)
{
    uint16_t version;
    pmt_container_type container;
    sb.sgetn(reinterpret_cast<char*>(&version), sizeof(version));
    sb.sgetn(reinterpret_cast<char*>(&container), sizeof(container));

    pmt ret;
    if (container == pmt_container_type::EMPTY) {
        // do nothing
    }
    else if (container == pmt_container_type::SCALAR) {
        pmt_element_type T_type;
        sb.sgetn(reinterpret_cast<char*>(&T_type), sizeof(T_type));

        switch(T_type) {
            case pmt_element_type::UINT8: {
                uint8_t val;
                sb.sgetn(reinterpret_cast<char*>(&val), sizeof(val));
                ret = val;
            } break;
            case pmt_element_type::UINT16: {
                uint16_t val;
                sb.sgetn(reinterpret_cast<char*>(&val), sizeof(val));
                ret = val;
            } break;
            case pmt_element_type::UINT32: {
                uint32_t val;
                sb.sgetn(reinterpret_cast<char*>(&val), sizeof(val));
                ret = val;
            } break;
            case pmt_element_type::UINT64: {
                uint64_t val;
                sb.sgetn(reinterpret_cast<char*>(&val), sizeof(val));
                ret = val;
            } break;
            case pmt_element_type::INT8: {
                int8_t val;
                sb.sgetn(reinterpret_cast<char*>(&val), sizeof(val));
                ret = val;
            } break;
            case pmt_element_type::INT16: {
                int16_t val;
                sb.sgetn(reinterpret_cast<char*>(&val), sizeof(val));
                ret = val;
            } break;
            case pmt_element_type::INT32: {
                int32_t val;
                sb.sgetn(reinterpret_cast<char*>(&val), sizeof(val));
                ret = val;
            } break;
            case pmt_element_type::INT64: {
                int64_t val;
                sb.sgetn(reinterpret_cast<char*>(&val), sizeof(val));
                ret = val;
            } break;
            case pmt_element_type::FLOAT: {
                float val;
                sb.sgetn(reinterpret_cast<char*>(&val), sizeof(val));
                ret = val;
            } break;
            case pmt_element_type::DOUBLE: {
                double val;
                sb.sgetn(reinterpret_cast<char*>(&val), sizeof(val));
                ret = val;
            } break;
            case pmt_element_type::COMPLEX_FLOAT: {
                std::complex<float> val;
                sb.sgetn(reinterpret_cast<char*>(&val), sizeof(val));
                ret = val;
            } break;
            case pmt_element_type::COMPLEX_DOUBLE: {
                std::complex<double> val;
                sb.sgetn(reinterpret_cast<char*>(&val), sizeof(val));
                ret = val;
            }
            default:{

            }
        }
    }
    else if (container == pmt_container_type::UNIFORM_VECTOR) {
    
    }

    return ret;
}

std::string pmt::to_base64() const
{
    std::stringbuf sb; 
    auto nbytes = serialize(sb);
    std::string pre_encoded_str(nbytes, '0');
    sb.sgetn(pre_encoded_str.data(), nbytes);
    auto nencoded_bytes = Base64encode_len(nbytes);
    std::string encoded_str(nencoded_bytes, '0');
    auto nencoded = Base64encode(encoded_str.data(), pre_encoded_str.data(), nbytes);
    encoded_str.resize(nencoded - 1); // because it null terminates
    return encoded_str;
}

pmt pmt::from_base64(const std::string& encoded_str)
{
    std::string bufplain(encoded_str.size(), '0');
    Base64decode(bufplain.data(), encoded_str.data());
    std::stringbuf sb(bufplain);
    return deserialize(sb); 
}


} // namespace pmtv
