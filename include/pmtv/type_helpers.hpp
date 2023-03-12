#pragma once

#include <complex>
#include <concepts>
#include <map>
#include <memory>
#include <ranges>
#include <variant>
#include <vector>

#include <pmtv/rva_variant.hpp>

namespace pmtv {

/* Forward declaration of a struct_wrapper class.  This will allow us to store "any" structure in
 a pmt.  The only extra requirement is that we use a refl-cpp macro after the structure definiton.
 Also the data types of the structure must be mappable to pmt types.  (No custom datatypes, no
 pointer and length, etc)
 For example,
 struct my_data {
    float x;
    int y;
    std::complex<float> z;
 };

 REFL_AUTO(type(my_data), field(x), field(y), field(z))

 There is a helper function `pmt_from_struct()` that will create the pmt for you.  The data is not
 readily accessible inside of the pmt and must be cast back out. 
*/

// forward declaration
class struct_wrapper_base;

// This class exists because of an issue with deserializing a struct_wrapper.  When we serialize a
// struct, we can only deserialize it if we know the type.  This isn't a guarantee on the recieving
// end.  This class holds the serialized value as a string that can later be converted back to the
// struct.
// For example, We convert a struct to a pmt and then serialize it over the network.  The receiving
// block may be on a different system and may not know the structure definition.  It still needs to
// be able to handle it.  It creates a serialized_struct pmt and then we can later use the value.
class serialized_struct {
  private:
    std::vector<char> _data;
  public:
    using value_type = std::vector<char>::value_type;
    using size_type = std::vector<char>::size_type;
    serialized_struct(const std::string_view& value) : _data(value.begin(), value.end()) {}
    serialized_struct(std::vector<value_type>&& value) : _data(std::move(value)) {}
    const value_type* data() const noexcept { return _data.data(); }
    size_type size() const noexcept { return _data.size(); }
};

template <std::same_as<serialized_struct> T>
bool operator==(const T& x, const T& y) {
    if (x.size() == y.size())
        return std::equal(x.data(), x.data() + x.size(), y.data());
    return false;
}

namespace detail {

// Convert a list of types to the full set used for the pmt.
template<template<typename... > class VariantType, typename... Args>
struct as_pmt {
    using type = VariantType<std::monostate,
                             Args...,
                             std::vector<Args>...,
                             std::string,
                             std::vector<rva::self_t>,
                             std::map<std::string, rva::self_t>,
                             std::shared_ptr<struct_wrapper_base>,
                             serialized_struct>;
};

template<template<typename... > class TemplateType, typename ...T>
struct as_pmt<TemplateType, std::tuple<T...>> {
    using type = as_pmt<TemplateType, T...>::type;
};
}


template<template<typename... > class VariantType, class... Args>
using as_pmt_t = detail::as_pmt<VariantType, Args...>::type;

// Note that per the spec, std::complex is undefined for any type other than float, double, or long_double
using default_supported_types = std::tuple<bool,
                                           uint8_t, uint16_t, uint32_t, uint64_t,
                                           int8_t, int16_t, int32_t, int64_t,
                                           float, double, std::complex<float>, std::complex<double>>;

// initialisation via type list stored in tuple (N.B. tuple could be extended by user with custom OOT types)
using pmt_var_t = as_pmt_t<rva::variant, default_supported_types>;

using pmt_null = std::monostate;


template <typename T>
concept PmtNull = std::is_same_v<T, std::monostate>;

template <typename T>
concept Complex =
    std::is_same_v<T, std::complex<float>> || std::is_same_v<T, std::complex<double>>;

template <typename T>
concept Scalar = std::same_as<T, bool> || std::integral<T> || std::floating_point<T> || Complex<T>;

template <typename T>
concept UniformVector =
    std::ranges::contiguous_range<T> && Scalar<typename T::value_type>;

// A vector of bool can be optimized to one bit per element, so it doesn't satisfy UniformVector
template <typename T>
concept UniformBoolVector = 
    std::ranges::range<T> && std::same_as<typename T::value_type, bool>;

template <typename T>
concept PmtMap = std::is_same_v<T, std::map<std::string, pmt_var_t>>;

template <typename T>
concept String = std::is_same_v<T, std::string>;

template <typename T>
concept PmtVector =
    std::ranges::range<T> && std::is_same_v<typename T::value_type, pmt_var_t>;

template <typename T>
concept associative_array = requires
{
    typename T::key_type;
    typename T::value_type;
    typename T::begin;
    typename T::end;
};

template <typename T> struct is_shared_ptr : std::false_type {};
template <typename T> struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

template <typename T>
concept SharedPtr = is_shared_ptr<T>::value;

template <Scalar T>
std::string type_string()
{
    if constexpr (std::is_same_v<T, uint8_t>)
        return "uint8_t";
    else if constexpr (std::is_same_v<T, uint16_t>)
        return "uint16_t";
    else if constexpr (std::is_same_v<T, uint32_t>)
        return "uint32_t";
    else if constexpr (std::is_same_v<T, uint64_t>)
        return "uint64_t";
    else if constexpr (std::is_same_v<T, int8_t>)
        return "int8_t";
    else if constexpr (std::is_same_v<T, int16_t>)
        return "int16_t";
    else if constexpr (std::is_same_v<T, int32_t>)
        return "int32_t";
    else if constexpr (std::is_same_v<T, int64_t>)
        return "int64_t";
    else if constexpr (std::is_same_v<T, float>)
        return "float32";
    else if constexpr (std::is_same_v<T, double>)
        return "float64";
    else if constexpr (std::is_same_v<T, std::complex<float>>)
        return "complex:float32";
    else if constexpr (std::is_same_v<T, std::complex<double>>)
        return "complex:float64";
    return "Unknown Type";
}

template <UniformVector T>
std::string type_string()
{
    return "vector:" + type_string<typename T::value_type>();
}

/*template <PmtVector T>
std::string type_string() {
    return "vector:pmt";
}*/

template <PmtMap T>
std::string type_string()
{
    return "map:pmt";
}

template <class T>
std::string type_string()
{
    return "Unknown";
}

inline std::string get_type_string(const auto& arg)
{
    using T = std::decay_t<decltype(arg)>;
    return type_string<T>();
}

// This set of structs and types exist to help us work with containers.
// For example, maps should work with associative containers, but not vectors.
// These types help us distinguish between them all.
template <typename T, typename _ = void>
struct is_container : std::false_type {
};

// This syntax may look tricky, but is fairly simple to understand.  Any type,
// T, will "work" if it defines all of the things in the std::void_t.  If any
// of them are missing, then the struct will fall back to the value defined
// above.
template <typename T>
struct is_container<T,
                    std::void_t<typename T::value_type,
                                typename T::size_type,
                                typename T::iterator,
                                typename T::const_iterator,
                                decltype(std::declval<T>().size()),
                                decltype(std::declval<T>().begin()),
                                decltype(std::declval<T>().end())>>
    : public std::true_type {
};

// Vector like containers are defined as containers that store data elements in
// an "ordered" list like fashion.  This would include vectors, arrays, and
// lists.
template <typename Container, typename _ = void>
struct is_vector_like_container : std::false_type {
};

template <typename T>
struct is_vector_like_container<T,
                                std::void_t<typename T::value_type,
                                            typename T::size_type,
                                            typename T::iterator,
                                            // typename T::const_iterator,
                                            decltype(std::declval<T>().data()),
                                            decltype(std::declval<T>().size()),
                                            decltype(std::declval<T>().begin()),
                                            decltype(std::declval<T>().end())>>
    : public std::true_type {
};

// Map like containers have a key and a value.  This would include maps and
// unordered maps (hash tables).
template <typename Container, typename _ = void>
struct is_map_like_container : std::false_type {
};

template <typename T>
struct is_map_like_container<T,
                             std::void_t<typename T::value_type,
                                         typename T::mapped_type,
                                         typename T::size_type,
                                         typename T::iterator,
                                         typename T::const_iterator,
                                         decltype(std::declval<T>().size()),
                                         decltype(std::declval<T>().begin()),
                                         decltype(std::declval<T>().end())>>
    : public std::true_type {
};

template <typename T>
using IsContainer = std::enable_if_t<is_container<T>::value, bool>;

template <typename T>
using IsVectorLikeContainer = std::enable_if_t<is_vector_like_container<T>::value, bool>;

template <typename T>
using IsMapLikeContainer = std::enable_if_t<is_map_like_container<T>::value, bool>;

} // namespace pmtv
