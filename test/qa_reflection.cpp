/*-*-c++-*-*/
/*
 * Copyright 2023 John Sallay
 *
 * SPDX-License-Identifier: LGPL-3.0
 *
 */
#include <gtest/gtest.h>

#include <pmtv/pmt.hpp>

#include <fmt/core.h>
#include <string>

using namespace pmtv;

struct qqq {
    int x;
    float y;
    double z;
};

REFL_AUTO(type(qqq), field(x), field(y), field(z));


TEST(PmtRefl, Constructor)
{
    qqq x{1, 2.0, 4.1};
    auto xx = map_from_struct(x);
    auto yy = to_struct<qqq>(xx);
    EXPECT_EQ(x.x, yy.x);
    EXPECT_EQ(x.y, yy.y);
    EXPECT_EQ(x.z, yy.z);
}

TEST(PmtRefl, Serialize)
{
    qqq x{1, 2.0, 4.1};
    auto xx = map_from_struct(x);
    std::stringbuf sb;
    pmtv::serialize(sb, pmt(xx));
    auto y = pmtv::deserialize(sb);
    auto yy = to_struct<qqq>(pmtv::get_map(y));
    EXPECT_EQ(x.x, yy.x);
    EXPECT_EQ(x.y, yy.y);
    EXPECT_EQ(x.z, yy.z);
}


