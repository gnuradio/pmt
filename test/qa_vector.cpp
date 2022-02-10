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
#include <pmtf/vector.hpp>
#include <pmtf/scalar.hpp>
#include <pmtf/wrap.hpp>

#include <list>
#include <map>

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


template <typename T>
class PmtVectorFixture : public ::testing::Test
{
public:
    T get_value(int i) { return (T)i; }
    T zero_value() { return (T)0; }
    T nonzero_value() { return (T)17; }
    static const int num_values_ = 100;
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
    vector<TypeParam> empty_vec;
    EXPECT_EQ(empty_vec.size(), 0);
    int num_values = this->num_values_;

    // Size Constructor ( but uninit memory)
    vector<TypeParam> sized_vec(num_values);
    EXPECT_EQ(sized_vec.size(), num_values);

    // Fill Constructor
    vector<TypeParam> fill_vec(num_values, this->nonzero_value());
    EXPECT_EQ(fill_vec.size(), num_values);
    for (const auto& x: fill_vec) {
        EXPECT_EQ(x, this->nonzero_value());
    }

    // Init from std::vector
    std::vector<TypeParam> vec(this->num_values_);
    for (auto i = 0; i < this->num_values_; i++) {
        vec[i] = this->get_value(i);
    }

    // Range Constructor
    vector<TypeParam> range_vec(vec.begin(), vec.end());
    EXPECT_EQ(range_vec.size(), num_values);
    for (size_t i = 0; i < range_vec.size(); i++) {
        EXPECT_EQ(range_vec[i], vec[i]);
    }

    // Copy from std::vector
    auto pmt_vec = vector<TypeParam>(vec);
    EXPECT_EQ(pmt_vec == vec, true);

    // Copy Constructor
    auto a = vector<TypeParam>(pmt_vec);
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
    auto pmt_vec = vector<TypeParam>(vec);
    for (auto& xx : pmt_vec) {
        xx *= xx;
    }
    EXPECT_EQ(pmt_vec == vec_squared, true);

    pmt_vec = vector<TypeParam>(vec);
    for (auto& xx : pmt_vec) {
        xx += xx;
    }
    EXPECT_EQ(pmt_vec == vec_doubled, true);
}

TYPED_TEST(PmtVectorFixture, PmtVectorSerialize) {
    // Serialize/Deserialize and make sure that it works
    std::vector<TypeParam> vec(this->num_values_);
    for (auto i = 0; i < this->num_values_; i++) {
        vec[i] = this->get_value(i);
    }
    vector<TypeParam> x(vec);
    std::stringbuf sb;
    x.get_pmt_buffer().serialize(sb);
    auto y = pmt::deserialize(sb);
    EXPECT_EQ(x.value(), vector<TypeParam>(y).value());
}

/*TYPED_TEST(PmtVectorFixture, PmtVectorWrap)
{
    // Initialize a PMT Wrap from a std::vector object
    std::vector<TypeParam> vec(this->num_values_);
    for (auto i = 0; i < this->num_values_; i++) {
        vec[i] = this->get_value(i);
    }

    wrap generic_pmt_obj = std::vector<TypeParam>(vec);
    auto y = get_vector<TypeParam>(generic_pmt_obj); 
    EXPECT_EQ(y == vec, true);

    // Try to cast as a scalar type
    EXPECT_THROW(get_scalar<int8_t>(generic_pmt_obj), std::runtime_error);
}*/


/*TYPED_TEST(PmtVectorFixture, VectorWrites)
{
    // Initialize a PMT Wrap from a std::vector object
    std::vector<TypeParam> vec(this->num_values_);
    std::vector<TypeParam> vec_modified(this->num_values_);
    for (auto i = 0; i < this->num_values_; i++) {
        vec[i] = this->get_value(i);
        vec_modified[i] = vec[i];
        if (i%7 == 2) {
            vec_modified[i] = vec[i] + this->get_value(i);
        }
    }

    auto pmt_vec = vector(vec);
    for (auto i = 0; i < this->num_values_; i++) {
        if (i%7 == 2) {
            pmt_vec[i] = pmt_vec[i] + this->get_value(i);
        }
    }
    EXPECT_EQ(pmt_vec, vec_modified);

}*/

TYPED_TEST(PmtVectorFixture, OtherConstructors) {

    // Check the other constructors
    vector<TypeParam> vec1(4);
    EXPECT_EQ(vec1.size(), 4);

    vector<TypeParam> vec2(4, this->nonzero_value());
    std::cout << vec2.size() << std::endl;
    for (auto& e: vec2)
        EXPECT_EQ(e, this->nonzero_value());


    std::vector<TypeParam> data(this->num_values_);
    for (auto i = 0; i < this->num_values_; i++) {
        data[i] = this->get_value(i);
    }

    vector<TypeParam> vec3(data.begin(), data.end());
    EXPECT_EQ(vec3.size(), data.size());
    size_t i = 0;
    for (auto& e: vec3)
        EXPECT_EQ(e, data[i++]);

    vector<TypeParam> vec4(vec3);
    EXPECT_EQ(vec3.value(), vec4.value());
}

TYPED_TEST(PmtVectorFixture, get_as)
{
    std::vector<TypeParam> vec(this->num_values_);
    for (auto i = 0; i < this->num_values_; i++) {
        vec[i] = this->get_value(i);
    }
    pmt x = vec;
    // Make sure that we can get the value back out
    auto y = get_as<std::vector<TypeParam>>(x);
    EXPECT_EQ(x, y);

    // Should also work as a span
    auto z = get_as<gsl::span<TypeParam>>(x);
    EXPECT_EQ(x, std::vector<TypeParam>(z.begin(), z.end()));
    
    // Should also work as a list
    auto q = get_as<std::list<TypeParam>>(x);
    EXPECT_EQ(x, std::vector<TypeParam>(q.begin(), q.end()));

    // Fail if wrong type of vector or non vector type
    EXPECT_THROW(get_as<int>(x), ConversionError);
    if constexpr(std::is_same_v<TypeParam, int>)
        EXPECT_THROW(get_as<std::vector<double>>(x), ConversionError);
    else
        EXPECT_THROW(get_as<std::vector<int>>(x), ConversionError);

    using mtype = std::map<std::string, pmt>;
    EXPECT_THROW(get_as<mtype>(x), ConversionError);
    
}

TYPED_TEST(PmtVectorFixture, base64)
{
    std::vector<TypeParam> vec(this->num_values_);
    pmt x = vec;
    
    // Make sure that we can get the value back out
    auto encoded_str = pmt(x).to_base64();
    auto y = pmt::from_base64(encoded_str);

    EXPECT_EQ(x, y);
}