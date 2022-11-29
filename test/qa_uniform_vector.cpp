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
    T get_value(int i) { return (T)i; }
    T zero_value() { return (T)0; }
    T nonzero_value() { return (T)17; }
    static const int num_values_ = 10;
};


template <>
std::complex<float> PmtVectorFixture<std::complex<float>>::get_value(int i)
{
    return std::complex<float>(i, -i);
}

template <>
std::complex<double> PmtVectorFixture<std::complex<double>>::get_value(int i)
{
    return std::complex<double>(i, -i);
}

template <>
std::complex<float> PmtVectorFixture<std::complex<float>>::zero_value()
{
    return std::complex<float>(0, 0);
}
template <>
std::complex<double> PmtVectorFixture<std::complex<double>>::zero_value()
{
    return std::complex<double>(0, 0);
}

template <>
std::complex<float> PmtVectorFixture<std::complex<float>>::nonzero_value()
{
    return std::complex<float>(17, -19);
}
template <>
std::complex<double> PmtVectorFixture<std::complex<double>>::nonzero_value()
{
    return std::complex<double>(17, -19);
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
    pmt empty_vec{ std::vector<TypeParam>() };
    EXPECT_EQ(std::get<std::vector<TypeParam>>(empty_vec).size(), 0);

    int num_values = this->num_values_;
    pmt sized_vec(vec_t<TypeParam>, num_values);
    EXPECT_EQ(sized_vec.size(), num_values);

    // Init from std::vector
    std::vector<TypeParam> vec(this->num_values_);
    for (auto i = 0; i < this->num_values_; i++) {
        vec[i] = this->get_value(i);
    }

    // Range Constructor
    pmt range_vec(vec_t<TypeParam>, vec.begin(), vec.end());
    EXPECT_EQ(range_vec.size(), num_values);
    const auto& range_vals = std::get<std::vector<TypeParam>>(range_vec);
    for (size_t i = 0; i < range_vec.size(); i++) {
        EXPECT_EQ(range_vals[i], vec[i]);
    }

    // Copy from std::vector
    pmt pmt_vec = std::vector<TypeParam>(vec);
    EXPECT_EQ(pmt_vec == vec, true);

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


TYPED_TEST(PmtVectorFixture, RangeBasedLoop)
{

    std::vector<TypeParam> vec(this->num_values_);
    std::vector<TypeParam> vec_doubled(this->num_values_);
    std::vector<TypeParam> vec_squared(this->num_values_);
    for (auto i = 0; i < this->num_values_; i++) {
        vec[i] = this->get_value(i);
        vec_doubled[i] = vec[i] + vec[i];
        vec_squared[i] = vec[i] * vec[i];
    }
    // Init from std::vector
    auto pmt_vec = pmt(vec);
    // for (auto& xx : std::span(std::get<std::vector<TypeParam>>(pmt_vec))) {
    for (auto& xx : get_span<TypeParam>(pmt_vec)) {
        xx *= xx;
    }

    EXPECT_EQ(pmt_vec == vec_squared, true);

    pmt_vec = vec;
    for (auto& xx : get_span<TypeParam>(pmt_vec)) {
        xx += xx;
    }
    EXPECT_EQ(pmt_vec == vec_doubled, true);
}

TYPED_TEST(PmtVectorFixture, PmtVectorSerialize)
{
    // Serialize/Deserialize and make sure that it works
    std::vector<TypeParam> vec(this->num_values_);
    for (auto i = 0; i < this->num_values_; i++) {
        vec[i] = this->get_value(i);
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
    std::vector<TypeParam> vec(this->num_values_);
    std::vector<TypeParam> vec_modified(this->num_values_);
    for (auto i = 0; i < this->num_values_; i++) {
        vec[i] = this->get_value(i);
        vec_modified[i] = vec[i];
        if (i % 7 == 2) {
            vec_modified[i] = vec[i] + this->get_value(i);
        }
    }

    auto pmt_vec = pmt(vec);
    auto pmt_span = get_span<TypeParam>(pmt_vec);
    for (auto i = 0; i < this->num_values_; i++) {
        if (i % 7 == 2) {
            pmt_span[i] = pmt_span[i] + this->get_value(i);
        }
    }
    EXPECT_EQ(pmt_vec == vec_modified, true);
}


TYPED_TEST(PmtVectorFixture, get_as)
{
    std::vector<TypeParam> vec(this->num_values_);
    for (auto i = 0; i < this->num_values_; i++) {
        vec[i] = this->get_value(i);
    }
    pmt x = vec;
    // Make sure that we can get the value back out
    auto y = std::get<std::vector<TypeParam>>(x);
    EXPECT_TRUE(x == y);

    // Should also work as a span
    auto z = get_span<TypeParam>(x);
    EXPECT_TRUE(x == std::vector<TypeParam>(z.begin(), z.end()));

    // // Should also work as a list
    // auto q = std::list<TypeParam>(x);
    // EXPECT_TRUE(x == std::vector<TypeParam>(q.begin(), q.end()));

    // // Fail if wrong type of vector or non vector type
    // EXPECT_THROW(int(x), ConversionError);
    // if constexpr(std::is_same_v<TypeParam, int>)
    //     EXPECT_THROW(std::vector<double>(x), ConversionError);
    // else
    //     EXPECT_THROW(std::vector<int>(x), ConversionError);

    // using mtype = std::map<std::string, pmt>;
    // EXPECT_THROW(mtype(x), ConversionError);
}

TYPED_TEST(PmtVectorFixture, base64)
{
    std::vector<TypeParam> vec(this->num_values_);
    pmt x = vec;

    // Make sure that we can get the value back out
    auto encoded_str = pmtv::to_base64(x);
    auto y = pmtv::from_base64(encoded_str);

    EXPECT_TRUE(x == y);
}

// #if 0
// TYPED_TEST(PmtVectorFixture, vector_wrapper)
// {
//     std::vector<TypeParam> vec(this->num_values_);
//     pmt x = vec;
//     // This should not throw
//     pmtf::vector_wrap z(x);

//     EXPECT_EQ(z.size(), vec.size());
//     EXPECT_EQ(z.bytes_per_element(), sizeof(TypeParam));
//     EXPECT_EQ(z.bytes(), vec.size() * sizeof(TypeParam));
//     EXPECT_EQ(z,x);
//     EXPECT_EQ(x,z);

//     pmtf::vector_wrap a(vec);
//     std::cout << a << std::endl;

//     // TODO: Define all of the functionality that we should have here.
//     // Pointer to the beginning

// }
// #endif
