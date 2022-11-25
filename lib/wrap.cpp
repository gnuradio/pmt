/*-*-c++-*-*/
/*
 * Copyright 2021 John Sallay
 *
 * SPDX-License-Identifier: LGPL-3.0
 *
 */
#include <pmtf/wrap.hpp>

namespace pmtf {

template <>
std::ostream& operator<< <pmt, true>(std::ostream& os, const pmt& value) {
    switch(value.data_type()) {
        case Data::PmtString: return operator<<(os, string(value));
        case Data::ScalarFloat32: return operator<<(os, scalar<float>(value));
        case Data::ScalarFloat64: return operator<<(os, scalar<double>(value));
        case Data::ScalarComplex64: return operator<<(os, scalar<std::complex<float>>(value));
        case Data::ScalarComplex128: return operator<<(os, scalar<std::complex<double>>(value));
        case Data::ScalarInt8: return operator<<(os, scalar<int8_t>(value));
        case Data::ScalarInt16: return operator<<(os, scalar<int16_t>(value));
        case Data::ScalarInt32: return operator<<(os, scalar<int32_t>(value));
        case Data::ScalarInt64: return operator<<(os, scalar<int64_t>(value));
        case Data::ScalarUInt8: return operator<<(os, scalar<uint8_t>(value));
        case Data::ScalarUInt16: return operator<<(os, scalar<uint16_t>(value));
        case Data::ScalarUInt32: return operator<<(os, scalar<uint32_t>(value));
        case Data::ScalarUInt64: return operator<<(os, scalar<uint64_t>(value));
        case Data::ScalarBool: return operator<<(os, scalar<bool>(value));
        case Data::VectorFloat32: return operator<<(os, vector<float>(value));
        case Data::VectorFloat64: return operator<<(os, vector<double>(value));
        case Data::VectorComplex64: return operator<<(os, vector<std::complex<float>>(value));
        case Data::VectorComplex128: return operator<<(os, vector<std::complex<double>>(value));
        case Data::VectorInt8: return operator<<(os, vector<int8_t>(value));
        case Data::VectorInt16: return operator<<(os, vector<int16_t>(value));
        case Data::VectorInt32: return operator<<(os, vector<int32_t>(value));
        case Data::VectorInt64: return operator<<(os, vector<int64_t>(value));
        case Data::VectorUInt8: return operator<<(os, vector<uint8_t>(value));
        case Data::VectorUInt16: return operator<<(os, vector<uint16_t>(value));
        case Data::VectorUInt32: return operator<<(os, vector<uint32_t>(value));
        case Data::VectorUInt64: return operator<<(os, vector<uint64_t>(value));
        case Data::VectorPmtHeader: return operator<<(os, vector<pmt>(value));
        case Data::MapHeaderString: return operator<<(os, map(value));
        case Data::Tag: return operator<<(os, tag(value));
        default:
            throw std::runtime_error("Unknown pmt type passed to operator<<");
    }
}

size_t pmt::elements() const {
    switch(data_type()) {
        case Data::PmtString:
            return string(*this).size();
        case Data::ScalarFloat32:
        case Data::ScalarFloat64:
        case Data::ScalarComplex64:
        case Data::ScalarComplex128:
        case Data::ScalarInt8:
        case Data::ScalarInt16:
        case Data::ScalarInt32:
        case Data::ScalarInt64:
        case Data::ScalarUInt8:
        case Data::ScalarUInt16:
        case Data::ScalarUInt32:
        case Data::ScalarUInt64:
        case Data::ScalarBool:
            return 1;
        case Data::VectorFloat32: return vector<float>(*this).size();
        case Data::VectorFloat64: return vector<double>(*this).size();
        case Data::VectorComplex64: return vector<std::complex<float>>(*this).size();
        case Data::VectorComplex128: return vector<std::complex<double>>(*this).size();
        case Data::VectorInt8: return vector<int8_t>(*this).size();
        case Data::VectorInt16: return vector<int16_t>(*this).size();
        case Data::VectorInt32: return vector<int32_t>(*this).size();
        case Data::VectorInt64: return vector<int64_t>(*this).size();
        case Data::VectorUInt8: return vector<uint8_t>(*this).size();
        case Data::VectorUInt16: return vector<uint16_t>(*this).size();
        case Data::VectorUInt32: return vector<uint32_t>(*this).size();
        case Data::VectorUInt64: return vector<uint64_t>(*this).size();
        case Data::VectorPmtHeader: return vector<pmt>(*this).size();
        case Data::MapHeaderString: return map(*this).size();
        case Data::Tag: return 1;
        default:
            throw std::runtime_error("Unknown pmt type passed to operator<<");
    }

}

void pmt::pre_serial_update() const {
    // If other is a pmt, then we will convert the first arg to its type
    // Then we will call this again to convert the second arg.
    std::cout << (data_type() == Data::MapHeaderString) << " " <<  (data_type() == Data::Tag) << std::endl;
    switch (data_type()) {
        case Data::MapHeaderString:
            map(*this).pre_serial_update();
            break;
        case Data::Tag:
            tag(*this).pre_serial_update();
            break;
        default:
            return;
    }
}

}
