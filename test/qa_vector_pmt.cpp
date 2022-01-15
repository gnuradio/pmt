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
#include <pmtf/map.hpp>
#include <pmtf/wrap.hpp>

using namespace pmtf;

TEST(PmtVectorPmt, EmptyVector) {
    vector<pmt> empty;
}

TEST(PmtVectorPmt, VectorResize) {
    vector<pmt> empty;
    empty.resize(5);
}

TEST(PmtVectorPmt, Constructors) {
    // Sized
    vector<pmt> sized(10);
    EXPECT_EQ(sized.size(), 10);

    // Filled
    vector<pmt> filled(5, 2.0f);
    EXPECT_EQ(filled.size(), 5);
    for (auto& value: filled) EXPECT_EQ(value, 2.0f);

    // Range
    vector<pmt> range(filled.begin(), filled.end());
    EXPECT_EQ(range, filled);

    // Copy from std::vector
    std::vector<pmt> std_vec{5, string("abc")};
    vector<pmt> from_std(std_vec);
    //std::cout << std_vec << std::endl;
    //std::cout << from_std << std::endl;
    EXPECT_EQ(std_vec, from_std);

    // Copy from pmtf::vector
    vector<pmt> copied(filled);
    EXPECT_EQ(copied, filled);

    // Initializer list
    vector<pmt> il{5, "abc", std::complex<float>(1.0, -2.0), scalar<int>(3), pmt("z")};
    EXPECT_EQ(il.size(), 5);
    EXPECT_EQ(il[0], 5);
    EXPECT_EQ(il[1], "abc");
    EXPECT_EQ(il[2], std::complex<float>(1.0, -2.0));
    EXPECT_EQ(il[3], scalar<int>(3));
    EXPECT_EQ(il[4], pmt("z"));
}

/*TEST(PmtMap, PmtMapTests)
{
    std::complex<float> val1(1.2, -3.4);
    std::vector<int32_t> val2{ 44, 34563, -255729, 4402 };

    // Create the PMT map
    std::map<std::string, pmt> input_map({
        { "key1", val1 },
        { "key2", val2 },
    });
    map map_pmt(input_map);

    // Lookup values in the PMT map and compare with what was put in there
    auto vv1 = map_pmt["key1"];
    std::cout << vv1 << std::endl;
    EXPECT_EQ(get_scalar<std::complex<float>>(vv1), val1);

    auto vv2 = map_pmt["key2"];
    EXPECT_EQ(vv2 == val2, true);
    std::cout << map_pmt << std::endl;
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
    
}*/

