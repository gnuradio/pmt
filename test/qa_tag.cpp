/*-*-c++-*-*/
/*
 * Copyright 2021 John Sallay
 *
 * SPDX-License-Identifier: LGPL-3.0
 *
 */
#include <gtest/gtest.h>
#include <complex>

#include <pmtf/base.hpp>
#include <pmtf/vector.hpp>
#include <pmtf/scalar.hpp>
#include <pmtf/tag.hpp>
#include <pmtf/wrap.hpp>

using namespace pmtf;

TEST(PmtTag, EmptyTag) {
    tag empty;
    empty["abc"] = scalar(4);
}

TEST(PmtTag, PmtTagConstruct)
{
    tag ta(156, 0, "abc", 43);
    tag tb(156,100, {{"abc", 43}});
    tag tc = tb;
    EXPECT_EQ(tb, tc);
}

/*TEST(PmtMap, MapSerialize)
{
    std::complex<float> val1(1.2, -3.4);
    std::vector<int32_t> val2{ 44, 34563, -255729, 4402 };

    // Create the PMT map
    std::map<std::string, pmt> input_map({
        { "key1", val1 },
        { "key2", val2 },
    });
    map map_pmt(input_map);
    std::stringbuf sb;
    map_pmt.get_pmt_buffer().serialize(sb);
    auto y = pmt::deserialize(sb);
    std::cout << map_pmt << std::endl;
    std::cout << map_pmt.size() << " " << map_pmt.size2() << std::endl;
    std::cout << y << std::endl;
    std::cout << "Before Test\n";
    std::cout << (map_pmt == y) << std::endl;
    EXPECT_EQ(map_pmt, y);

}

TEST(PmtMap, get_as)
{
    std::complex<float> val1(1.2, -3.4);
    std::vector<int32_t> val2{ 44, 34563, -255729, 4402 };

    // Create the PMT map
    std::map<std::string, pmt> input_map({
        { "key1", val1 },
        { "key2", val2 },
    });
    pmt x = input_map;
    // Make sure that we can get the value back out
    auto y = get_as<std::map<std::string, pmt>>(x);
    EXPECT_EQ(x, y);

    // Throw an error for other types.
    EXPECT_THROW(get_as<float>(x), ConversionError);
    
}

TEST(PmtMap, base64)
{
    std::complex<float> val1(1.2, -3.4);
    std::vector<int32_t> val2{ 44, 34563, -255729, 4402 };

    // Create the PMT map
    std::map<std::string, pmt> input_map({
        { "key1", val1 },
        { "key2", val2 },
    });
    pmt x = input_map;
    
    // Make sure that we can get the value back out
    auto encoded_str = pmt(x).to_base64();
    auto y = pmt::from_base64(encoded_str);

    EXPECT_EQ(x, y);
}*/
