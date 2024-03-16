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

#include <pmtv/pmt.hpp>
#include <sstream>

using namespace pmtv;

using testing_types = ::testing::Types<uint8_t,
                                       int8_t,
                                       uint16_t,
                                       int16_t,
                                       uint32_t,
                                       int32_t,
                                       uint64_t,
                                       int64_t,
                                       std::size_t,
                                       float,
                                       double,
                                       std::complex<float>,
                                       std::complex<double>>;

template <typename T>
class PmtScalarFixture : public ::testing::Test
{
public:
    T get_value() { return T(4); }
    T zero_value() { return T(0); }
};

template <>
float PmtScalarFixture<float>::get_value()
{
    return 4.1f;
}
template <>
double PmtScalarFixture<double>::get_value()
{
    return 4.1;
}

template <>
std::complex<float> PmtScalarFixture<std::complex<float>>::get_value()
{
    return std::complex<float>(4.1f, -4.1f);
}

template <>
std::complex<double> PmtScalarFixture<std::complex<double>>::get_value()
{
    return std::complex<double>(4.1, -4.1);
}

TYPED_TEST_SUITE(PmtScalarFixture, testing_types);

TYPED_TEST(PmtScalarFixture, PmtScalarNull)
{
    // Should initialize to nullptr
    pmt x; //{this->get_value()};
    EXPECT_TRUE(x == pmt_null());
}

TYPED_TEST(PmtScalarFixture, PmtScalarConstruction)
{
    // We should be able to do:
    // a = 4;
    // a(4);
    // a = pmt(4)
    // a(pmt(4))
    auto value = this->get_value();
    // Init from value
    auto a = value;
    EXPECT_TRUE(a == value);

    // Copy Constructor
    auto b = a;
    EXPECT_TRUE(b == value);
    EXPECT_TRUE(b == a);

    // Assignment operator from pmt
    pmt c(b);
    EXPECT_TRUE(c == value);
    EXPECT_TRUE(c == a);

    // Assignment operator from value
    pmt d(value);
    EXPECT_TRUE(d == value);
    EXPECT_TRUE(d == a);
    EXPECT_TRUE(value == d);

    pmt e = value;
    EXPECT_TRUE(a == e);
    EXPECT_TRUE(e == value);
    EXPECT_TRUE(e == b);
}

TYPED_TEST(PmtScalarFixture, PmtScalarValue)
{
    // Get the value, change the value
    auto value = this->get_value();
    pmt x(value);
    EXPECT_TRUE(x == value);
    value *= 2;
    x = value;
    EXPECT_TRUE(x == value);
    // pmt e({{"abc", 123}, {"you and me", "baby"}});
    pmt e(std::vector({ 4, 5, 6 }));
}

TYPED_TEST(PmtScalarFixture, PmtScalarPrint)
{
    // Send to string stream and make sure it works.
    auto value = this->get_value();
    pmt x(value);
    std::stringstream ss;
    std::stringstream ss_check;
    ss << x;
    // Annoying special cases
    if constexpr(Complex<TypeParam>) {
        if (value.imag() >= 0)
            ss_check << value.real() << "+j" << value.imag();
        else
            ss_check << value.real() << "-j" << -value.imag();
    } else if constexpr(std::same_as<TypeParam, signed char>)
        ss_check << int(value);
    else if constexpr(std::same_as<TypeParam, unsigned char>)
        ss_check << unsigned(value);
    else
        ss_check << value;
    EXPECT_EQ(ss.str(), ss_check.str());
}

TYPED_TEST(PmtScalarFixture, PmtScalarSerialize)
{
    // Serialize/Deserialize and make sure that it works
    auto value = this->get_value();
    pmt x(value);
    std::stringbuf sb;
    pmtv::serialize(sb, x);
    auto y = pmtv::deserialize(sb);

    if constexpr (support_size_t && std::is_same_v<std::size_t, TypeParam>) {
        EXPECT_TRUE(static_cast<uint64_t>(value) == y);
        EXPECT_FALSE(value == y);
    } else {
        EXPECT_TRUE(value == y);
    }
}

TYPED_TEST(PmtScalarFixture, explicit_cast)
{
    pmt x = this->get_value();
    // Make sure that we can get the value back out
    auto y = pmtv::cast<TypeParam>(x);
    EXPECT_TRUE(x == y);

    if constexpr (Complex<TypeParam>) {
        auto z1 = pmtv::cast<std::complex<double>>(x);
        EXPECT_TRUE(std::complex<double>(this->get_value()) == z1);

        auto z2 = pmtv::cast<std::complex<float>>(x);
        EXPECT_TRUE(std::complex<float>(this->get_value()) == z2);
    }

    if constexpr (!Complex<TypeParam>) {
        // Cast up to complex<double>
        auto z1 = pmtv::cast<std::complex<double>>(x);
        EXPECT_TRUE(std::complex<double>(static_cast<double>(this->get_value())) == z1);

        auto z2 = pmtv::cast<std::complex<float>>(x);
        EXPECT_TRUE(std::complex<float>(static_cast<float>(this->get_value())) == z2);

        auto z3 = pmtv::cast<double>(x);
        EXPECT_TRUE(double(this->get_value()) == z3);
    }

    // Fail if we try to get a container type
    // FIXME: doesn't throw yet because this is not detected
    // EXPECT_ANY_THROW(std::vector<int>(x));
}

TYPED_TEST(PmtScalarFixture, wrong_cast)
{
    if constexpr (Scalar<TypeParam> && !Complex<TypeParam> ) {
        TypeParam p0 {54};
        pmt p1 = p0;
        EXPECT_TRUE(p0 == p1);

        // cast to different type than TypeParam
        if constexpr (!std::is_same_v<TypeParam, double>) {
            EXPECT_FALSE(static_cast<double >(p0) == p1);
        } else {
            EXPECT_FALSE(static_cast<int>(p0) == p1);
        }
    }
}

TYPED_TEST(PmtScalarFixture, base64)
{
    auto value = this->get_value();
    pmt x(value);
    // Make sure that we can get the value back out
    auto encoded_str = pmtv::to_base64(x);
    auto y = pmtv::from_base64(encoded_str);

    if constexpr (support_size_t && std::is_same_v<std::size_t, TypeParam>) {
        EXPECT_TRUE(static_cast<uint64_t>(value) == y);
    } else {
        EXPECT_TRUE(x == y);
    }
}

TYPED_TEST(PmtScalarFixture, element_size)
{
    pmt x = this->get_value();
    EXPECT_TRUE(elements(x) == 1);
    EXPECT_TRUE(bytes_per_element(x) == sizeof(TypeParam));
}

TYPED_TEST(PmtScalarFixture, fmt) {
    pmt x = this->get_value();
    EXPECT_EQ(fmt::format("{}", x), fmt::format("{}", this->get_value()));
}
