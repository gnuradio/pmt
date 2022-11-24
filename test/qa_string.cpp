/*-*-c++-*-*/
/*
 * Copyright 2021 John Sallay
 *
 * SPDX-License-Identifier: LGPL-3.0
 *
 */
#include <gtest/gtest.h>

#include <pmtv/pmt.hpp>

#include <fmt/core.h>
#include <string>

using namespace pmtv;


TEST(PmtString, Constructor)
{
    // Empty Constructor
    pmt empty_vec{ std::string() };
    EXPECT_EQ(std::get<std::string>(empty_vec).size(), 0);

    std::string s1{ "hello world" };

    auto p = pmt(s1);

    auto s2 = pmtv::cast<std::string>(p);

    EXPECT_TRUE(s1 == s2);
}

TEST(PmtString, Serialization)
{
    std::string s1{ "hello world" };

    auto x = pmt(s1);

    std::stringbuf sb;
    pmtv::serialize(sb, x);
    auto y = pmtv::deserialize(sb);
    EXPECT_EQ(x == y, true);
}
