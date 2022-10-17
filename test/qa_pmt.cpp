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
#include <pmtf/map.hpp>
#include <pmtf/scalar.hpp>
#include <pmtf/string.hpp>
#include <pmtf/vector.hpp>
#include <pmtf/wrap.hpp>

#include <pmtv/base.hpp>

using namespace pmtf;

TEST(Pmt, get_as) {
    pmt x = 4.0f;
    auto y = get_as<float>(x);
    EXPECT_EQ(y, x);

    pmt z = vector<float>{1.2, 3.6};
    auto zz = get_as<std::vector<float>>(z);
    EXPECT_EQ(zz, z);
}
// // TEST(Pmt, BasicPmtTests)
// // {
// //     std::complex<float> cplx_val = std::complex<float>(1.2, -3.4);
// //     auto x = scalar(cplx_val);

// //     EXPECT_EQ(x, cplx_val);
// //     EXPECT_EQ(x.data_type(), Data::ScalarComplex64);

// //     std::vector<int32_t> int_vec_val{ 5, 9, 23445, 63, -25 };
// //     auto int_pmt_vec = vector<int32_t>(int_vec_val);
// //     EXPECT_EQ(int_pmt_vec, int_vec_val);
// //     EXPECT_EQ(int_pmt_vec.data_type(), Data::VectorInt32);

// //     std::vector<std::complex<float>> cf_vec_val{ { 0, 1 }, { 2, 3 }, { 4, 5 } };
// //     auto cf_pmt_vec = vector<std::complex<float>>(cf_vec_val);
// //     EXPECT_EQ(cf_pmt_vec, cf_vec_val);
// //     EXPECT_EQ(cf_pmt_vec.data_type(), Data::VectorComplex64);
// // }

// TEST(Pmt, PmtScalarValueTests)
// {
//     auto x = scalar_value<int>(4);
//     EXPECT_EQ(x, 4);
//     scalar_value<int> y(4);
//     EXPECT_EQ(x, y);
//     x = 5;
//     EXPECT_EQ(x, 5.0);
//     y = x;
//     EXPECT_EQ(x, y);
//     scalar_value<int> z = 4;
//     int a = x.value();
//     EXPECT_EQ(a, 5.0);
// }

// TEST(Pmt, PmtScalarTests) {
//     auto x = scalar<int>(4);
//     EXPECT_EQ(x, 4);
//     scalar<int> y(4);
//     EXPECT_EQ(x, y);
//     x = 5;
//     EXPECT_EQ(x, 5.0);
//     y = x;
//     EXPECT_EQ(x, y);
//     scalar<int> z = 4;
//     int a = int(x);
//     EXPECT_EQ(a, 5.0);
//     float b = float(x);
//     EXPECT_EQ(b, 5.0);
// }

// TEST(Pmt, PmtVectorValueTests) {
//     auto x = pmt_vector_value<std::complex<float>>({{1, -1}, {2.1, 3.0}});
//     std::vector<std::complex<float>> y({{1, -1}, {2.1, 3.0}});
//     EXPECT_EQ(x == y, true );
//     auto z =  pmt_vector_value<std::complex<float>>(y);
//     auto a =  pmt_vector_value<std::complex<float>>(x);
// }

// TEST(Pmt, PmtVectorTests) {
//     // Init from values
//     auto x = vector<std::complex<float>>({{1, -1}, {2.1, 3.0}});
//     std::vector<std::complex<float>> y({{1, -1}, {2.1, 3.0}});
//     // Do they equal each other
//     EXPECT_EQ(x, y );
//     // Init from vector
//     auto z =  vector<std::complex<float>>(y);
//     // Copy constructor
//     auto a =  vector<std::complex<float>>(x);
//     // Assignment operator
//     a = x;
//     // TODO: Add in Move contstructor
//     // Make sure we can do a range based for loop
//     for (auto& xx : x) {
//         xx += 1;
//     }
//     // Print
//     std::cout << x << std::endl;
// }

TEST(Pmt, asdf)
{
    auto x = scalar<uint8_t>(4);
    std::cout << int(x.value()) << std::endl;
    x = 5;
    std::cout << int(x.value()) << std::endl;
    auto y = vector<float>(std::vector<float>{1,2,3});
    for (const auto& zz : y) {
        std::cout << zz << std::endl;
    }
    y.value()[2] = 4.5;
    std::cout << y.value()[2] << std::endl;
    //std::cout << y << std::endl;
    pmt z = x;

    map m;
    m["abc"] = x;
    //m["def"] = y;
    //std::cout << m << std::endl;
    //for (const auto& [k, v]: m) {
    //    std::cout << k << ": " << v << std::endl;
    //}
}

// //     EXPECT_EQ(str_pmt, "hello");
// // }


