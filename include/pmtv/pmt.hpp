#pragma once

#include <pmtv/rva_variant.hpp>
#include <pmtv/type_helpers.hpp>
#include <pmtv/version.hpp>
#include "base64/base64.h"
#include <cstddef>
#include <complex>
#include <ranges>
#include <span>

#include <fmt/format.h>

namespace pmtv {


using pmt_var_t = rva::variant<
    std::nullptr_t,
    uint8_t, uint16_t, uint32_t, uint64_t,
    int8_t, int16_t, int32_t, int64_t,
    float, double, std::complex<float>, std::complex<double>,
    std::vector<uint8_t>, std::vector<uint16_t>, std::vector<uint32_t>, std::vector<uint64_t>,
    std::vector<int8_t>, std::vector<int16_t>, std::vector<int32_t>, std::vector<int64_t>,
    std::vector<float>, std::vector<double>,
    std::vector<std::complex<float>>, std::vector<std::complex<double>>,
    std::string,
    std::vector<rva::self_t>,
    std::map<std::string, rva::self_t>>;


class pmt : public pmt_var_t {
    public:
    pmt(): pmt_var_t(nullptr) {}
    pmt(const pmt& other) = default;
    
    template <class T>
    pmt(const T& other) : pmt_var_t(other) {} 
    // Probably only useful for strings
    template <class T>
    pmt(const T* other);

    // Forwarding Constructor to wrap pmt_var_t
    template <class... Args>
    pmt(Args&&... args)
        : variant{std::forward<Args>(args)...}
    {}

    pmt& operator=(const pmt& other) = default;

    size_t serialize(std::streambuf& sb) const;
    static pmt deserialize(std::streambuf& sb);
    std::string to_base64() const;
    static pmtv::pmt from_base64(const std::string& encoded_str);



    // Allows us to cast from a pmt like this: float x = float(mypmt);
    // Must be explicit.
    template <class T>
    explicit operator T() const {
        return std::visit([](const auto& arg) -> T {
            using U = std::decay_t<decltype(arg)>;
            if constexpr(std::constructible_from<T, U>) return T(arg);
            else throw std::runtime_error(fmt::format("Invalid PMT Cast {} {}", typeid(T).name(), typeid(U).name()));
        }, *this); }

private:
    template <typename T>
    static T _deserialize_val(std::streambuf& sb);

    template <typename T>
    static pmt _deserialize_vec(std::streambuf& sb, size_t sz);
};



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

// FIXME - make this consistent endianness
size_t pmt::serialize(std::streambuf& sb) const {
    size_t length = 0;
    length += sb.sputn(reinterpret_cast<const char*>(&pmt_version), 2);
    
    pmt_container_type container;
    std::visit([&container](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        container = container_type<T>();        
    }, *this);

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
            
        }, *this);
    
    }
    else if (container == pmt_container_type::UNIFORM_VECTOR) {
        std::visit([&length, &sb](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr(UniformVector<T>) {

                auto id = element_type<T>();
                length += sb.sputn(reinterpret_cast<const char*>(&id), 1);
                uint64_t sz = arg.size();
                length += sb.sputn(reinterpret_cast<const char*>(&sz), sizeof(uint64_t));
                length += sb.sputn(reinterpret_cast<const char*>(arg.data()), arg.size()*sizeof(arg[0]));
            }
        }, *this);
    
    }

    return length;
}

template <typename T>
T pmt::_deserialize_val(std::streambuf& sb) {
    if constexpr(Scalar<T>) {
    T val;
    sb.sgetn(reinterpret_cast<char*>(&val), sizeof(val));
    std::cout << val << std::endl;
    return val;
    }
    else {
        throw std::runtime_error("pmt::_deserialize_value: attempted to deserialize non-scalar type");
    }
}

template <typename T>
pmt pmt::_deserialize_vec(std::streambuf& sb, size_t sz) {
    if constexpr(UniformVector<T>) {
        std::vector<typename T::value_type> val(sz);
        sb.sgetn(reinterpret_cast<char*>(val.data()), sz*sizeof(val[0]));
        return pmt(val);
    }
    else {
        throw std::runtime_error("pmt::_deserialize_vec: attempted to deserialize non-vector type");
    }
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
            case pmt_element_type::BOOL: return _deserialize_val<bool>(sb); 
            case pmt_element_type::UINT8: return _deserialize_val<uint8_t>(sb); 
            case pmt_element_type::UINT16: return _deserialize_val<uint16_t>(sb); 
            case pmt_element_type::UINT32: return _deserialize_val<uint32_t>(sb); 
            case pmt_element_type::UINT64: return _deserialize_val<uint64_t>(sb); 
            case pmt_element_type::INT8: return _deserialize_val<int8_t>(sb); 
            case pmt_element_type::INT16: return _deserialize_val<int16_t>(sb); 
            case pmt_element_type::INT32: return _deserialize_val<int32_t>(sb); 
            case pmt_element_type::INT64: return _deserialize_val<int64_t>(sb); 
            case pmt_element_type::FLOAT: return _deserialize_val<float>(sb); 
            case pmt_element_type::DOUBLE: return _deserialize_val<double>(sb); 
            case pmt_element_type::COMPLEX_FLOAT: return _deserialize_val<std::complex<float>>(sb); 
            case pmt_element_type::COMPLEX_DOUBLE: return _deserialize_val<std::complex<double>>(sb); 
            default: throw std::runtime_error("pmt::deserialized: Invalid PMT Scalar type");
        }
    }
    else if (container == pmt_container_type::UNIFORM_VECTOR) {
        pmt_element_type T_type;
        sb.sgetn(reinterpret_cast<char*>(&T_type), 1);
        uint64_t sz; 
        sb.sgetn(reinterpret_cast<char*>(&sz), sizeof(uint64_t));

        switch(T_type) {
            case pmt_element_type::BOOL: return _deserialize_vec<std::vector<bool>>(sb, sz); 
            case pmt_element_type::UINT8: return _deserialize_vec<std::vector<uint8_t>>(sb, sz); 
            case pmt_element_type::UINT16: return _deserialize_vec<std::vector<uint16_t>>(sb, sz); 
            case pmt_element_type::UINT32: return _deserialize_vec<std::vector<uint32_t>>(sb, sz); 
            case pmt_element_type::UINT64: return _deserialize_vec<std::vector<uint64_t>>(sb, sz); 
            case pmt_element_type::INT8: return _deserialize_vec<std::vector<int8_t>>(sb, sz); 
            case pmt_element_type::INT16: return _deserialize_vec<std::vector<int16_t>>(sb, sz); 
            case pmt_element_type::INT32: return _deserialize_vec<std::vector<int32_t>>(sb, sz); 
            case pmt_element_type::INT64: return _deserialize_vec<std::vector<int64_t>>(sb, sz); 
            case pmt_element_type::FLOAT: return _deserialize_vec<std::vector<float>>(sb, sz); 
            case pmt_element_type::DOUBLE: return _deserialize_vec<std::vector<double>>(sb, sz); 
            case pmt_element_type::COMPLEX_FLOAT: return _deserialize_vec<std::vector<std::complex<float>>>(sb, sz); 
            case pmt_element_type::COMPLEX_DOUBLE: return _deserialize_vec<std::vector<std::complex<double>>>(sb, sz); 
            default: throw std::runtime_error("pmt::deserialize: Invalid PMT UniformVector type");
        }
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

}
