/*-*-c++-*-*/
/*
 * Copyright 2021 John Sallay
 *
 * SPDX-License-Identifier: LGPL-3.0
 *
 */
#include <pmtf/vector.hpp>
#include <algorithm>

namespace flatbuffers {
    pmtf::Complex128 Pack(const std::complex<double>& obj) {
        return pmtf::Complex128(obj.real(), obj.imag());
    }
    std::complex<double> UnPack(const pmtf::Complex128& obj) {
        return std::complex<double>(obj.re(), obj.im());
    }
}
namespace pmtf {

std::ostream& operator<<(std::ostream& os, const vector_wrap& value) {
    return operator<<(os, value.get_pmt_buffer());
}

} // namespace pmtf
