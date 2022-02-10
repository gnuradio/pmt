#pragma once

#include <string>
#include <complex>
#include <pmtf/pmtf_generated.h>

namespace pmtf {

// Forward declare pmt
class pmt;


// Define some SFINAE templates.  Since we can create pmts from the various classes,
// We want to ensure that we really do or do not have one when we call a function.
template <typename T>
using IsPmt = std::enable_if_t<std::is_same_v<T, pmt>, bool>;
template <typename T>
using IsNotPmt = std::enable_if_t<!std::is_same_v<T, pmt>, bool>;

template<typename T, typename _ = void>
struct is_pmt_derived : std::false_type {};

template<typename T>
struct is_pmt_derived<
        T,
        std::void_t<decltype(std::declval<T>().get_pmt_buffer())>
        > : public std::true_type {};

template <typename T>
using IsNotPmtDerived = std::enable_if_t<!is_pmt_derived<T>::value, bool>;

template <class T>
struct is_complex : std::false_type {};

template <class T>
struct is_complex<std::complex<T>> : std::true_type {};

template <typename T>
using IsComplex = std::enable_if_t<is_complex<T>::value, bool>;

template <typename T>
using IsNotComplex = std::enable_if_t<!is_complex<T>::value, bool>;

template <typename T>
using IsArithmetic = std::enable_if_t<std::is_arithmetic_v<T>>;

template <typename T>
using IsScalarBase = std::enable_if_t<std::is_arithmetic_v<T> || is_complex<T>::value, bool>;

template <typename T>
using IsVectorBase = std::enable_if_t<std::is_arithmetic_v<T> || is_complex<T>::value || std::is_same_v<T,pmt>, bool>;

template<typename T, typename _ = void>
struct is_container : std::false_type {};

template<typename T>
struct is_container<
        T,
        std::void_t<
                typename T::value_type,
                typename T::size_type,
                typename T::iterator,
                typename T::const_iterator,
                decltype(std::declval<T>().size()),
                decltype(std::declval<T>().begin()),
                decltype(std::declval<T>().end())
            >
        > : public std::true_type {};

template <typename Container, typename _ = void>
struct is_vector_like_container: std::false_type {};

template <typename T>
struct is_vector_like_container<
        T,
        std::void_t<
                typename T::value_type,
                typename T::size_type,
                typename T::iterator,
                typename T::const_iterator,
                decltype(std::declval<T>().data()),
                decltype(std::declval<T>().size()),
                decltype(std::declval<T>().begin()),
                decltype(std::declval<T>().end())
            >
        > : public std::true_type {};

template <typename Container, typename _ = void>
struct is_map_like_container: std::false_type {};

template <typename T>
struct is_map_like_container<
        T,
        std::void_t<
                typename T::value_type,
                typename T::mapped_type,
                typename T::size_type,
                typename T::iterator,
                typename T::const_iterator,
                decltype(std::declval<T>().size()),
                decltype(std::declval<T>().begin()),
                decltype(std::declval<T>().end())
            >
        > : public std::true_type {};

template <typename T>
using IsContainer = std::enable_if_t<is_container<T>::value, bool>;

template <typename T>
using IsVectorLikeContainer = std::enable_if_t<is_vector_like_container<T>::value, bool>;

template <typename T>
using IsMapLikeContainer = std::enable_if_t<is_map_like_container<T>::value, bool>;

// We need to know the struct type for complex values
template <class T> struct scalar_type;
template <> struct scalar_type<std::complex<float>> { using type = Complex64; };
template <> struct scalar_type<std::complex<double>> { using type = Complex128; };

template <class T> inline std::string ctype_string();
template <> inline std::string ctype_string<bool>() { return "bool"; }
template <> inline std::string ctype_string<char>() { return "char"; }
template <> inline std::string ctype_string<uint8_t>() { return "uint8_t"; }
template <> inline std::string ctype_string<uint16_t>() { return "uint16_t"; }
template <> inline std::string ctype_string<uint32_t>() { return "uint32_t"; }
template <> inline std::string ctype_string<uint64_t>() { return "uint64_t"; }
template <> inline std::string ctype_string<int8_t>() { return "int8_t"; }
template <> inline std::string ctype_string<int16_t>() { return "int16_t"; }
template <> inline std::string ctype_string<int32_t>() { return "int32_t"; }
template <> inline std::string ctype_string<int64_t>() { return "int64_t"; }
template <> inline std::string ctype_string<float>() { return "float"; }
template <> inline std::string ctype_string<double>() { return "double"; }
template <> inline std::string ctype_string<std::complex<float>>() { return "complex<float>"; }
template <> inline std::string ctype_string<std::complex<double>>() { return "complex<double>"; }
template <> inline std::string ctype_string<pmt>() { return "pmt"; }

template< class T >
struct remove_cvref {
    typedef std::remove_cv_t<std::remove_reference_t<T>> type;
};
template< class T >
using remove_cvref_t = typename remove_cvref<T>::type;
}
