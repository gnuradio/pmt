/*-*-c++-*-*/
/*
 * Copyright 2021 John Sallay
 *
 * SPDX-License-Identifier: LGPL-3.0
 *
 */
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include <pmtf/base.hpp>
#include <pmtf/pmtf_scalar.hpp>
#include <pmtf/pmtf_vector.hpp>

using namespace pmtf;
using namespace std;

int main(int argc, char* argv[])
{

    // auto cplx_pmt = pmt_vector<std::complex<float>>::make(std::complex<float>(1.2,-3.4));
    // std::cout << cplx_pmt.value() << std::endl;

    auto x = std::vector<int>{ 4, 5, 6 };
    auto int_vec_pmt2 = pmt_vector<int32_t>{ 7, 8, 9 };

    int_vec_pmt2[1] = 3;
    std::cout << int_vec_pmt2[1] << std::endl;


    std::stringbuf sb; // fake channel
    sb.str("");        // reset channel to empty
    bool ret = int_vec_pmt2.ptr()->serialize(sb);
    std::cout << ret << std::endl;
    auto base_ptr = base::deserialize(sb);

    // std::cout << "isequal: " << (*int_vec_pmt2 == *base_ptr) << std::endl;

    // for (auto d : vec) {
    //     std::cout << d << ",";
    // }
    std::cout << std::endl;
}
