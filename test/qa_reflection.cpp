/*-*-c++-*-*/
/*
 * Copyright 2023 John Sallay
 *
 * SPDX-License-Identifier: LGPL-3.0
 *
 */
#include <gtest/gtest.h>

#include <pmtv/pmt.hpp>

#include <fmt/core.h>
#include <string>

using namespace pmtv;

struct qqq {
    int x;
    float y;
    double z;
};

REFL_AUTO(type(qqq), field(x), field(y), field(z));

class AA {

};

class BB : public AA {};


TEST(PmtRefl, Constructor)
{
    qqq x{1, 2.0, 4.1};
    auto xx = std::make_shared<struct_wrapper<qqq>>(x);
    std::shared_ptr<struct_wrapper_base> b = xx;
    //std::variant<std::shared_ptr<struct_wrapper_base>, int> aa = xx;
    std::variant<std::string, bool> var5 {std::in_place_index<0>, "ABCDE", 3};
    pmt q(std::in_place_type<uint32_t>, 4);
    //pmt z(std::in_place_type<std::shared_ptr<struct_wrapper_base>>, xx);
}

TEST(PmtRefl, Serialize)
{
    qqq x{1, 2.0, 4.1};
    auto xx = pmt_from_struct(x);
    std::stringbuf sb;
    pmtv::serialize(sb, xx);
    /*
    TODO:
      Write function that goes from serialized struct back to struct.
      Create serialized struct datatype that is just a wrapper around a string.
      Add above as pmt type
      Serialize/Des really easy.
      Write/Modify function to to work serialized struct or pre-serialized struct.
    */
    //auto y = pmtv::deserialize(sb);
    //EXPECT_EQ(x == y, true);

}


