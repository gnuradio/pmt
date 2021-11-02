/*-*-c++-*-*/
/*
 * Copyright 2021 John Sallay
 *
 * SPDX-License-Identifier: LGPL-3.0
 *
 */
#include <pmtf/wrap.hpp>

namespace pmtf {

std::ostream& operator<<(std::ostream& os, const wrap& x) {
    // We need a virtual member function so that we can use polymorphism.
    x.ptr()->print(os);
    return os;
}

template <> wrap::wrap<decltype(nullptr)>(const decltype(nullptr)& x) { d_ptr = nullptr; }
template <> bool operator==<decltype(nullptr)>(const wrap& x, const decltype(nullptr)& other) {
        return false;  
    }
}
