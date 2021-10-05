/*-*-c++-*-*/
/*
 * Copyright 2021 John Sallay
 *
 * SPDX-License-Identifier: LGPL-3.0
 *
 */
#include <pmtf/pmtf_wrap.hpp>

namespace pmtf {

std::ostream& operator<<(std::ostream& os, const pmt_wrap& x) {
    // We need a virtual member function so that we can use polymorphism.
    x.ptr()->print(os);
    return os;
}

template <> pmt_wrap::pmt_wrap<decltype(nullptr)>(const decltype(nullptr)& x) { d_ptr = nullptr; }

}
