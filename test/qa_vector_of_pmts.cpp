/*-*-c++-*-*/
/*
 * Copyright 2021 John Sallay
 *
 * SPDX-License-Identifier: LGPL-3.0
 *
 */
#include <gtest/gtest.h>
#include <complex>

#include <pmtv/uniform_vector.hpp>

#include <list>
#include <map>

using namespace pmtv;

/*
How to do the map wrapper???
0) Derived class 
    Has all the same methods.
    Works anywhere a pmt is needed.
    Need to use pointers, which is a little different than what I'm doing.
    Allows for custom constructors.
1) View class - Get a mutable view into a pmt that acts as a map.
2) Helper class - Just like fb pmts.

What does it need to do???
Construct from an initializer list.
Range based for loop.
operator[] (lookup and add)
Cheap copies (could be moves)    
*/

TEST(PmtVectorPmt, Constructor) {
    
}
