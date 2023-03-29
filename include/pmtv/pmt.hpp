#pragma once


#include "base64/base64.h"
#include <pmtv/type_helpers.hpp>
#include <pmtv/version.hpp>
#include <complex>
#include <cstddef>
#include <ranges>
#include <span>
#include <pmtv/refl.hpp>

// Support for std::format is really spotty.
// Gcc12 does not support it.
// Eventually replace with std::format when that is widely available.
#include <fmt/format.h>

namespace pmtv {

using pmt = pmt_var_t;
using map_t = std::map<std::string, pmt>;

template <class T>
inline constexpr std::in_place_type_t<std::vector<T>> vec_t{};

template <typename T>
concept IsPmt = std::is_same_v<T, pmt>;

// template <class T, class V>
// auto get_vector(V value) -> decltype(std::get<std::vector<T>>(value) {
//     return std::get<std::vector<T>>(value);
// }
template <class T, class V>
std::vector<T>& get_vector(V value)
{
    return std::get<std::vector<T>>(value);
}

template <class T, class V>
std::span<T> get_span(V& value)
{
    return std::span(std::get<std::vector<T>>(value));
}

template <class V>
map_t& get_map(V& value)
{
    return std::get<map_t>(value);
}

template <IsPmt P>
size_t elements(const P& value)
{
    return std::visit(
        [](const auto& arg) -> size_t {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::same_as<std::monostate, T>)
                return 0;
            else if constexpr (std::ranges::range<T>)
                return arg.size();
            return 1;
        },
        value.get_base());
}

template <IsPmt P>
size_t bytes_per_element(const P& value)
{
    return std::visit(
        [](const auto& arg) -> size_t {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::same_as<std::monostate, T>)
                return 0;
            else if constexpr (std::ranges::range<T>)
                return sizeof(typename T::value_type);
            return sizeof(T);
        },
        value.get_base());
}


template <typename T>
constexpr auto readable_members = filter(refl::member_list<T>{}, [](auto member) { return is_readable(member); });


/*********************Map Conversion Functions*****************************************/

// Functions for converting between pmt stuctures and maps.
template <class T>
constexpr void map_from_struct(const T& value, map_t& result) {
    // iterate over the members of T
    for_each(refl::reflect(value).members, [&](auto member)
    {
        if constexpr (is_readable(member))
        {
            result[get_display_name(member)] = member(value);
        }
    });
}

template <class T>
constexpr auto map_from_struct(const T& value) {
    // iterate over the members of T
    map_t result;
    map_from_struct(value, result);
    return result;
}

template <class T>
void to_struct(const map_t& value, T& result) {
    // iterate over the members of T
    for_each(refl::reflect(result).members, [&](auto member)
    {
        if constexpr (is_readable(member))
        {
            using member_type = std::decay_t<decltype(member(result))>;
            member(result) = std::get<member_type>(value.at(get_display_name(member)));
        }
    });
}

template <class T>
T to_struct(const map_t& value) {
    T result;
    to_struct(value, result);
    return result;
}

template <class T>
bool validate_map(const map_t& value, bool exact=false) {
    // Ensure that the map contains the members of the struct with the correct types.
    // iterate over the members of T
    T temp;
    if (exact && value.size() != readable_members<T>.size) return false;
    bool result = true;
    for_each(refl::reflect(temp).members, [&](auto member)
    {
        if constexpr (is_readable(member))
        {
            using member_type = std::decay_t<decltype(member(temp))>;
            // Does the map contain the key and hold the correct type?
            if (! value.count(get_display_name(member)) || 
                ! std::holds_alternative<member_type>(value.at(get_display_name(member))))
                result = false;
        }
    });
    return result;
}

class struct_wrapper_base {
  public:
    virtual void to_map(map_t& value) = 0;
    virtual map_t to_map() = 0;
    virtual size_t serialize(std::streambuf& sb) = 0;
    //virtual std::type_info& type_info() = 0;
    virtual ~struct_wrapper_base() {}
};

/*
I'm in trouble.  I need a way to convert from struct_wrapper_base back to a struct.  I was going to use a virtual template function,
but those aren't possible.  What are other options?

I can have a free template function
*/

template <typename T>
concept StructWrapperPtr =
    // Either a pointer or a smart pointer
    std::derived_from<std::remove_pointer_t<T>, struct_wrapper_base> || (SharedPtr<T> && std::derived_from<typename T::element_type, struct_wrapper_base>);


template <class T>
constexpr uint8_t pmtTypeIndex()
{
    if constexpr (std::same_as<T, std::monostate>)
        return 0;
    else if constexpr (std::same_as<T, bool>)
        return 1;
    else if constexpr (std::signed_integral<T>)
        return 2;
    else if constexpr (std::unsigned_integral<T>)
        return 3;
    else if constexpr (std::floating_point<T>)
        return 4;
    else if constexpr (Complex<T>)
        return 5;
    else if constexpr (std::same_as<T, std::string>)
        return 6;
    else if constexpr (std::ranges::range<T>) {
        if constexpr (UniformVector<T>) {
            return pmtTypeIndex<typename T::value_type>() << 4;
        }
        else {
            return 7; // for vector of PMTs
        }
    }
    else if constexpr (std::same_as<T, std::map<std::string, pmt>>)
        return 8;
    else if constexpr (StructWrapperPtr<T>)
        return 9;
    else if constexpr(std::same_as<T, serialized_struct>)
        return 10;
}

template <class T>
constexpr uint16_t serialId()
{
    if constexpr (Complex<T>) {
        return (pmtTypeIndex<T>() << 8) | sizeof(typename T::value_type);
    }
    else if constexpr (Scalar<T> || std::same_as<T, bool>) {
        static_assert(sizeof(T) < 32, "Can't serial data wider than 16 bytes");
        return (pmtTypeIndex<T>() << 8) | sizeof(T);
    }
    else if constexpr (UniformVector<T>) {
        static_assert(sizeof(typename T::value_type) < 32,
                      "Can't serial data wider than 16 bytes");
        return (pmtTypeIndex<T>() << 8) | sizeof(typename T::value_type);
    }
    else
        return pmtTypeIndex<T>() << 8;
}

// Forward decalaration so we can recursively serialize.
template <IsPmt P>
size_t serialize(std::streambuf& sb, const P& value);

template <class T>
struct serialInfo {
    using value_type = T;
    static constexpr uint16_t value = serialId<T>();
};

inline size_t _serialize_version(std::streambuf& sb) {
    return sb.sputn(reinterpret_cast<const char*>(&pmt_version), 2);
}

template <class T>
size_t _serialize_id(std::streambuf& sb) {
    using Td = std::decay_t<T>;
    auto id = serialInfo<Td>::value;
    return sb.sputn(reinterpret_cast<const char*>(&id), 2);
}

template <PmtVector T>
size_t _serialize(std::streambuf& sb, const T& arg) {
    auto length = _serialize_id<T>(sb);
    uint64_t sz = arg.size();
    length += sb.sputn(reinterpret_cast<const char*>(&sz), sizeof(uint64_t));
    for (auto& value: arg) {
        length += serialize(sb, value);
    }
    return length;
}

template <UniformBoolVector T>
size_t _serialize(std::streambuf& sb, const T& arg) {
    auto length = _serialize_id<T>(sb);
    uint64_t sz = arg.size();
    length += sb.sputn(reinterpret_cast<const char*>(&sz), sizeof(uint64_t));
    char one = 1;
    char zero = 0;
    for (auto value : arg) {
        sb.sputn(value ? &one : &zero, sizeof(char));
    }
    length += arg.size() * sizeof(char);
    return length;
}
template <UniformVector T>
size_t _serialize(std::streambuf& sb, const T& arg) {
    auto length = _serialize_id<T>(sb);
    uint64_t sz = arg.size();
    length += sb.sputn(reinterpret_cast<const char*>(&sz), sizeof(uint64_t));
    length += sb.sputn(reinterpret_cast<const char*>(arg.data()),
                       arg.size() * sizeof(arg[0]));
    return length;
}

template <PmtNull T>
size_t _serialize(std::streambuf& sb, const T& arg) {
    return _serialize_id<T>(sb);
}

template <Scalar T>
size_t _serialize(std::streambuf& sb, const T& arg) {
    auto length = _serialize_id<T>(sb);
    length += sb.sputn(reinterpret_cast<const char*>(&arg), sizeof(arg));
    return length;
}

template <std::same_as<std::shared_ptr<struct_wrapper_base>> T>
size_t _serialize(std::streambuf& sb, const T& arg) {
    auto length = _serialize_id<T>(sb);
    length += arg->serialize(sb);
    return length;
}

template <std::same_as<serialized_struct> T>
size_t _serialize(std::streambuf& sb, const T& arg) {
    auto length = _serialize_id<T>(sb);
    length += sb.sputn(arg.data(), arg.size());
    return length;
}

template <PmtMap T>
size_t _serialize(std::streambuf& sb, const T& arg) {
    auto length = _serialize_id<T>(sb);
    uint32_t nkeys = arg.size();
    length += sb.sputn(reinterpret_cast<const char*>(&nkeys), sizeof(nkeys));
    uint32_t ksize;
    for (const auto& [k, v] : arg) {
        // For right now just prefix the size to the key and send it
        ksize = k.size();
        length +=
            sb.sputn(reinterpret_cast<const char*>(&ksize), sizeof(ksize));
        length += sb.sputn(k.c_str(), ksize);
        length += serialize(sb, v);
    }
    return length;
}

// FIXME - make this consistent endianness
template <IsPmt P>
size_t serialize(std::streambuf& sb, const P& value)
{
    size_t length = _serialize_version(sb);

    std::visit(
        [&length, &sb](auto&& arg) {
            length += _serialize(sb, arg);
        },
        value);

    return length;
}

template <class T>
T _deserialize_val(std::streambuf& sb);

static pmt deserialize(std::streambuf& sb)
{
    uint16_t version;
    // pmt_container_type container;
    sb.sgetn(reinterpret_cast<char*>(&version), sizeof(version));
    // sb.sgetn(reinterpret_cast<char*>(&container), sizeof(container));

    uint16_t receivedId;
    sb.sgetn(reinterpret_cast<char*>(&receivedId), sizeof(receivedId));

    pmt ret;

    switch (receivedId) {
    case serialInfo<bool>::value:
        return _deserialize_val<bool>(sb);
    case serialInfo<uint8_t>::value:
        return _deserialize_val<uint8_t>(sb);
    case serialInfo<uint16_t>::value:
        return _deserialize_val<uint16_t>(sb);
    case serialInfo<uint32_t>::value:
        return _deserialize_val<uint32_t>(sb);
    case serialInfo<uint64_t>::value:
        return _deserialize_val<uint64_t>(sb);
    case serialInfo<int8_t>::value:
        return _deserialize_val<int8_t>(sb);
    case serialInfo<int16_t>::value:
        return _deserialize_val<int16_t>(sb);
    case serialInfo<int32_t>::value:
        return _deserialize_val<int32_t>(sb);
    case serialInfo<int64_t>::value:
        return _deserialize_val<int64_t>(sb);
    case serialInfo<float>::value:
        return _deserialize_val<float>(sb);
    case serialInfo<double>::value:
        return _deserialize_val<double>(sb);
    case serialInfo<std::complex<float>>::value:
        return _deserialize_val<std::complex<float>>(sb);
    case serialInfo<std::complex<double>>::value:
        return _deserialize_val<std::complex<double>>(sb);

    // case serialInfo<std::vector<bool>>::value: return
    // _deserialize_val<std::vector<bool>>(sb);
    case serialInfo<std::vector<uint8_t>>::value:
        return _deserialize_val<std::vector<uint8_t>>(sb);
    case serialInfo<std::vector<uint16_t>>::value:
        return _deserialize_val<std::vector<uint16_t>>(sb);
    case serialInfo<std::vector<uint32_t>>::value:
        return _deserialize_val<std::vector<uint32_t>>(sb);
    case serialInfo<std::vector<uint64_t>>::value:
        return _deserialize_val<std::vector<uint64_t>>(sb);
    case serialInfo<std::vector<int8_t>>::value:
        return _deserialize_val<std::vector<int8_t>>(sb);
    case serialInfo<std::vector<int16_t>>::value:
        return _deserialize_val<std::vector<int16_t>>(sb);
    case serialInfo<std::vector<int32_t>>::value:
        return _deserialize_val<std::vector<int32_t>>(sb);
    case serialInfo<std::vector<int64_t>>::value:
        return _deserialize_val<std::vector<int64_t>>(sb);
    case serialInfo<std::vector<float>>::value:
        return _deserialize_val<std::vector<float>>(sb);
    case serialInfo<std::vector<double>>::value:
        return _deserialize_val<std::vector<double>>(sb);
    case serialInfo<std::vector<std::complex<float>>>::value:
        return _deserialize_val<std::vector<std::complex<float>>>(sb);
    case serialInfo<std::vector<std::complex<double>>>::value:
        return _deserialize_val<std::vector<std::complex<double>>>(sb);

    case serialInfo<std::string>::value:
        return _deserialize_val<std::string>(sb);

    case serialInfo<map_t>::value:
        return _deserialize_val<map_t>(sb);
    case serialInfo<serialized_struct>::value:
        return _deserialize_val<serialized_struct>(sb);
    default:
        throw std::runtime_error("pmt::deserialize: Invalid PMT type type");
    }

    return ret;
}

template <class T>
T _deserialize_val(std::streambuf& sb)
{
    if constexpr (Scalar<T>) {
        T val;
        sb.sgetn(reinterpret_cast<char*>(&val), sizeof(val));
        return val;
    }
    else if constexpr (UniformVector<T> && !String<T>) {
        uint64_t sz;
        sb.sgetn(reinterpret_cast<char*>(&sz), sizeof(uint64_t));
        std::vector<typename T::value_type> val(sz);
        sb.sgetn(reinterpret_cast<char*>(val.data()), sz * sizeof(val[0]));
        return val;
    }
    else if constexpr (String<T>) {
        uint64_t sz;
        sb.sgetn(reinterpret_cast<char*>(&sz), sizeof(uint64_t));
        std::string val(sz, '0');
        sb.sgetn(reinterpret_cast<char*>(val.data()), sz);
        return val;
    }
    else if constexpr (PmtMap<T>) {
        map_t val;

        uint32_t nkeys;
        sb.sgetn(reinterpret_cast<char*>(&nkeys), sizeof(nkeys));
        for (uint32_t n = 0; n < nkeys; n++) {
            uint32_t ksize;
            sb.sgetn(reinterpret_cast<char*>(&ksize), sizeof(ksize));
            std::vector<char> data;
            data.resize(ksize);
            sb.sgetn(data.data(), ksize);

            val[std::string(data.begin(), data.end())] = deserialize(sb);
        }
        return val;
    }
    else if constexpr(std::same_as<T, serialized_struct>) {
        uint64_t sz;
        sb.sgetn(reinterpret_cast<char*>(&sz), sizeof(uint64_t));
        std::vector<char> val(sz, '0');
        sb.sgetn(reinterpret_cast<char*>(val.data()), sz);
        return serialized_struct(std::move(val));
    }
    else {
        throw std::runtime_error(
            "pmt::_deserialize_value: attempted to deserialize invalid PMT type");
    }
}

template <class T>
class struct_wrapper : public struct_wrapper_base {
  private:
    T _data;

  public:
    struct_wrapper(const T& data) : _data(data) {}
    void to_map(map_t& value) {
        map_from_struct(_data, value);
    }
    map_t to_map() {
        return map_from_struct(_data);
    }
    /*std::type_info& type_info() { return typeid(T); }
    template <class U>
    U& get() {
        static_assert(std::type_info<U> == std::type_info<T>);
        return _data;
    }*/
    size_t serialize(std::streambuf& sb) {
        // iterate over the members of T
        size_t length = _serialize_version(sb);
        length += _serialize_id<serialized_struct>(sb);
        uint32_t nkeys = readable_members<T>.size;
        length += sb.sputn(reinterpret_cast<const char*>(&nkeys), sizeof(nkeys));
        uint32_t ksize;
        for_each(refl::reflect(_data).members, [&](auto member)
        {
            if constexpr (is_readable(member))
            {
                using member_type = std::decay_t<decltype(member(_data))>;
                auto k = get_display_name(member);
                length += sb.sputn(reinterpret_cast<const char*>(&ksize), sizeof(ksize));
                length += sb.sputn(k, ksize);
                
                auto id = serialInfo<member_type>::value;
                length += sb.sputn(reinterpret_cast<const char*>(&id), 2);
                length += _serialize(sb, member(_data));
            }
        });
        return length;
    }
};

template <class T>
pmt pmt_from_struct(const T& value) {
    return pmt(std::make_shared<struct_wrapper<T>>(value));
}

/*template <class T>
void struct_from_pmt(const pmt& value, T& result) {
    std::visit(
        [](const auto& arg, T& result) -> void {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (StructWrapperPtr T>)
                result = 
            else if constexpr (std::same_as<T, serialized_struct)
                return sizeof(typename T::value_type);
            return sizeof(T);
        },
        value.get_base());
}*/


template <IsPmt P>
std::string to_base64(const P& value)
{
    std::stringbuf sb;
    auto nbytes = serialize(sb, value);
    std::string pre_encoded_str(nbytes, '0');
    sb.sgetn(pre_encoded_str.data(), nbytes);
    auto nencoded_bytes = Base64encode_len(nbytes);
    std::string encoded_str(nencoded_bytes, '0');
    auto nencoded = Base64encode(encoded_str.data(), pre_encoded_str.data(), nbytes);
    encoded_str.resize(nencoded - 1); // because it null terminates
    return encoded_str;
}

[[maybe_unused]] static pmt from_base64(const std::string& encoded_str)
{
    std::string bufplain(encoded_str.size(), '0');
    Base64decode(bufplain.data(), encoded_str.data());
    std::stringbuf sb(bufplain);
    return deserialize(sb);
}

// Allows us to cast from a pmt like this: auto x = cast<float>(mypmt);
template <class T, IsPmt P>
T cast(const P& value)
{
    return std::visit(
        [](const auto& arg) -> T {
            using U = std::decay_t<decltype(arg)>;
            if constexpr (std::constructible_from<T, U>)
                return T(arg);
            // else if constexpr (PmtMap<T> && PmtMap<U>) {
            //     return std::get<std::map<std::string, pmt_var_t>>(arg);
            // }
            else
                throw std::runtime_error(fmt::format(
                    "Invalid PMT Cast {} {}", typeid(T).name(), typeid(U).name()));
        },
        value);
}

} // namespace pmtv

namespace fmt {
template <>
struct formatter<pmtv::map_t::value_type> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const pmtv::map_t::value_type& kv, FormatContext& ctx) {
        return format_to(ctx.out(), "{}: {}", kv.first, kv.second);
    }
};

template <pmtv::Complex C>
struct formatter<C> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const C& arg, FormatContext& ctx) {
        if (arg.imag() >= 0)
            return format_to(ctx.out(), "{0}+j{1}", arg.real(), arg.imag());
        else
            return format_to(ctx.out(), "{0}-j{1}", arg.real(), -arg.imag());
    }
};


template<pmtv::IsPmt P>
struct formatter<P>
{

    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const P& value, FormatContext& ctx);
};

template<pmtv::IsPmt P>
template <typename FormatContext>
auto formatter<P>::format(const P& value, FormatContext& ctx) {
    // Due to an issue with the c++ spec that has since been resolved, we have to do something
    // funky here.  See 
    // https://stackoverflow.com/questions/37526366/nested-constexpr-function-calls-before-definition-in-a-constant-expression-con
    // This problem only appears to occur in gcc 11 in certain optimization modes. The problem
    // occurs when we want to format a vector<pmt>.  Ideally, we can write something like:
    //      return fmt::format_to(ctx.out(), "[{}]", fmt::join(arg, ", "));
    // However, due to the above issue, it fails to compile.  So we have to do the equivalent
    // ourselves.  We can't recursively call the formatter, but we can recursively call a lambda
    // function that does the formatting.
    // It gets more complicated, because we need to pass the function into the lambda.  We can't
    // pass in the lamdba as it is defined, so we create a nested lambda.  Which accepts a function
    // as a argument.
    // Because we are calling std::visit, we can't pass non-variant arguments to the visitor, so we
    // have to create a new nested lambda every time we format a vector to ensure that it works.
    using namespace pmtv;
    auto format_func = [&ctx](const auto arg) {
        auto function = [&ctx](const auto arg, auto function) {
        using namespace pmtv;
        using T = std::decay_t<decltype(arg)>;
        if constexpr (Scalar<T> || Complex<T>)
            return fmt::format_to(ctx.out(), "{}", arg);
        else if constexpr (std::same_as<T, std::string>)
            return fmt::format_to(ctx.out(), "{}",  arg);
        else if constexpr (UniformVector<T>)
            return fmt::format_to(ctx.out(), "[{}]", fmt::join(arg, ", "));
        else if constexpr (std::same_as<T, std::vector<pmt>>) {
            fmt::format_to(ctx.out(), "[");
            auto new_func = [&function](const auto arg) { function(arg, function); };
            for (auto& a: std::span(arg).first(arg.size()-1)) {
                std::visit(new_func, a);
                fmt::format_to(ctx.out(), ", ");
            }
            std::visit(new_func, arg[arg.size()-1]);
            return fmt::format_to(ctx.out(), "]");
            // When we drop support for gcc11, get rid of the nested lambda and replace
            // the above with this line.
            //return fmt::format_to(ctx.out(), "[{}]", fmt::join(arg, ", "));
        } else if constexpr (PmtMap<T>)
            return fmt::format_to(ctx.out(), "{{{}}}", fmt::join(arg, ", "));
        else if constexpr (std::same_as<std::monostate, T>)
            return fmt::format_to(ctx.out(), "null");
        return fmt::format_to(ctx.out(), "unknown type {}", typeid(T).name());
        };
        return function(arg, function);
    };
    return std::visit(format_func, value);
    }
} // namespace fmt

namespace pmtv {
    template <IsPmt P>
    std::ostream& operator<<(std::ostream& os, const P& value) {
        os << fmt::format("{}", value);
        return os;
    }
}

