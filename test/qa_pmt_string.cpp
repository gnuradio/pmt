#include <gtest/gtest.h>

#include <pmtf/pmtf.hpp>
#include <pmtf/pmtf_string.hpp>

#include <iostream>

using namespace pmtf;

TEST(Pmt, PmtStringTests)
{
    {
    auto str_pmt = pmt_string("hello");
    EXPECT_EQ(str_pmt, "hello");
    }
    {
    pmt_string str_pmt = "goodbye";
    EXPECT_EQ(str_pmt, "goodbye");
    }
}


TEST(Pmt, PmtStringAssignment)
{
    auto str_pmt = pmt_string("hello");
    
    str_pmt = "goodbye";

    EXPECT_EQ(str_pmt, "goodbye");
}


TEST(Pmt, PmtStringSerdes)
{
    auto str_pmt = pmt_string("hello");
    
    std::stringbuf sb; // fake channel
    sb.str("");        // reset channel to empty
    bool ret = str_pmt.ptr()->serialize(sb);
    std::cout << ret << std::endl;
    auto base_ptr = pmt_base::deserialize(sb);

    
    EXPECT_EQ(get_pmt_string(base_ptr), "hello");
}
