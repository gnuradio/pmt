#include <complex>
#include <concepts>
#include <ranges>
#include <variant>
#include <vector>

namespace pmtv {
// It is really hard to declare a variant that contains itself.
// These are the steps required.


template <typename T> struct is_shared_ptr : std::false_type {};
template <typename T> struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};
template <typename T> concept IsSharedPtr = is_shared_ptr<T>::value;

template <typename T>
concept PmtNull = std::is_same_v<T, nullptr_t>;

template <typename T>
concept Complex = std::is_same_v<T, std::complex<float>> || std::is_same_v<T, std::complex<double>>;

template <typename T>
concept Scalar = std::integral<T> || std::floating_point<T> || Complex<T>;

template <typename T>
concept UniformVector = std::ranges::contiguous_range<T> && Scalar<typename T::value_type>;

template <typename T>
concept UniformVectorInsidePmt = IsSharedPtr<T> && UniformVector<typename T::element_type>;

/*template <typename T>
concept PmtMap = std::is_same_v<T, std::map<std::string, _pmt_storage>>;

template <typename T>
concept PmtMapInsidePmt = IsSharedPtr<T> && PmtMap<T>;

template <typename T>
concept PmtVector = std::is_same_v<T, std::vector<_pmt_storage>>;*/

template <typename T>
concept associative_array = requires {
    typename T::key_type;
    typename T::value_type;
    typename T::begin;
    typename T::end;
};

template <Scalar T>
std::string type_string() {
    if constexpr(std::is_same_v<T, uint8_t>) return "uint8_t";
    else if constexpr(std::is_same_v<T, uint16_t>) return "uint16_t";
    else if constexpr(std::is_same_v<T, uint32_t>) return "uint32_t";
    else if constexpr(std::is_same_v<T, uint64_t>) return "uint64_t";
    else if constexpr(std::is_same_v<T, int8_t>) return "int8_t";
    else if constexpr(std::is_same_v<T, int16_t>) return "int16_t";
    else if constexpr(std::is_same_v<T, int32_t>) return "int32_t";
    else if constexpr(std::is_same_v<T, int64_t>) return "int64_t";
    else if constexpr(std::is_same_v<T, float>) return "float32";
    else if constexpr(std::is_same_v<T, double>) return "float64";
    else if constexpr(std::is_same_v<T, std::complex<float>>) return "complex:float32";
    else if constexpr(std::is_same_v<T, std::complex<double>>) return "complex:float64";
    return "Unknown Type";
}

enum class pmt_element_type : uint8_t {
  UNKNOWN = 0,
  UINT8 = 1,
  UINT16,
  UINT32,
  UINT64,
  INT8,
  INT16,
  INT32,
  INT64,
  FLOAT,
  DOUBLE,
  COMPLEX_FLOAT,
  COMPLEX_DOUBLE,
  BOOL,

  STRING=253,
  PMT=254,
  INVALID=255
};

template <Scalar T>
pmt_element_type element_type() {
    if constexpr(std::is_same_v<T, uint8_t>) return pmt_element_type::UINT8;
    else if constexpr(std::is_same_v<T, uint16_t>) return pmt_element_type::UINT16;
    else if constexpr(std::is_same_v<T, uint32_t>) return pmt_element_type::UINT32;
    else if constexpr(std::is_same_v<T, uint64_t>) return pmt_element_type::UINT64;
    else if constexpr(std::is_same_v<T, int8_t>) return pmt_element_type::INT8;
    else if constexpr(std::is_same_v<T, int16_t>) return pmt_element_type::INT16;
    else if constexpr(std::is_same_v<T, int32_t>) return pmt_element_type::INT32;
    else if constexpr(std::is_same_v<T, int64_t>) return pmt_element_type::INT64;
    else if constexpr(std::is_same_v<T, float>) return pmt_element_type::FLOAT;
    else if constexpr(std::is_same_v<T, double>) return pmt_element_type::DOUBLE;
    else if constexpr(std::is_same_v<T, std::complex<float>>) return pmt_element_type::COMPLEX_FLOAT;
    else if constexpr(std::is_same_v<T, std::complex<double>>) return pmt_element_type::COMPLEX_DOUBLE;
    return pmt_element_type::UNKNOWN;
}


enum class pmt_container_type : uint16_t {
    EMPTY,
    SCALAR,
    UNIFORM_VECTOR,
    PMT_VECTOR,
    MAP,
    STRING
};

template <Scalar T>
pmt_container_type container_type() {
    return pmt_container_type::SCALAR;
}

template <UniformVector T>
std::string type_string() {
    return "vector:" + type_string<typename T::value_type>();
}

template <UniformVector T>
pmt_element_type element_type() {
    return element_type<typename T::value_type>();
}

template <UniformVectorInsidePmt T>
pmt_element_type element_type() {
    return element_type<typename T::element_type>();
}

template <UniformVector T>
pmt_container_type container_type() {
    return pmt_container_type::UNIFORM_VECTOR;
}

template <UniformVectorInsidePmt T>
pmt_container_type container_type() {
    return pmt_container_type::UNIFORM_VECTOR;
}

/*template <PmtVector T>
std::string type_string() {
    return "vector:pmt";
}*/

/*template <PmtVector T>
pmt_element_type element_type() {
    return pmt_element_type::PMT;
}

template <PmtVector T>
pmt_container_type container_type() {
    return pmt_container_type::PMT_VECTOR;
}

template <PmtMap T>
std::string type_string() {
    return "map:pmt";
}

template <PmtMap T>
pmt_element_type element_type() {
    return pmt_element_type::PMT;
}

template <PmtMap T>
pmt_container_type container_type() {
    return pmt_container_type::MAP;
}*/

template <class T>
std::string type_string() {
    return "Unknown";
}

template <class T>
pmt_element_type element_type() {
    return pmt_element_type::UNKNOWN;
}

template <class T>
pmt_container_type container_type() {
    return pmt_container_type::EMPTY;
}


inline std::string get_type_string(const auto& arg) {
    using T = std::decay_t<decltype(arg)>;
    return type_string<T>();
}

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
                // typename T::const_iterator,
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

} // namespace


