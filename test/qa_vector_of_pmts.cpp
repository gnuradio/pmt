/*-*-c++-*-*/
/*
 * Copyright 2021 John Sallay
 *
 * SPDX-License-Identifier: LGPL-3.0
 *
 */
#include <gtest/gtest.h>
#include <complex>

#include <list>
#include <map>

#include <pmtv/pmt.hpp>

#include <fmt/core.h>

using namespace pmtv;

/*
How to do the map wrapper???
0) Derived class
    Has all the same methods.
    Works anywhere a pmt is needed.
    Need to use pointers, which is a little different than what I'm doing.
    Allows for custom constructors.
1) View class - Get a mutable view into a pmt that acts as a map.
2) Helper class - Just like fb pmts.

What does it need to do???
Construct from an initializer list.
Range based for loop.
operator[] (lookup and add)
Cheap copies (could be moves)
*/

TEST(PmtVectorPmt, Constructor)
{
    // Empty Constructor
    pmt empty_vec{ std::vector<pmt>() };
    EXPECT_EQ(std::get<std::vector<pmt>>(empty_vec).size(), 0);
    pmt il_vec{ std::vector<pmt>{1.0, 2, "abc"}};
    std::vector<pmt> vec;
    vec.push_back(pmt(1));
    vec.push_back(pmt(std::vector<uint32_t>{ 1, 2, 3 }));

    auto p = pmt(vec);

    auto vec2 = pmtv::get_vector<pmt>(p);

    EXPECT_TRUE(vec[0] == vec2[0]);
    EXPECT_TRUE(vec[1] == vec2[1]);
}

TEST(PmtVectorPmt, fmt)
{
    std::vector<pmt> vec;
    vec.push_back(pmt(1));
    vec.push_back(pmt(std::vector<uint32_t>{ 1, 2, 3 }));
    EXPECT_EQ(fmt::format("{}", pmt(vec)), fmt::format("[{}]", fmt::join(vec, ", ")));

}
