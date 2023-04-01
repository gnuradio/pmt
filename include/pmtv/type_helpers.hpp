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
                             std::map<std::string, rva::self_t>
                             >;
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

} // namespace pmtv
