#include <gtest/gtest.h>

#include <pmtf/base.hpp>
#include <pmtf/string.hpp>

#include <iostream>

using namespace pmtf;

TEST(PmtString, Basic)
{
    {
    auto str_pmt = string("hello");
    EXPECT_EQ(str_pmt, "hello");
    }
    {
    string str_pmt = "goodbye";
    EXPECT_EQ(str_pmt, "goodbye");
    }
}


TEST(PmtString, Assignment)
{
    auto str_pmt = string("hello");
    
    str_pmt = "goodbye";

    EXPECT_EQ(str_pmt, "goodbye");
}


TEST(PmtString, Serdes)
{
    auto str_pmt = string("hello");
    
    std::stringbuf sb; // fake channel
    sb.str("");        // reset channel to empty
    bool ret = str_pmt.ptr()->serialize(sb);
    std::cout << ret << std::endl;
    auto base_ptr = base::deserialize(sb);

    
    EXPECT_EQ(get_string(base_ptr), "hello");
}

TEST(Pmt, PmtWrap)
{
    wrap x;
    x = std::string("hello");
    
    EXPECT_EQ(get_string(x), "hello");
}
