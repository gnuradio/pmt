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

TEST(PmtTag, Serialize)
{
    tag tb(156,100, {{"abc", 43}});

    std::stringbuf sb;
    pmt(tb).serialize(sb);
    auto y = pmt::deserialize(sb);
    EXPECT_EQ(tb, y);

}

TEST(PmtTag, base64)
{
    tag x(156,100, {{"abc", 43}});
    
    // Make sure that we can get the value back out
    auto encoded_str = pmt(x).to_base64();
    auto y = pmt::from_base64(encoded_str);

    EXPECT_EQ(x, y);
}
