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
#include <pmtf/scalar.hpp>
#include <pmtf/wrap.hpp>
#include <sstream>

using namespace pmtf;

TEST(PmtNull, Construction) {
    null empty;
    pmt also_empty;
    EXPECT_EQ(empty, also_empty);
    scalar x(4);
    EXPECT_EQ(empty == x, false);
}

TEST(PmtNull, Assignment) {
    // We should be able to assign to default constructed pmt
    pmt x;
    x = 4;
}
