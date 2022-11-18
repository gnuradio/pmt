#pragma once


#include <pmtv/type_helpers.hpp>
#include <pmtv/version.hpp>
#include "base64/base64.h"
#include <cstddef>
#include <complex>
#include <ranges>
#include <span>

#include <fmt/format.h>

namespace pmtv {

using pmt = pmt_var_t;

// class pmt : public pmt_var_t {
//     public:
//     pmt(): pmt_var_t(nullptr) {}
//     pmt(const pmt& other) = default;
    
//     template <class T>
//     pmt(const T& other) : pmt_var_t(other) {} 
//     // Probably only useful for strings
//     template <class T>
//     pmt(const T* other);

//     // Forwarding Constructor to wrap pmt_var_t
//     template <class... Args>
//     pmt(Args&&... args)
//         : variant{std::forward<Args>(args)...}
//     {}

//     pmt& operator=(const pmt& other) = default;

//     template <class U>
//     bool operator==(const U& other) const {
//         return std::visit([other](const auto& arg) -> bool {
//             using T = std::decay_t<decltype(arg)>;
//             using Ud = std::decay_t<U>;
//             return PmtEqual<T,Ud>(arg, other); }
//             , get_base());
//     }

//     size_t serialize(std::streambuf& sb) const;
//     static pmt deserialize(std::streambuf& sb);
//     std::string to_base64() const;
//     static pmtv::pmt from_base64(const std::string& encoded_str);



//     // Allows us to cast from a pmt like this: float x = float(mypmt);
//     // Must be explicit.
//     template <class T>
//     explicit operator T() const {
//         return std::visit([](const auto& arg) -> T {
//             using U = std::decay_t<decltype(arg)>;
//             if constexpr(std::constructible_from<T, U>) return T(arg);
//             // else if constexpr (PmtMap<T> && PmtMap<U>) {
//             //     return std::get<std::map<std::string, pmt_var_t>>(arg);                
//             // }
//             else throw std::runtime_error(fmt::format("Invalid PMT Cast {} {}", typeid(T).name(), typeid(U).name()));
//         }, *this); }

// private:
//     template <typename T>
//     static T _deserialize_val(std::streambuf& sb);

//     template <typename T>
//     static pmt _deserialize_vec(std::streambuf& sb, size_t sz);
// };

// template <class T, class U>
// bool PmtEqual(const T& arg, const U& other) {

//     if constexpr(PmtNull<T> && std::same_as<T, U>) {
//         return true;
//     }
//     else if constexpr(std::is_same_v<U, pmt>) {
//         // If we are comparing to a pmt, another call to ==
//         // will peel the value out
        
//         return arg == other;
//     }
//     else if constexpr(Scalar<T> && std::is_convertible_v<T,U>) {
//         // Make sure we don't mess up signs or floating point
//         // We don't want uint8_t(255) == int8_t(-1)
//         // Or float(1.5) == int(1)
//         if constexpr(std::same_as<T, U>) {
//             return arg == other;
//         }
//         if constexpr(std::signed_integral<T> && std::unsigned_integral<U>){
//             return (arg > 0) && (U(arg) == other);
//         }
//         else if constexpr(std::unsigned_integral<T> && std::signed_integral<U>){
//             return (other > 0) && (T(other) == arg);
//         }
//         else if constexpr(Complex<T> && Complex<U>) {
//             return std::complex<double>(arg) == std::complex<double>(other);
//         }
//         else if constexpr(Complex<T> && ! Complex<U>) {
//             return false;
//         }
//         else if constexpr(Complex<U> && ! Complex<T>) { 
//             return false;
//         }
//         else { 
//             return arg == other;
//         }
//     }
//     // else if constexpr(UniformVectorInsidePmt<T> && UniformVectorInsidePmt<U>) {
//     //     return std::visit([&arg, &other]() -> bool {
//     //         return PmtEqual(*arg, *other); }
//     //         );
//     // }
//     // else if constexpr(UniformVectorInsidePmt<T> && UniformVector<U>) {
//     //     return std::visit([&arg, &other]() -> bool {
//     //         return PmtEqual(*arg, other); }
//     //         );
//     // }
//     // else if constexpr(UniformVector<T> && UniformVectorInsidePmt<U>) {
//     //     return std::visit([&arg, &other]() -> bool {
//     //         return PmtEqual(arg, *other); }
//     //         );
//     // }
//     else if constexpr(UniformVector<T> && UniformVector<U>) {
//         // if constexpr(std::is_same_v<T, U>) {
//         if constexpr(std::is_same_v<typename T::value_type, typename U::value_type>) {
//             if (arg.size() == other.size()) {
//                 return std::equal(arg.begin(), arg.end(), other.begin());
//             }
//             else 
//             {
//                 return false;
//             }
//         }
//         else {
//             // std::cerr << typeid(T).name() << " " << typeid(U).name() << std::endl;
//             return PmtEqual(arg, other);
//         }
//     }
//     else {
//         // std::cerr << typeid(T).name() << " " << typeid(U).name() << std::endl;
//         return false;
//     }
//     // else if constexpr(std::is_convertible_v<T, U>) return arg == other;
//     // else if constexpr(std::ranges::view<T> && std::ranges::view<U>) {
//     //     if (std::is_same_v<typename T::value_type, typename U::value_type>) {
//     //         if (arg.size() == other.size()) return std::equal(arg.begin(), arg.end(), other.begin());
//     //         else return false;
//     //     }
//     // }
    
//     return false;
    
// }


template <class T>
inline constexpr std::in_place_type_t<std::vector<T>> vec_t{};

template <typename T>
concept IsPmt = std::is_same_v<T, pmt>;

/*template <class T, class V>
auto get_vector(V value) -> decltype(std::get<std::vector<T>>(value) {
    return std::get<std::vector<T>>(value);
}*/

template <class T, class V>
std::span<T> get_span(V& value) {
    return std::span(std::get<std::vector<T>>(value));
}

template <std::ranges::view T>
std::ostream& _ostream_pmt_vector(std::ostream& os, const T& vec) {
    bool first = true;
    os << "[";
    for (const auto& v: vec) {
        if (first) os << v;
        else os << ", " << v;
        first = false;
    }
    os << "]";
    return os;
}

template <IsPmt P>
std::ostream& operator<<(std::ostream& os, const P& value) {
    return std::visit([&os](const auto& arg) -> std::ostream& {
        using T = std::decay_t<decltype(arg)>;
        if constexpr(Complex<T>) os << "(" << arg.real() << "," << arg.imag() << ")";
        else if constexpr(Scalar<T>) os << arg;
        else if constexpr(UniformVector<T>)  _ostream_pmt_vector(os, std::span(arg));
        /*else if constexpr(IsSharedPtr<T>) {
            if constexpr(UniformVector<typename T::element_type>) {
                _ostream_pmt_vector(os, std::span(*arg));
            } else if constexpr(PmtVector<typename T::element_type>) {
                //_ostream_pmt_vector(os, std::span(*arg));
            } else if constexpr(PmtMap<typename T::element_type>) {
                //_ostream_pmt_map(os, *arg);
            }
        }*/
        else if constexpr(std::same_as<std::nullptr_t, T>) os << "null";
        else if constexpr(std::same_as<T, std::string>) os << arg;
        return os; }
        , value.get_base());
}

template <IsPmt P>
size_t elements(const P& value) {
    return std::visit([](const auto& arg) -> size_t {
        using T = std::decay_t<decltype(arg)>;
        if constexpr(std::same_as<std::nullptr_t, T>) return 0;
        else if constexpr(std::ranges::range<T>) return arg.size();
        return 1; }
        , value.get_base());
}

template <IsPmt P>
size_t bytes_per_element(const P& value) {
    return std::visit([](const auto& arg) -> size_t {
        using T = std::decay_t<decltype(arg)>;
        if constexpr(std::same_as<std::nullptr_t, T>) return 0;
        else if constexpr(std::ranges::range<T>) return sizeof(typename T::value_type);
        return sizeof(T); }
        , value.get_base());
}

template <class T> constexpr uint8_t pmtTypeIndex() {
    if constexpr(std::same_as<T, std::nullptr_t>) return 0;
    else if constexpr(std::same_as<T, bool>) return 1;
    else if constexpr(std::signed_integral<T>) return 2;
    else if constexpr(std::unsigned_integral<T>) return 3;
    else if constexpr(std::floating_point<T>) return 4;
    else if constexpr(Complex<T>) return 5;
    else if constexpr(std::same_as<T, std::string>) return 6;
    else if constexpr(std::ranges::range<T>) {
        if constexpr(UniformVector<T>) {
        return pmtTypeIndex<typename T::value_type>() << 4; 
        }
        else {
            return 7; // for vector of PMTs
        }
    }
    else if constexpr(std::same_as<T, std::map<std::string, pmt>>) return 8;
}

template <class T>
constexpr uint16_t serialId() {
    if constexpr(Scalar<T> || std::same_as<T, bool>) {
        static_assert(sizeof(T) < 32, "Can't serial data wider than 16 bytes");
        return (pmtTypeIndex<T>() << 8) | sizeof(T);
    }
    else if constexpr(UniformVector<T>) {
        static_assert(sizeof(typename T::value_type) < 32, "Can't serial data wider than 16 bytes");
        return (pmtTypeIndex<T>() << 8) | sizeof(typename T::value_type);
    } else return pmtTypeIndex<T>() << 8;
}

template <class T>
struct serialInfo {
    using value_type=T;
    static constexpr uint16_t value = serialId<T>();
};

// FIXME - make this consistent endianness
template <IsPmt P>
size_t serialize(std::streambuf& sb, const P& value) {
    size_t length = 0;
    length += sb.sputn(reinterpret_cast<const char*>(&pmt_version), 2);
    

        std::visit([&length, &sb](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            
            auto id = serialInfo<T>::value;
            length += sb.sputn(reinterpret_cast<const char*>(&id), 2);
            if constexpr(Scalar<T>) {
                auto v = arg;
                length += sb.sputn(reinterpret_cast<const char*>(&v), sizeof(v));
            }
            else if constexpr(UniformVector<T>) {
                uint64_t sz = arg.size();
                length += sb.sputn(reinterpret_cast<const char*>(&sz), sizeof(uint64_t));
                length += sb.sputn(reinterpret_cast<const char*>(arg.data()), arg.size()*sizeof(arg[0]));
            }
            
        }, value);



    return length;
}

template <class T>
T _deserialize_val(std::streambuf& sb) {
    if constexpr(Scalar<T>) {
        T val;
        sb.sgetn(reinterpret_cast<char*>(&val), sizeof(val));
        return val;
    }
    else if constexpr(UniformVector<T>) {
        uint64_t sz; 
        sb.sgetn(reinterpret_cast<char*>(&sz), sizeof(uint64_t));
        std::vector<typename T::value_type> val(sz);
        sb.sgetn(reinterpret_cast<char*>(val.data()), sz*sizeof(val[0]));
        return val;
    }
    else {
        throw std::runtime_error("pmt::_deserialize_value: attempted to deserialize invalid PMT type");
    }
}

pmt deserialize(std::streambuf& sb)
{
    uint16_t version;
    // pmt_container_type container;
    sb.sgetn(reinterpret_cast<char*>(&version), sizeof(version));
    // sb.sgetn(reinterpret_cast<char*>(&container), sizeof(container));

    uint16_t receivedId;
    sb.sgetn(reinterpret_cast<char*>(&receivedId), sizeof(receivedId));

    pmt ret;

    switch(receivedId) {
        case serialInfo<bool>::value: return _deserialize_val<bool>(sb);
        case serialInfo<uint8_t>::value: return _deserialize_val<uint8_t>(sb);
        case serialInfo<uint16_t>::value: return _deserialize_val<uint16_t>(sb);
        case serialInfo<uint32_t>::value: return _deserialize_val<uint32_t>(sb);
        case serialInfo<uint64_t>::value: return _deserialize_val<uint64_t>(sb);
        case serialInfo<int8_t>::value: return _deserialize_val<int8_t>(sb);
        case serialInfo<int16_t>::value: return _deserialize_val<int16_t>(sb);
        case serialInfo<int32_t>::value: return _deserialize_val<int32_t>(sb);
        case serialInfo<int64_t>::value: return _deserialize_val<int64_t>(sb);
        case serialInfo<float>::value: return _deserialize_val<float>(sb);
        case serialInfo<double>::value: return _deserialize_val<double>(sb);
        case serialInfo<std::complex<float>>::value: return _deserialize_val<std::complex<float>>(sb);
        case serialInfo<std::complex<double>>::value: return _deserialize_val<std::complex<double>>(sb);

        // case serialInfo<std::vector<bool>>::value: return _deserialize_val<std::vector<bool>>(sb); 
        case serialInfo<std::vector<uint8_t>>::value: return _deserialize_val<std::vector<uint8_t>>(sb); 
        case serialInfo<std::vector<uint16_t>>::value: return _deserialize_val<std::vector<uint16_t>>(sb); 
        case serialInfo<std::vector<uint32_t>>::value: return _deserialize_val<std::vector<uint32_t>>(sb); 
        case serialInfo<std::vector<uint64_t>>::value: return _deserialize_val<std::vector<uint64_t>>(sb); 
        case serialInfo<std::vector<int8_t>>::value: return _deserialize_val<std::vector<int8_t>>(sb); 
        case serialInfo<std::vector<int16_t>>::value: return _deserialize_val<std::vector<int16_t>>(sb); 
        case serialInfo<std::vector<int32_t>>::value: return _deserialize_val<std::vector<int32_t>>(sb); 
        case serialInfo<std::vector<int64_t>>::value: return _deserialize_val<std::vector<int64_t>>(sb); 
        case serialInfo<std::vector<float>>::value: return _deserialize_val<std::vector<float>>(sb); 
        case serialInfo<std::vector<double>>::value: return _deserialize_val<std::vector<double>>(sb); 
        case serialInfo<std::vector<std::complex<float>>>::value: return _deserialize_val<std::vector<std::complex<float>>>(sb); 
        case serialInfo<std::vector<std::complex<double>>>::value: return _deserialize_val<std::vector<std::complex<double>>>(sb); 
        default: throw std::runtime_error("pmt::deserialize: Invalid PMT type type");
    }

    return ret;
}

template <IsPmt P>
std::string to_base64(const P& value)
{
    std::stringbuf sb; 
    auto nbytes = serialize(sb, value);
    std::string pre_encoded_str(nbytes, '0');
    sb.sgetn(pre_encoded_str.data(), nbytes);
    auto nencoded_bytes = Base64encode_len(nbytes);
    std::string encoded_str(nencoded_bytes, '0');
    auto nencoded = Base64encode(encoded_str.data(), pre_encoded_str.data(), nbytes);
    encoded_str.resize(nencoded - 1); // because it null terminates
    return encoded_str;
}

pmt from_base64(const std::string& encoded_str)
{
    std::string bufplain(encoded_str.size(), '0');
    Base64decode(bufplain.data(), encoded_str.data());
    std::stringbuf sb(bufplain);
    return deserialize(sb); 
}

// Allows us to cast from a pmt like this: auto x = cast<float>(mypmt);
template <class T, IsPmt P>
T cast(const P& value)  {
    return std::visit([](const auto& arg) -> T {
        using U = std::decay_t<decltype(arg)>;
        if constexpr(std::constructible_from<T, U>) return T(arg);
        // else if constexpr (PmtMap<T> && PmtMap<U>) {
        //     return std::get<std::map<std::string, pmt_var_t>>(arg);                
        // }
        else throw std::runtime_error(fmt::format("Invalid PMT Cast {} {}", typeid(T).name(), typeid(U).name()));
    }, value); }

}
