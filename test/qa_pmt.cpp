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

using namespace pmtf;

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

// // TEST(Pmt, PmtStringTests)
// // {
// //     auto str_pmt = string("hello");
// //     std::cout << str_pmt << std::endl;

// //     EXPECT_EQ(str_pmt, "hello");
// // }


// /*TEST(Pmt, PmtMapTests)
// {
//     std::complex<float> val1(1.2, -3.4);
//     std::vector<int32_t> val2{ 44, 34563, -255729, 4402 };

//     // Create the PMT map
//     std::map<std::string, wrap> input_map({
//         { "key1", val1 },
//         { "key2", val2 },
//     });
//     auto map_pmt = map<std::string>(input_map);

//     // Lookup values in the PMT map and compare with what was put in there
//     auto vv1 = map_pmt["key1"];
//     std::cout << vv1 << std::endl;
//     EXPECT_EQ(get_scalar<std::complex<float>>(vv1), val1);

//     auto vv2 = map_pmt["key2"];
//     EXPECT_EQ(vv2, val2);
// }*/

// TEST(Pmt, PmtWrap)
// {
//     wrap x = int8_t(4);
//     get_scalar<int8_t>(x);
//     get_scalar<int8_t>(x);
// }
// TEST(pmt, CanBe)
// {
//     wrap x = int32_t(257);
//     assert(can_be<int8_t>(x) == true);
//     assert(can_be<uint64_t>(x) == true);
//     assert(can_be<float>(x) == true);
//     assert(can_be<std::complex<double>>(x) == true);
//     assert(can_be<bool>(x) == true);
//     assert(can_be<std::string>(x) == false);
//     assert(can_be<scalar<int32_t>>(x) == true);
//     assert(can_be<scalar<int8_t>>(x) == true);
// }

TEST(Pmt, MapWrapper) {
    map<std::string> x;
    x["abc"] = 4;
    x["qwer"] = std::vector<int>{1,2,4};
    for (auto& [key, value]: x) {
        std::cout << key << std::endl;
    }

    wrap w(x);
    auto w_as_map = get_map<std::string>(w);

    auto item1 = w_as_map["abc"];
    auto item1_as_scalar = get_scalar_value<int>(item1);

    EXPECT_EQ(item1_as_scalar,4);
    auto res = (get_vector_value<int>(w_as_map["qwer"]) == std::vector<int>{1,2,4});
    EXPECT_TRUE(res);
}


TEST(Pmt, PmtWrap2) {
    wrap x;
    wrap y;

    x = 3;
}
