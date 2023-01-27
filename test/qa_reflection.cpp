/*-*-c++-*-*/
/*
 * Copyright 2021-2022 John Sallay
 * Copyright 2021-2022 Josh Morman
 *
 * SPDX-License-Identifier: LGPL-3.0
 *
 */
#include <gtest/gtest.h>
#include <complex>
#include <chrono>

#include <pmtv/pmt.hpp>
#include <pmtv/reflect.hpp>
#include <sstream>

using namespace pmtv;

struct simple {
    uint8_t x;
    float y;
    std::string z;
};

REFL_AUTO(type(simple), field(x), field(y), field(z))

struct simple_diff_type {
    uint8_t x;
    double y;
    std::string z;
};
REFL_AUTO(type(simple_diff_type), field(x), field(y), field(z))

struct simple_missing {
    float y;
    std::string z;
};
REFL_AUTO(type(simple_missing), field(y), field(z))

struct simple_extra {
    uint8_t x;
    std::complex<float> a;
    float y;
    std::string z;
};
REFL_AUTO(type(simple_extra), field(x), field(a), field(y), field(z))

template <class Tp>
inline __attribute__((always_inline)) void DoNotOptimize(Tp const& value) {
  asm volatile("" : : "r,m"(value) : "memory");
}
TEST(PmtReflect, Simple)
{
    // Empty Constructor
    simple test{4, -3.14, "abc"};
    simple back2;
    auto reflected = pmtv::from_struct(test);
    size_t N = 50000000;
    auto start1 = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < N; i++) {
        const auto reflected2 = pmtv::from_struct(test);
        back2 = pmtv::to_struct<simple>(reflected2);
        //test.y += back2.y;
    }
    auto end1 = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1).count() << std::endl;
    auto start2 = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < N; i++) {
        const auto reflected2 = pmtv::from_struct_faster(test);
        back2 = pmtv::to_struct_faster<simple>(reflected2);
        //test.y += back2.y;
    }
    auto end2 = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2).count() << std::endl;
    auto start3 = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < N; i++) {
        const auto reflected2 = pmtv::from_struct_faster2(test);
        back2 = pmtv::to_struct_faster2<simple>(reflected2);
        //test.y += back2.y;
    }
    auto end3 = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end3 - start3).count() << std::endl;
    auto test2 = pmtv::from_struct_faster2(test);
    EXPECT_EQ(std::get<float>(reflected["y"])*N, test.y);
    /*EXPECT_EQ(reflected["x"], test.x);
    EXPECT_EQ(reflected["y"], test.y);
    EXPECT_EQ(reflected["z"], test.z);

    auto back = pmtv::to_struct<simple>(reflected);
    EXPECT_EQ(reflected["x"], back.x);
    EXPECT_EQ(reflected["y"], back.y);
    EXPECT_EQ(reflected["z"], back.z);

    EXPECT_TRUE(validate_map<simple>(reflected));
    EXPECT_FALSE(validate_map<simple_diff_type>(reflected));
    EXPECT_FALSE(validate_map<simple_extra>(reflected));
    EXPECT_TRUE(validate_map<simple_missing>(reflected));
    EXPECT_FALSE(validate_map<simple_missing>(reflected, true));*/
}

