#include <pmtv/refl.hpp>
#include <pmtv/pmt.hpp>

namespace pmtv {

template <typename T>
constexpr auto readable_members = filter(refl::member_list<T>{}, [](auto member) { return is_readable(member); });

// Functions for converting between pmt stuctures and maps.
template <class T>
constexpr auto from_struct(const T& value) {
    // iterate over the members of T
    map_t result;
    for_each(refl::reflect(value).members, [&](auto member)
    {
        if constexpr (is_readable(member))
        {
            result[get_display_name(member)] = member(value);
        }
    });
    return result;
}

template <class T>
constexpr auto from_struct_faster(const T& value) {
    //using pair_t = std::pair<const std::string, const pmt>;
    using pair_t = std::pair<std::string, pmt>;
    /*const auto temp = map_to_array<pair_t>(refl::reflect(value).members, [&](auto member)
    {
        if constexpr (is_readable(member))
        {
            return pair_t{get_display_name(member), member(value)};
        }

    });
    return std::vector<pair_t>(temp.begin(), temp.end());*/
    std::vector<pair_t> result(readable_members<T>.size);
    size_t i = 0;
    for_each(refl::reflect(value).members, [&](auto member)
    {
        if constexpr (is_readable(member))
        {
            result[i++] = {get_display_name(member), member(value)};
        }
    });
    return result;

}

template <class T>
const auto from_struct_faster2(const T& value) {
    std::vector<pmt> result(readable_members<T>.size);
    size_t i = 0;
    for_each(refl::reflect(value).members, [&](auto member)
    {
        if constexpr (is_readable(member))
        {
            result[i++] = member(value);
        }
    });
    return result;

}

template <class T>
T to_struct(const map_t& value) {
    // iterate over the members of T
    T result;
    for_each(refl::reflect(result).members, [&](auto member)
    {
        if constexpr (is_readable(member))
        {
            using member_type = std::decay_t<decltype(member(result))>;
            member(result) = std::get<member_type>(value.at(get_display_name(member)));
        }
    });
    return result;
}

// Put in a vector of std::pairs instead of a dictionary.  We end up with much faster
// construction time, but it would be much slower to search.
template <class T>
T to_struct_faster(const std::vector<std::pair<std::string, pmt>>& value) {
    // iterate over the members of T
    T result;
    size_t i = 0;
    for_each(refl::reflect(result).members, [&](auto member)
    {
        if constexpr (is_readable(member))
        {
            using member_type = std::decay_t<decltype(member(result))>;
            member(result) = std::get<member_type>(value[i++].second);
        }
    });
    return result;
}

// Convert struct to a vector of values.  We loose the key information, but each pmt is typed so
// we still have the type and values.
// This is faster than to_struct_faster, because we only produce one array.
template <class T>
T to_struct_faster2(const std::vector<pmt>& value) {
    // iterate over the members of T
    T result;
    size_t i = 0;
    for_each(refl::reflect(result).members, [&](auto member)
    {
        if constexpr (is_readable(member))
        {
            using member_type = std::decay_t<decltype(member(result))>;
            member(result) = std::get<member_type>(value[i++]);
        }
    });
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

}