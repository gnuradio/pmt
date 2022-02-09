/*-*-c++-*-*/
/*
 * Copyright 2021 John Sallay
 *
 * SPDX-License-Identifier: LGPL-3.0
 *
 */
#include <pmtf/base.hpp>
#include <pmtf/scalar.hpp>
#include <pmtf/string.hpp>
#include <pmtf/vector.hpp>
#include "base64/base64.h"
#include <map>

namespace flatbuffers {
pmtf::Complex64 Pack(const std::complex<float>& obj)
{
    return pmtf::Complex64(obj.real(), obj.imag());
}

const std::complex<float> UnPack(const pmtf::Complex64& obj)
{
    return std::complex<float>(obj.re(), obj.im());
}
} // namespace flatbuffers

namespace pmtf {


///////////////////////////////////////////////////////////////////////////////////
// Static PMT Base functions
///////////////////////////////////////////////////////////////////////////////////

/*base::sptr base::from_pmt(const pmtf::Pmt* fb_pmt)
{
    switch (fb_pmt->data_type()) {
    case Data::PmtString:
        return std::static_pointer_cast<base>(string_value::from_pmt(fb_pmt));
    case Data::ScalarComplex64:
        return std::static_pointer_cast<base>(
            scalar_value<std::complex<float>>::from_pmt(fb_pmt));
    case Data::ScalarComplex128:
        return std::static_pointer_cast<base>(
            scalar_value<std::complex<double>>::from_pmt(fb_pmt));
    case Data::ScalarInt8:
        return std::static_pointer_cast<base>(scalar_value<int8_t>::from_pmt(fb_pmt));
    case Data::ScalarUInt8:
        return std::static_pointer_cast<base>(scalar_value<uint8_t>::from_pmt(fb_pmt));
    case Data::ScalarInt16:
        return std::static_pointer_cast<base>(scalar_value<int16_t>::from_pmt(fb_pmt));
    case Data::ScalarUInt16:
        return std::static_pointer_cast<base>(scalar_value<uint16_t>::from_pmt(fb_pmt));
    case Data::ScalarInt32:
        return std::static_pointer_cast<base>(scalar_value<int32_t>::from_pmt(fb_pmt));
    case Data::ScalarUInt32:
        return std::static_pointer_cast<base>(scalar_value<uint32_t>::from_pmt(fb_pmt));
    case Data::ScalarInt64:
        return std::static_pointer_cast<base>(scalar_value<int64_t>::from_pmt(fb_pmt));
    case Data::ScalarUInt64:
        return std::static_pointer_cast<base>(scalar_value<uint64_t>::from_pmt(fb_pmt));
    case Data::VectorInt8:
        return std::static_pointer_cast<base>(pmt_vector_value<int8_t>::from_pmt(fb_pmt));
    case Data::VectorUInt8:
        return std::static_pointer_cast<base>(pmt_vector_value<uint8_t>::from_pmt(fb_pmt));
    case Data::VectorInt16:
        return std::static_pointer_cast<base>(pmt_vector_value<int16_t>::from_pmt(fb_pmt));
    case Data::VectorUInt16:
        return std::static_pointer_cast<base>(pmt_vector_value<uint16_t>::from_pmt(fb_pmt));
    case Data::VectorInt32:
        return std::static_pointer_cast<base>(pmt_vector_value<int32_t>::from_pmt(fb_pmt));
    case Data::VectorUInt32:
        return std::static_pointer_cast<base>(pmt_vector_value<uint32_t>::from_pmt(fb_pmt));
    case Data::VectorInt64:
        return std::static_pointer_cast<base>(pmt_vector_value<int64_t>::from_pmt(fb_pmt));
    case Data::VectorUInt64:
        return std::static_pointer_cast<base>(pmt_vector_value<uint64_t>::from_pmt(fb_pmt));
    case Data::VectorFloat32:
        return std::static_pointer_cast<base>(pmt_vector_value<float>::from_pmt(fb_pmt));
    case Data::VectorFloat64:
        return std::static_pointer_cast<base>(pmt_vector_value<double>::from_pmt(fb_pmt));
    case Data::VectorComplex64:
        return std::static_pointer_cast<base>(pmt_vector_value<std::complex<float>>::from_pmt(fb_pmt));
    // case Data::VectorComplex128:
    //     return std::static_pointer_cast<base>(pmt_vector_value<std::complex<double>>::from_pmt(fb_pmt));

    default:
        throw std::runtime_error("Unsupported PMT Type");
    }
}

base::sptr base::from_buffer(const uint8_t* buf, size_t size)
{
    auto PMT = GetPmt(buf);
    return from_pmt(PMT);
}

template class scalar_value<std::complex<float>>;
template class pmt_vector_value<std::int32_t>;
// template class pmt_vector_value<std::complex<float>>;*/


std::string pmt::to_base64()
{
    std::stringbuf sb; 
    auto nbytes = serialize(sb);
    std::string pre_encoded_str(nbytes, '0');
    sb.sgetn(pre_encoded_str.data(), nbytes);
    auto nencoded_bytes = Base64encode_len(nbytes);
    std::string encoded_str(nencoded_bytes, '0');
    auto nencoded = Base64encode(encoded_str.data(), pre_encoded_str.data(), nbytes);
    encoded_str.resize(nencoded - 1); // because it null terminates
    return encoded_str;
}

pmtf::pmt pmt::from_base64(const std::string& encoded_str)
{
    std::string bufplain(encoded_str.size(), '0');
    Base64decode(bufplain.data(), encoded_str.data());
    std::stringbuf sb(bufplain);
    return pmtf::pmt::deserialize(sb); 
}

} // namespace pmtf
