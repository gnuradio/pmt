#pragma once

#include <cassert>
#include <complex>
#include <concepts>
#include <map>
#include <memory>
#include <ranges>
#include <span>
#include <variant>
#include <vector>

#include <fmt/format.h>
#include <pmtv/rva_variant.hpp>

namespace pmtv {

struct Tensor1d { explicit Tensor1d() = default; };

template <class _T>
class Tensor {
  public:
    using T = std::conditional_t<std::same_as<_T, bool>, uint8_t, _T>;
    using value_type = T;

    Tensor() = default;

    template <std::ranges::range Extents> requires (std::same_as<typename Extents::value_type, size_t>)
    Tensor(const Extents& extents) : _extents(extents.begin(), extents.end()), _data(calculate_size(extents)) {}

    // I would like to be able to pass in a range and have it set up the extents as a single dim.
    // I could do a range of value_type, but then it would be confused for integer/size_t cases which is correct.
    // So for floating point types, I can have a constructor of extents, a constructor of values and a constructor
    // of extents and values.

    // What if the data type is size_t?  How do I handle that?  There isn't a way to handle it.  What if I create a
    // lamdba function.
    // pmt

    template <std::ranges::range Extents, std::ranges::range Data>
    requires  (std::same_as<typename Extents::value_type, size_t> && std::same_as<typename Data::value_type, T>)
    Tensor(const Extents& extents, const Data& data) : _extents(extents.begin(), extents.end()), _data(data.begin(), data.end()) {
        if (calculate_size(extents) != data.size()) {
            throw std::runtime_error(fmt::format("Data size = {} doesn't match extents size = {}", data.size(), calculate_size(extents)));
        }
    }

    template <std::ranges::range Data>
    requires  (std::same_as<typename Data::value_type, T>)
    Tensor(Tensor1d, const Data& data) : _extents(1, data.size()), _data(data.begin(), data.end()) {}

    Tensor(Tensor1d, std::initializer_list<T> data) : _extents(1, data.size()), _data(data.begin(), data.end()) {}
    Tensor(Tensor1d, size_t length) : _extents(1, length), _data(length) {}

    Tensor(std::initializer_list<size_t> extents) : _extents(extents), _data(calculate_size(extents)) {}

    constexpr std::span<const size_t> extents() const noexcept { return std::span(_extents); }
    constexpr std::size_t size() const noexcept { return _data.size(); }
    constexpr T* data() noexcept { return _data.data(); }
    constexpr const T* data() const noexcept { return _data.data(); }
    constexpr std::span<T> data_span() noexcept {return std::span(_data); }
    constexpr std::span<const T> data_span() const noexcept {return std::span(_data); }

    // Templated multi-bracket operator
    template <std::integral... Indices>
    constexpr T& operator[](Indices... indices) {
        assert((sizeof...(indices) == _extents.size()) && "incorrect number of indices");
        return _data[calculate_index(indices...)];
        //return _data[calculate_index({static_cast<std::size_t>(indices)...})];
    }

    template <std::integral... Indices>
    [[nodiscard]] constexpr const T& operator[](Indices... indices) const {
        assert(sizeof...(indices) == _extents.size() && "incorrect number of indices");
        return _data[calculate_index({static_cast<std::size_t>(indices)...})];
    }

    const T& get(const std::span<size_t>& indices) const {
        return _data[calculate_index(indices)];
    }

    constexpr bool operator==(const Tensor<T>& other ) const noexcept {
        return _extents == other._extents && _data == other._data;
    }

    constexpr bool operator!=(const Tensor<T>& other) const noexcept { return ! operator==(other); }

    //operator std::mdspan<T, std::dynamic_extent>() const {
    //    return std::mdspan<T, std::dynamic_extent>(_data.data(), _data.size());
    //}
  private:
    std::vector<std::size_t> _extents;
    std::vector<T> _data;

    constexpr std::size_t calculate_size(std::span<const size_t> extents) const noexcept {
        std::size_t size = 1;
        for (size_t extent : extents) {
            size *= extent;
        }
        return size;
    }

    std::size_t calculate_index(std::span<const std::size_t> indices) const {
        assert(indices.size() == _extents.size() && "incorrect number of indices");
        std::size_t index = 0;
        std::size_t stride = 1;
        for (int i = static_cast<int>(_extents.size()) - 1; i >= 0; --i) {
            index += indices[i] * stride;
            stride *= _extents[i];
        }
        return index;
    }

    template <std::integral... Indices>
    constexpr std::size_t calculate_index(Indices... indices) const noexcept {
        static_assert((std::is_convertible_v<Indices, std::size_t> && ...), "Indices must be convertible to std::size_t");
        assert(sizeof...(indices) == _extents.size() && "Incorrect number of indices");
        return calculate_index_impl<0>(static_cast<std::size_t>(indices)...);
    }

    template <std::size_t N, typename First, typename... Rest>
    constexpr std::size_t calculate_index_impl(First first, Rest... rest) const noexcept {
        if constexpr (N + 1 == sizeof...(Rest) + 1) {
            return first;
        } else {
            return first * _extents[N] + calculate_index_impl<N + 1>(rest...);
        }
    }

};

template <class T>
struct is_pmt_tensor : std::false_type {};

template <class T>
struct is_pmt_tensor<Tensor<T>> : std::true_type {};

template <class T>
concept PmtTensor = is_pmt_tensor<std::remove_cvref_t<T>>::value;


namespace detail {

// Convert a list of types to the full set used for the pmt.
template<template<typename... > class VariantType, typename... Args>
struct as_pmt {
    using type = VariantType<std::monostate,
                             bool,
                             Args...,
                             Tensor<Args>...,
                             std::string,
                             std::vector<std::string>,
                             std::vector<rva::self_t>,
                             std::map<std::string, rva::self_t, std::less<>>
                             >;
};

template<template<typename... > class TemplateType, typename ...T>
struct as_pmt<TemplateType, std::tuple<T...>> {
    using type = typename as_pmt<TemplateType, T...>::type;
};
}


template<template<typename... > class VariantType, class... Args>
using as_pmt_t = typename detail::as_pmt<VariantType, Args...>::type;

// Check if `std::size_t` has the same type as `uint16_t` or `uint32_t` or `uint64_t`.
// If it has the same type, then there is no need to add `std::size_t` the supported types.
// Otherwise, `std::size_t` is added to the supported types.
// This can happen if one builds using Emscripten where `std::size_t` is defined as `unsigned long` and
// `uint32_t` and `uint64_t` are defined as `unsigned int` and `unsigned long long`, respectively.
static constexpr bool support_size_t = !std::is_same_v<std::size_t, uint16_t> && !std::is_same_v<std::size_t, uint32_t> && !std::is_same_v<std::size_t, uint64_t>;

// Note that per the spec, std::complex is undefined for any type other than float, double, or long_double
using default_supported_types_without_size_t = std::tuple<//bool,
        uint8_t, uint16_t, uint32_t, uint64_t,
        int8_t, int16_t, int32_t, int64_t,
        float, double, std::complex<float>, std::complex<double>>;

// Add std::size_t to default_supported_types_without_size_t
using default_supported_types_with_size_t = decltype(std::tuple_cat(std::declval<default_supported_types_without_size_t>(), std::declval<std::tuple<std::size_t>>()));

using default_supported_types = typename std::conditional_t<support_size_t, default_supported_types_with_size_t, default_supported_types_without_size_t>;

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
concept UniformStringVector =
    std::ranges::range<T> && std::same_as<typename T::value_type, std::string>;

template <typename T>
concept PmtMap = std::is_same_v<T, std::map<std::string, pmt_var_t, std::less<>>>;

template <typename T>
concept String = std::is_same_v<T, std::string>;

template <typename T>
concept PmtVector =
    std::ranges::range<T> && std::is_same_v<typename T::value_type, pmt_var_t>;

template <typename T>
concept IsTensor = requires { typename T::value_type; } && std::same_as<T, Tensor<typename T::value_type>>;

} // namespace pmtv
