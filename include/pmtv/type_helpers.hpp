#pragma once

#include <cassert>
#include <complex>
#include <concepts>
#include <map>
#include <memory>
#include <ranges>
#include <variant>
#include <vector>
#include <span>
#include <iostream>

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

    constexpr bool operator==(const Tensor<T>(other)) const noexcept {
        return _extents == other._extents && _data == other._data;
    }

    constexpr bool operator!=(const Tensor<T>(other)) const noexcept { return ! operator==(other); }

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

// Forwarding reference works with lvalue and rvalue references.
// template <std::ranges::range T>
// auto pmt_vector = [](T&& in) {
//     using value_type = typename T::value_type;
//     return Tensor<value_type>(in.size(), std::forward<T>(in));
// }

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
                             Args...,
                             Tensor<Args>...,
                             std::string,
                             std::vector<rva::self_t>,
                             std::map<std::string, rva::self_t>>;
};

template<template<typename... > class TemplateType, typename ...T>
struct as_pmt<TemplateType, std::tuple<T...>> {
    using type = as_pmt<TemplateType, T...>::type;
};
}


template<template<typename... > class VariantType, class... Args>
using as_pmt_t = detail::as_pmt<VariantType, Args...>::type;

// Note that per the spec, std::complex is undefined for any type other than float, double, or long_double
using default_supported_types = std::tuple<//bool,
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
    std::ranges::contiguous_range<T> && std::is_same_v<T::value_type, pmt_var_t>;

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

template <typename T>
concept IsTensor = requires { typename T::value_type; } && std::same_as<T, Tensor<typename T::value_type>>;

} // namespace pmtv
