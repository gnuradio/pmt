/*-*-c++-*-*/
/*
 * Copyright 2021 John Sallay
 *
 * SPDX-License-Identifier: LGPL-3.0
 *
 */
#include <gtest/gtest.h>
#include <complex>
#include <string>
#include <string_view>

#include <pmtv/pmt.hpp>
#include <pmtv/format.hpp>

using namespace pmtv;

TEST(PmtMap, EmptyMap)
{
    auto empty = pmt(map_t{});
    auto v = get_map(empty);
    v["abc"] = pmt(uint64_t(4));
    v["xyz"] = pmt(std::vector<double>{ 1, 2, 3, 4, 5 });

    using namespace std::literals;
    using namespace std::string_literals;
    using namespace std::string_view_literals;
    auto keyStringLiteral = "abc";
    std::string keyString = keyStringLiteral;
    std::string_view keyStringView = keyStringLiteral;
    EXPECT_TRUE(std::get<uint64_t>(v.at(keyString)) == uint64_t(4));
    EXPECT_TRUE(std::get<uint64_t >(v.find(keyString)->second) == uint64_t(4));
    EXPECT_TRUE(std::get<uint64_t>(v.find(keyStringView)->second) == uint64_t(4));
    EXPECT_TRUE(std::get<uint64_t>(v.find(keyStringLiteral)->second) == uint64_t(4));
    EXPECT_TRUE(std::get<uint64_t>(v.at("abc"s)) == uint64_t(4));
    EXPECT_TRUE(std::get<uint64_t>(v.at(keyStringLiteral)) == uint64_t(4));
    // EXPECT_TRUE(v.at("abc"sv) == pmt(uint64_t(4))) -- not allowed by the ISO-C++ standard
    // map::at(T key) checks for exact key and its type 'T'
    // wrapping of std::string_view with std::string is required by ISO-C++ design of map::at(..)
    EXPECT_TRUE(v.at(std::string(keyStringView)) == pmt(uint64_t(4))); // <- this is OK
}


TEST(PmtMap, PmtMapTests)
{
    std::complex<float> val1(1.2f, -3.4f);
    std::vector<int32_t> val2{ 44, 34563, -255729, 4402 };

    // Create the PMT map
    pmtv::map_t input_map({
        { "key1", val1 },
        { "key2", val2 },
    });

    pmt map_pmt = input_map;
    std::cout << map_pmt << std::endl;
    std::cout << pmt(val1) << std::endl;

    // Lookup values in the PMT map and compare with what was put in there
    pmt vv1 = get_map(map_pmt)["key1"];
    std::cout << std::get<std::complex<float>>(vv1) << std::endl;
    std::cout << "Before" << std::endl;
    std::cout << vv1 << std::endl;
    EXPECT_TRUE(std::get<std::complex<float>>(vv1) == val1);

    auto vv2 = get_map(map_pmt)["key2"];
    EXPECT_TRUE(get_vector<int32_t>(vv2) == val2);
    std::cout << map_pmt << std::endl;
}

TEST(PmtMap, MapSerialize)
{
    std::complex<float> val1(1.2f, -3.4f);
    std::vector<int32_t> val2{ 44, 34563, -255729, 4402 };

    // Create the PMT map
    map_t input_map({
        { "key1", val1 },
        { "key2", val2 },
    });
    pmt map_pmt(input_map);
    std::stringbuf sb;
    serialize(sb, map_pmt);
    auto y = pmtv::deserialize(sb);
    auto z = std::get<map_t>(y);
    EXPECT_TRUE(map_pmt == y);
}

TEST(PmtMap, get_as)
{
    std::complex<float> val1(1.2f, -3.4f);
    std::vector<int32_t> val2{ 44, 34563, -255729, 4402 };

    // Create the PMT map
    pmtv::map_t input_map({
        { "key1", val1 },
        { "key2", val2 },
    });
    auto x = pmt(input_map);
    // Make sure that we can get the value back out
    // auto y = std::map<std::string, pmt, std::less<>>(x);
    auto y = get_map(x);
    EXPECT_EQ(x == y, true);

    // Throw an error for other types.
    // EXPECT_ANY_THROW(float(x));
}

TEST(PmtMap, base64)
{
    std::complex<float> val1(1.2f, -3.4f);
    std::vector<int32_t> val2{ 44, 34563, -255729, 4402 };

    // Create the PMT map
    pmtv::map_t input_map({
        { "key1", val1 },
        { "key2", val2 },
    });
    pmt x = input_map;

    // Make sure that we can get the value back out
    auto encoded_str = pmtv::to_base64(x);
    auto y = pmtv::from_base64(encoded_str);

    EXPECT_TRUE(x == y);
}

TEST(PmtMap, fmt)
{
    std::complex<float> val1(1.2f, -3.4f);
    std::vector<int32_t> val2{ 44, 34563, -255729, 4402 };

    // Create the PMT map
    pmtv::map_t input_map({
        { "key1", val1 },
        { "key2", val2 },
    });
    pmt x = input_map;
    EXPECT_EQ(fmt::format("{}", x), fmt::format("{{{}}}", fmt::join(input_map, ", ")));

}
// #endif
