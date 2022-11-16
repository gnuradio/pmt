/*-*-c++-*-*/
/*
 * Copyright 2021 John Sallay
 *
 * SPDX-License-Identifier: LGPL-3.0
 *
 */
#include <gtest/gtest.h>
#include <complex>

#include <pmtv/map.hpp>
#include <pmtv/uniform_vector.hpp>

using namespace pmtv;

TEST(PmtMap, EmptyMap) {
    map empty;
    empty["abc"] = pmt(uint64_t(4)); 
    empty["xyz"] = pmt(std::vector<double>{1,2,3,4,5}); 
}


TEST(PmtMap, PmtMapTests)
{
    std::complex<float> val1(1.2, -3.4);
    std::vector<int32_t> val2{ 44, 34563, -255729, 4402 };

    // Create the PMT map
    std::map<std::string, _pmt_storage_base> input_map({
        { "key1", val1 },
        { "key2", val2 },
    });
    map map_pmt(input_map);
    std::cout << map_pmt << std::endl;
    std::cout << pmt(val1) << std::endl;

    // Lookup values in the PMT map and compare with what was put in there
    pmt vv1 = map_pmt["key1"];
    std::cout << std::complex<float>(vv1) << std::endl;
    std::cout << "Before" << std::endl;
    std::cout << vv1 << std::endl;
    EXPECT_TRUE(std::complex<float>(vv1) == val1);

    auto vv2 = map_pmt["key2"];
    EXPECT_TRUE(uniform_vector<int32_t>(vv2) == val2);
    std::cout << map_pmt << std::endl;
}
#if 0
TEST(PmtMap, MapSerialize)
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
}
#endif
