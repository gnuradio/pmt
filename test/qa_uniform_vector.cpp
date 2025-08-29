/*-*-c++-*-*/
/*
 * Copyright 2021 John Sallay
 *
 * SPDX-License-Identifier: LGPL-3.0
 *
 */
#include <gtest/gtest.h>
#include <complex>

#include <pmtv/pmt.hpp>
#include <pmtv/serialiser.hpp>
#include <pmtv/format.hpp>

#include <list>
#include <map>

using namespace pmtv;

using testing_types = ::testing::Types<uint32_t,
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
class PmtVectorFixture : public ::testing::Test
{
public:
    T get_value(int i) { return T(i); }
    T zero_value() { return T(0); }
    T nonzero_value() { return T(17); }
    static constexpr std::size_t num_values_ = 10UZ;
};


template <>
std::complex<float> PmtVectorFixture<std::complex<float>>::get_value(int i)
{
    return { static_cast<float>(i), static_cast<float>(-i) };
}

template <>
std::complex<double> PmtVectorFixture<std::complex<double>>::get_value(int i)
{
    return { static_cast<double>(i), static_cast<double>(-i) };
}

template <>
std::complex<float> PmtVectorFixture<std::complex<float>>::zero_value()
{
    return { 0, 0};
}
template <>
std::complex<double> PmtVectorFixture<std::complex<double>>::zero_value()
{
    return {0, 0};
}

template <>
std::complex<float> PmtVectorFixture<std::complex<float>>::nonzero_value()
{
    return { 17, -19};
}
template <>
std::complex<double> PmtVectorFixture<std::complex<double>>::nonzero_value()
{
    return {17, -19 };
}

TYPED_TEST_SUITE(PmtVectorFixture, testing_types);

/*TYPED_TEST(PmtVectorFixture, PmtVectorNull)
{
    auto pmtv = vector<TypeParam>({});

    // The following line fails inside of flatbuffers
    auto pmtw = wrap(vector<TypeParam>({}));
}*/

TYPED_TEST(PmtVectorFixture, VectorConstructors)
{
    // Empty Constructor
    pmt empty_vec{ pmtv::Tensor<TypeParam>() };
    EXPECT_EQ(std::get<pmtv::Tensor<TypeParam>>(empty_vec).size(), 0);    

    std::vector<size_t> v(1UZ, this->num_values_);
    std::cout << v[0] << std::endl;
    pmt sized_vec(pmtv::tensor_t<TypeParam>, pmtv::extents_from, v);
    EXPECT_EQ(std::get<Tensor<TypeParam>>(sized_vec).size(), v[0]);

    pmtv::Tensor<TypeParam> vec({this->num_values_});
    for (std::size_t i = 0; i < this->num_values_; i++) {
        vec[i] = this->get_value(static_cast<int>(i));
    }

    // Copy from std::vector
    pmt pmt_vec = vec;
    auto pmt_comp = std::get<Tensor<TypeParam>>(pmt_vec);
    EXPECT_EQ(pmt_comp, vec);

    // Copy Constructor
    pmt a = pmt_vec;
    EXPECT_EQ(a == vec, true);
    EXPECT_EQ(a == pmt_vec, true);

    // Assignment operator from std::vector
    a = vec;
    EXPECT_EQ(a == vec, true);
    EXPECT_EQ(a == pmt_vec, true);

    a = pmt_vec;
    EXPECT_EQ(a == vec, true);
    EXPECT_EQ(a == pmt_vec, true);

    // TODO: Add in Move contstructor
}

TYPED_TEST(PmtVectorFixture, PmtVectorSerialize)
{
    // Serialize/Deserialize and make sure that it works
    pmtv::Tensor<TypeParam> vec({this->num_values_});
    for (std::size_t i = 0; i < this->num_values_; i++) {
        vec[i] = this->get_value(static_cast<int>(i));
    }
    pmt x(vec);
    std::stringbuf sb;
    pmtv::serialize(sb, x);
    auto y = pmtv::deserialize(sb);
    EXPECT_EQ(x == y, true);
}

TYPED_TEST(PmtVectorFixture, VectorWrites)
{
    // Initialize a PMT Wrap from a std::vector object
    pmtv::Tensor<TypeParam> vec({this->num_values_});
    pmtv::Tensor<TypeParam> vec_modified({this->num_values_});
    for (std::size_t i = 0; i < this->num_values_; i++) {
        vec[i] = this->get_value(static_cast<int>(i));
        vec_modified[i] = vec[i];
        if (i % 7 == 2) {
            vec_modified[i] = vec[i] + this->get_value(static_cast<int>(i));
        }
    }

    auto pmt_vec = pmt(vec);
    auto pmt_span = get_span<TypeParam>(pmt_vec);
    for (std::size_t i = 0; i < this->num_values_; i++) {
        if (i % 7 == 2) {
            pmt_span[i] = pmt_span[i] + this->get_value(static_cast<int>(i));
        }
    }
    EXPECT_EQ(pmt_vec == vec_modified, true);
}


TYPED_TEST(PmtVectorFixture, get_as)
{
    pmtv::Tensor<TypeParam> vec({this->num_values_});
    for (std::size_t i = 0; i < this->num_values_; i++) {
        vec[i] = this->get_value(static_cast<int>(i));
    }
    pmt x = vec;
    // Make sure that we can get the value back out
    auto y = std::get<pmtv::Tensor<TypeParam>>(x);
    EXPECT_TRUE(x == y);

    // Should also work as a span
    auto z = get_span<TypeParam>(x);
    EXPECT_TRUE(std::equal(z.begin(), z.end(), vec.data()));
}

TYPED_TEST(PmtVectorFixture, base64)
{
    Tensor<TypeParam> vec({ this->num_values_ });
    pmt x = vec;

    // Make sure that we can get the value back out
    auto encoded_str = pmtv::to_base64(x);
    auto y = pmtv::from_base64(encoded_str);

    EXPECT_TRUE(x == y);
}

TYPED_TEST(PmtVectorFixture, fmt) {
    Tensor<TypeParam> vec({ this->num_values_ });
    pmt x = vec;
    EXPECT_EQ(fmt::format("{}", x), fmt::format("[{}]", fmt::join(vec.data_span(), ", ")));
}
