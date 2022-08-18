#pragma once

#include <string>
#include <complex>
#include <pmtf/pmtf_generated.h>

/**********************************************************************
 * This header is where we put most of the icky SFINAE stuff.  Most 
 * people do not need to understand what is going on here.
 *********************************************************************/

namespace pmtf {

// Forward declare pmt
class pmt;

// Pmt is a very permissive type.  For example, a pmt can be constructed from an
// std::vector<int>.  This means that functions that take a pmt as an input arg,
// will accept an std::vector<int> and do an implicit conversion.  This is not
// desirable behaviour.
// These templates can be used to make a function only work if the type is (or 
// is not) a pmt. No implicit conversions will be performed.
template <typename T>
using IsPmt = std::enable_if_t<std::is_same_v<T, pmt>, bool>;
template <typename T>
using IsNotPmt = std::enable_if_t<!std::is_same_v<T, pmt>, bool>;

// These structs and types are used to decide if the class passed in is one of
// our pmt wrapper classes like scalar, vector, etc.
template<typename T, typename _ = void>
struct is_pmt_derived : std::false_type {};

template<typename T>
struct is_pmt_derived<
        T,
        // All of the classes define this function
        std::void_t<decltype(std::declval<T>().get_pmt_buffer())>
        > : public std::true_type {};

template <typename T>
using IsNotPmtDerived = std::enable_if_t<!is_pmt_derived<T>::value, bool>;
template <typename T>
using IsPmtDerived = std::enable_if_t<is_pmt_derived<T>::value, bool>;

// There are times where we need to do something different if the data is
// complex.  These structs and types make is easy for us to distinguish them.
template <class T>
struct is_complex : std::false_type {};
template <class T>
struct is_complex<std::complex<T>> : std::true_type {};

template <typename T>
using IsComplex = std::enable_if_t<is_complex<T>::value, bool>;
template <typename T>
using IsNotComplex = std::enable_if_t<!is_complex<T>::value, bool>;

// This set of structs and types exist to help us work with containers.
// For example, maps should work with associative containers, but not vectors.
// These types help us distinguish between them all.
template<typename T, typename _ = void>
struct is_container : std::false_type {};

// This syntax may look tricky, but is fairly simple to understand.  Any type,
// T, will "work" if it defines all of the things in the std::void_t.  If any
// of them are missing, then the struct will fall back to the value defined
// above.
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

// Vector like containers are defined as containers that store data elements in
// an "ordered" list like fashion.  This would include vectors, arrays, and
// lists.
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

// Map like containers have a key and a value.  This would include maps and 
// unordered maps (hash tables).
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

// For complex data, we need to be able to map the type to a pmt type.
template <class T> struct scalar_type;
template <> struct scalar_type<std::complex<float>> { using type = Complex64; };
template <> struct scalar_type<std::complex<double>> { using type = Complex128; };

// For error reporting we need to be able to map types to strings.
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

}
