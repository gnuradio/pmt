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

// TEST(Pmt, BasicPmtTests)
// {
//     std::complex<float> cplx_val = std::complex<float>(1.2, -3.4);
//     auto x = scalar(cplx_val);

//     EXPECT_EQ(x, cplx_val);
//     EXPECT_EQ(x.data_type(), Data::ScalarComplex64);

//     std::vector<int32_t> int_vec_val{ 5, 9, 23445, 63, -25 };
//     auto int_pmt_vec = vector<int32_t>(int_vec_val);
//     EXPECT_EQ(int_pmt_vec, int_vec_val);
//     EXPECT_EQ(int_pmt_vec.data_type(), Data::VectorInt32);

//     std::vector<std::complex<float>> cf_vec_val{ { 0, 1 }, { 2, 3 }, { 4, 5 } };
//     auto cf_pmt_vec = vector<std::complex<float>>(cf_vec_val);
//     EXPECT_EQ(cf_pmt_vec, cf_vec_val);
//     EXPECT_EQ(cf_pmt_vec.data_type(), Data::VectorComplex64);
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

    map m;
    m["abc"] = std::make_shared<scalar<uint8_t>>(x);
    m["def"] = std::make_shared<vector<float>>(y);
    std::cout << m << std::endl;
    for (const auto& [k, v]: m) {
        std::cout << k << ": " << v << std::endl;
    }
}

TEST(Pmt, PmtScalarTests) {
    auto x = scalar<int>(4);
    EXPECT_EQ(x, 4);
    scalar<int> y(4);
    EXPECT_EQ(x, y);
    x = 5;
    EXPECT_EQ(x, 5.0);
    y = x;
    EXPECT_EQ(x, y);
    scalar<int> z = 4;
    int a = int(x);
    EXPECT_EQ(a, 5.0);
    float b = float(x);
    EXPECT_EQ(b, 5.0);
}

TEST(Pmt, PmtVectorTests) {
    // Init from values
    auto x = vector<std::complex<float>>({{1, -1}, {2.1, 3.0}});
    std::vector<std::complex<float>> y({{1, -1}, {2.1, 3.0}});
    // Do they equal each other
    EXPECT_EQ(x, y );
    // Init from vector
    auto z =  vector<std::complex<float>>(y);
    // Copy constructor
    auto a =  vector<std::complex<float>>(x);
    // Assignment operator
    a = x;
    // TODO: Add in Move contstructor
    // Make sure we can do a range based for loop
    for (auto& xx : x) {
        xx += 1;
    }
    // Print
    std::cout << x << std::endl;
}

// TEST(Pmt, PmtStringTests)
// {
//     auto str_pmt = string("hello");
//     std::cout << str_pmt << std::endl;

//     EXPECT_EQ(str_pmt, "hello");
// }


/*TEST(Pmt, PmtMapTests)
{
    std::complex<float> val1(1.2, -3.4);
    std::vector<int32_t> val2{ 44, 34563, -255729, 4402 };

    // Create the PMT map
    std::map<std::string, wrap> input_map({
        { "key1", val1 },
        { "key2", val2 },
    });
    auto map_pmt = map<std::string>(input_map);

    // Lookup values in the PMT map and compare with what was put in there
    auto vv1 = map_pmt["key1"];
    std::cout << vv1 << std::endl;
    EXPECT_EQ(get_scalar<std::complex<float>>(vv1), val1);

    auto vv2 = map_pmt["key2"];
    EXPECT_EQ(vv2, val2);
}*/

TEST(Pmt, PmtWrap)
{
    wrap x = int8_t(4);
    get_scalar<int8_t>(x);
    get_scalar<int8_t>(x);
}
TEST(pmt, CanBe)
{
    wrap x = int32_t(257);
    assert(can_be<int8_t>(x) == true);
    assert(can_be<uint64_t>(x) == true);
    assert(can_be<float>(x) == true);
    assert(can_be<std::complex<double>>(x) == true);
    assert(can_be<bool>(x) == true);
    assert(can_be<std::string>(x) == false);
    assert(can_be<scalar<int32_t>>(x) == true);
    assert(can_be<scalar<int8_t>>(x) == true);
}

/*TEST(Pmt, VectorWrites)
{
    {
        std::vector<std::complex<float>> cf_vec_val{ { 0, 1 }, { 2, 3 }, { 4, 5 } };
        std::vector<std::complex<float>> cf_vec_val_modified{ { 4, 5 },
                                                              { 6, 7 },
                                                              { 8, 9 } };
        auto cf_pmt_vec = vector(cf_vec_val);
        EXPECT_EQ(cf_pmt_vec, cf_vec_val);
        EXPECT_EQ(cf_pmt_vec.data_type(), Data::VectorComplex64);

        cf_vec_val[0] = { 4, 5 };
        cf_vec_val[1] = { 6, 7 };
        cf_vec_val[2] = { 8, 9 };

        EXPECT_EQ(cf_pmt_vec, cf_vec_val_modified);
    }
    {
        std::vector<uint32_t> int_vec_val{ 1, 2, 3, 4, 5 };
        std::vector<uint32_t> int_vec_val_modified{ 6, 7, 8, 9, 10 };
        auto int_pmt_vec = vector(int_vec_val);
        EXPECT_EQ(int_pmt_vec, int_vec_val);
        EXPECT_EQ(int_pmt_vec.data_type(), Data::VectorUInt32);

        int_vec_val[0] = 6;
        int_vec_val[1] = 7;
        int_vec_val[2] = 8;
        int_vec_val[3] = 9;
        int_vec_val[4] = 10;

        EXPECT_EQ(int_pmt_vec, int_vec_val_modified);
    }
}

TEST(Pmt, VectorWrapper) {
    vector<uint32_t> x(10);
    vector<uint32_t> y{1,2,3,4,6,7};
    std::vector<uint32_t> data{1,2,3,4,6,7};
    for (size_t i = 0; i < y.size(); i++) {
        EXPECT_EQ(y[i], data[i]);
    }
    // Make sure that range based for loop works.
    size_t i = 0;
    for (auto& e : y) {
        EXPECT_EQ(e, data[i++]);
    }

    // Make sure I can mutate the data
    for (auto& e: y) {
        e += 2;
    }
    i = 0;
    for (auto& e: y) {
        EXPECT_EQ(e, data[i++]+2);
    }

    // Create from an std::vector
    vector<uint32_t> x_vec(std::vector<uint32_t>{1,2,3,4,6,7});

    // Check the other constructors
    vector<uint32_t> vec1(4);
    EXPECT_EQ(vec1.size(), 4);
    for (auto& e: vec1)
        EXPECT_EQ(e, 0);

    vector<uint32_t> vec2(4, 2);
    for (auto& e: vec2)
        EXPECT_EQ(e, 2);

    vector<uint32_t> vec3(data.begin(), data.end());
    EXPECT_EQ(vec3.size(), data.size());
    i = 0;
    for (auto& e: vec3)
        EXPECT_EQ(e, data[i++]);

    vector<uint32_t> vec4(vec3);
    EXPECT_EQ(vec3.ptr(), vec4.ptr());
}

TEST(Pmt, MapWrapper) {
    map<std::string> x;
    x["abc"] = 4;
    x["qwer"] = std::vector<int>{1,2,4};
    for (auto& [key, value]: x) {
        std::cout << key << std::endl;
    }
}
*/

TEST(Pmt, PmtWrap2) {
    wrap x;
    wrap y;

    x = 3;
}
