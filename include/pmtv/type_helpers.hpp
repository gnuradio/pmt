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

namespace detail {

// Convert a list of types to the full set used for the pmt.
template<template<typename... > class VariantType, typename... Args>
struct as_pmt {
    using type = VariantType<std::monostate,
                             Args...,
                             std::vector<Args>...,
                             std::string,
                             std::vector<rva::self_t>,
                             std::map<std::string, rva::self_t>>;
};

template<template<typename... > class TemplateType, typename ...T>
struct as_pmt<TemplateType, std::tuple<T...>> {
    using type = typename as_pmt<TemplateType, T...>::type;
};
}


template<template<typename... > class VariantType, class... Args>
using as_pmt_t = typename detail::as_pmt<VariantType, Args...>::type;

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

template <typename T>
concept PmtMap = std::is_same_v<T, std::map<std::string, pmt_var_t>>;

template <typename T>
concept String = std::is_same_v<T, std::string>;

template <typename T>
concept PmtVector =
    std::ranges::contiguous_range<T> && std::is_same_v<typename T::value_type, pmt_var_t>;

template <typename T>
concept associative_array = requires
{
    typename T::key_type;
    typename T::value_type;
    typename T::begin;
    typename T::end;
};

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
