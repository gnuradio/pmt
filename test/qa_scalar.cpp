/*-*-c++-*-*/
/*
 * Copyright 2021 John Sallay
 * Copyright 2021 Josh Morman
 *
 * SPDX-License-Identifier: LGPL-3.0
 *
 */
#include <gtest/gtest.h>
#include <complex>

#include <pmtv/pmt.hpp>
#include <sstream>

using namespace pmtv;

using testing_types = ::testing::Types<
                                        uint8_t,
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

template <typename T>
class PmtScalarFixture : public ::testing::Test
{
public:
    T get_value() { return T(4); }
    T zero_value() { return T(0); }
};


template <> float PmtScalarFixture<float>::get_value() { return 4.1; }
template <> double PmtScalarFixture<double>::get_value() { return 4.1; }

template <>
std::complex<float> PmtScalarFixture<std::complex<float>>::get_value()
{
    return std::complex<float>(4.1, -4.1);
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
    EXPECT_TRUE(x == nullptr);
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
    EXPECT_EQ(a == value, true);

    // Copy Constructor
    auto b = a;
    EXPECT_EQ(b == value, true);
    EXPECT_EQ(b == a, true);

    // Assignment operator from pmt
    pmt c(b);
    EXPECT_EQ(c == value, true);
    EXPECT_EQ(c == a, true);

    // Assignment operator from value
    pmt d(value);
    EXPECT_EQ(d == value, true);
    EXPECT_EQ(d == a, true);
    EXPECT_EQ(value == d, true);
   
    pmt e = value;
    EXPECT_EQ(a == e, true);
    EXPECT_EQ(e == value, true);
    EXPECT_EQ(e == b, true);
}

TYPED_TEST(PmtScalarFixture, PmtScalarValue) {
    // Get the value, change the value
    auto value = this->get_value();
    pmt x(value);
    EXPECT_EQ(x, value);
    value *= 2;
    x = value;
    EXPECT_EQ(x, value);
    //pmt e({{"abc", 123}, {"you and me", "baby"}});
    pmt e(std::vector({4, 5, 6}));
}



TYPED_TEST(PmtScalarFixture, PmtScalarPrint) {
    // Send to string stream and make sure it works.
    auto value = this->get_value();
    pmt x(value);
    std::stringstream ss;
    std::stringstream ss_check;
    ss << x;
    ss_check << value;
    EXPECT_EQ(ss.str(), ss_check.str());
}


// TYPED_TEST(PmtScalarFixture, PmtScalarSerialize) {
//     // Serialize/Deserialize and make sure that it works
//     auto value = this->get_value();
//     pmt x(value);
//     std::stringbuf sb;
//     pmtv::serialize(sb, x);
//     auto y = pmtv::deserialize(sb);
//     EXPECT_EQ(value, y);
// }


/*TYPED_TEST(PmtScalarFixture, explicit_cast)
{
    pmt x = this->get_value();
    // Make sure that we can get the value back out
    auto y = TypeParam(x);
    EXPECT_EQ(x == y, true);

    // Cast up to complex<double>
    auto z = std::complex<double>(x);
    EXPECT_EQ(std::complex<double>(this->get_value()) == z, true);

    // Cast up to double if possible
    if constexpr(!Complex<TypeParam>) {
        auto z = double(x);
        EXPECT_EQ(this->get_value() == z, true);
    }

    // Fail if we try to get a container type
    // FIXME: doesn't throw yet because this is not detected
    // EXPECT_ANY_THROW(std::vector<int>(x));
}*/


// TYPED_TEST(PmtScalarFixture, base64)
// {
//     pmt x = this->get_value();
//     // Make sure that we can get the value back out
//     auto encoded_str = to_base64(x);
//     auto y = from_base64(encoded_str);

//     EXPECT_EQ(x == y, true);
// }

// TYPED_TEST(PmtScalarFixture, element_size)
// {
//     pmt x = this->get_value();
//     EXPECT_EQ(elements(x), 1);
//     EXPECT_EQ(bytes_per_element(x), sizeof(TypeParam));
// }
