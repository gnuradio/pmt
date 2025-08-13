#pragma once


#include <pmtv/type_helpers.hpp>
#include <pmtv/version.hpp>
#include <complex>
#include <cstddef>
#include <ranges>
#include <span>

namespace pmtv {

    using pmt = pmt_var_t;
    using map_t = std::map<std::string, pmt, std::less<>>;

    template<class T>
    inline constexpr std::in_place_type_t<std::vector<T>> vec_t{};

    template<typename T>
    concept IsPmt = std::is_same_v<T, pmt>;

// template <class T, class V>
// auto get_vector(V value) -> decltype(std::get<std::vector<T>>(value) {
//     return std::get<std::vector<T>>(value);
// }
    template<class T, class V>
    std::vector<T> &get_vector(V &value) {
        return std::get<std::vector<T>>(value);
    }

    template<class T, class V>
    std::span<T> get_span(V &value) {
        return std::span(std::get<std::vector<T>>(value));
    }

    template<class V>
    map_t &get_map(V &value) {
        return std::get<map_t>(value);
    }

    template<IsPmt P>
    size_t elements(const P &value) {
        return std::visit(
                [](const auto &arg) -> size_t {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::same_as<std::monostate, T>)
                        return 0;
                    else if constexpr (std::ranges::range<T>)
                        return arg.size();
                    return 1;
                },
                value.get_base());
    }

    template<IsPmt P>
    size_t bytes_per_element(const P &value) {
        return std::visit(
                [](const auto &arg) -> size_t {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::same_as<std::monostate, T>)
                        return 0;
                    else if constexpr (std::ranges::range<T>)
                        return sizeof(typename T::value_type);
                    return sizeof(T);
                },
                value.get_base());
    }

    // Allows us to cast from a pmt like this: auto x = cast<float>(mypmt);
    template<class T, IsPmt P>
    T cast(const P &value) {
        return std::visit(
                [](const auto &arg) -> T {
                    using U = std::decay_t<decltype(arg)>;
                    if constexpr (std::convertible_to<U, T> || (Complex < T > && Complex < U > )) {
                        if constexpr (Complex < T >) {
                            if constexpr (std::integral<U> || std::floating_point<U>) {
                                return std::complex<typename T::value_type>(static_cast<typename T::value_type>(arg));
                            } else {
                                return static_cast<T>(arg);
                            }
                        } else {
                            return static_cast<T>(arg);
                        }
                    }
                        // else if constexpr (PmtMap<T> && PmtMap<U>) {
                        //     return std::get<std::map<std::string, pmt_var_t, std::less<>>>(arg);
                        // }
                    else
                        throw std::runtime_error("Invalid PMT Cast " + std::string(typeid(T).name()) + " " +
                                                 std::string(typeid(U).name()));
                },
                value);
    }

}
