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
#include <sstream>

using namespace pmtf;

using testing_types = ::testing::Types<uint8_t,
                                       int8_t,
                                       uint16_t,
                                       int16_t,
                                       uint32_t,
                                       int32_t,
                                       uint64_t,
                                       int64_t,
                                       float,
                                       double,
                                       std::complex<float>,
                                       std::complex<double>>;

// using testing_types = ::testing::Types<float>; //,
//                                                //    std::complex<double>>;


template <typename T>
class PmtScalarFixture : public ::testing::Test
{
public:
    T get_value(int i) { return (T)i; }
    T zero_value() { return (T)0; }
    T nonzero_value() { return (T)17; }
};


template <>
std::complex<float> PmtScalarFixture<std::complex<float>>::get_value(int i)
{
    return std::complex<float>(i, -i);
}

template <>
std::complex<double> PmtScalarFixture<std::complex<double>>::get_value(int i)
{
    return std::complex<double>(i, -i);
}

template <>
std::complex<float> PmtScalarFixture<std::complex<float>>::zero_value()
{
    return std::complex<float>(0, 0);
}
template <>
std::complex<double> PmtScalarFixture<std::complex<double>>::zero_value()
{
    return std::complex<double>(0, 0);
}

template <>
std::complex<float> PmtScalarFixture<std::complex<float>>::nonzero_value()
{
    return std::complex<float>(17, -19);
}
template <>
std::complex<double> PmtScalarFixture<std::complex<double>>::nonzero_value()
{
    return std::complex<double>(17, -19);
}

TYPED_TEST_SUITE(PmtScalarFixture, testing_types);

TYPED_TEST(PmtScalarFixture, PmtScalarNull)
{
    // Should initialize to 0
    scalar<TypeParam> x;
    EXPECT_EQ(x == TypeParam(0), true);
}

TYPED_TEST(PmtScalarFixture, PmtScalarConstruction)
{
    // We should be able to do:
    // a = 4;
    // a(4);
    // a = scalar(4)
    // a(scalar(4))
    auto value = this->get_value(0);
    // Init from value
    auto pmt_scalar = value;
    EXPECT_EQ(pmt_scalar == value, true);

    // Copy Constructor
    auto a = pmt_scalar;
    EXPECT_EQ(a == value, true);
    EXPECT_EQ(a == pmt_scalar, true);

    // Assignment operator from scalar
    scalar<TypeParam> b(a);
    EXPECT_EQ(b == value, true);
    EXPECT_EQ(b == pmt_scalar, true);

    scalar<TypeParam> c(value);
    EXPECT_EQ(c == value, true);
    EXPECT_EQ(c == pmt_scalar, true);
}

TYPED_TEST(PmtScalarFixture, PmtScalarValue) {
    // Get the value, change the value
    auto value = this->get_value(0);
    scalar<TypeParam> x(value);
    EXPECT_EQ(x.value(), value);
    value *= 2;
    x = value;
    EXPECT_EQ(x.value(), value);
}

TYPED_TEST(PmtScalarFixture, PmtScalarPrint) {
    // Send to string stream and make sure it works.
    auto value = this->get_value(0);
    scalar<TypeParam> x(value);
    std::stringstream ss;
    ss << x;
    std::cout << x << std::endl;
    if constexpr(is_complex<TypeParam>::value) {
        //EXPECT_EQ(ss.str(), std::to_string(value));
    } else
        EXPECT_EQ(ss.str(), std::to_string(value));
}

TYPED_TEST(PmtScalarFixture, PmtScalarSerialize) {
    // Serialize/Deserialize and make sure that it works
    auto value = this->get_value(0);
    scalar<TypeParam> x(value);
    std::stringbuf sb;
    x.get_pmt_buffer().serialize(sb);
    auto y = pmt::deserialize(sb);
    EXPECT_EQ(x.value(), scalar<TypeParam>(y).value());
}

TYPED_TEST(PmtScalarFixture, PmtScalarWrap)
{
    EXPECT_EQ(1 == 0, true);
    // Initialize a PMT Wrap from a std::vector object
    /*std::vector<TypeParam> vec(this->num_values_);
    for (auto i = 0; i < this->num_values_; i++) {
        vec[i] = this->get_value(i);
    }

    wrap generic_pmt_obj = std::vector<TypeParam>(vec);
    auto y = get_vector<TypeParam>(generic_pmt_obj); 
    EXPECT_EQ(y == vec, true);

    // Try to cast as a scalar type
    EXPECT_THROW(get_scalar<int8_t>(generic_pmt_obj), std::runtime_error);*/
}

