/*-*-c++-*-*/
/*
 * Copyright 2021 John Sallay
 *
 * SPDX-License-Identifier: LGPL-3.0
 *
 */
#include <pmtf/alloc.hpp>
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

pmt::pmt(const std::shared_ptr<base_buffer>& other) {
    _scalar = other;
    _map = nullptr;
}

pmt::pmt(const pmt& other) {
    _scalar = other._scalar;
    _vector = other._vector;
    _map = other._map;
}

pmt& pmt::operator=(const pmt& other) {
    _scalar = other._scalar;
    _vector = other._vector;
    _map = other._map;
    return *this;
}

size_t pmt::serialize(std::streambuf& sb) const {
    // Allow for object to do any pre-serialization updates.
    // Namely, a map can update it's size.
    pre_serial_update();
    size_t length = 0;
    length += sb.sputn(reinterpret_cast<const char*>(_scalar->raw()), _scalar->size());
    if (_vector) {
        for (const auto& v: *_vector) {
            length += v.serialize(sb);
        }
    }
    if (_map) {
        uint32_t size;
        for (const auto& [k, v]: *_map) {
            // For right now just prefix the size to the key and send it
            size = k.size();
            length += sb.sputn(reinterpret_cast<const char*>(&size), sizeof(uint32_t));
            length += sb.sputn(k.c_str(), size);
            length += v.serialize(sb);
        }
    }
    return length;
}

pmt pmt::deserialize(std::streambuf& sb) {
    uint32_t size;
    sb.sgetn(reinterpret_cast<char*>(&size), sizeof(size));
    AlignedAllocator* aa = new AlignedAllocator;
    char* x = reinterpret_cast<char*>(aa->allocate(size + sizeof(uint32_t)));
    *reinterpret_cast<uint32_t*>(x) = size;
    sb.sgetn(x + sizeof(uint32_t), size);
    flatbuffers::DetachedBuffer buf(aa, true, reinterpret_cast<uint8_t*>(x), size + sizeof(uint32_t), reinterpret_cast<uint8_t*>(x), size+sizeof(uint32_t));
    pmt cur(std::make_shared<base_buffer>(std::move(buf)));
    if (cur.data_type() == Data::VectorPmtHeader) {
        uint32_t count = cur._scalar->data_as<VectorPmtHeader>()->count();
        cur._vector = std::make_shared<std::vector<pmt>>(count);
        for (size_t i = 0; i < count; i++) {
            (*cur._vector)[i] = deserialize(sb);
        }
    } else if (cur.data_type() == Data::MapHeaderString || cur.data_type() == Data::Tag) {
        cur._map = std::make_shared<std::map<std::string, pmt>>();
        uint32_t count;
        if (cur.data_type() == Data::MapHeaderString)
            count = cur._scalar->data_as<MapHeaderString>()->count();
        else
            count = cur._scalar->data_as<Tag>()->count();
        std::vector<char> data;
        for (size_t i = 0; i < count; i++) {
            // Read length then string
            sb.sgetn(reinterpret_cast<char*>(&size), sizeof(uint32_t));
            data.resize(size);
            sb.sgetn(data.data(), size);
            // Deserialize the pmt map value
            (*cur._map)[std::string(data.begin(), data.end())] = deserialize(sb);
        }
    }
    return cur;
}

Data pmt::data_type() const {
    if (_scalar != nullptr) {
        return _scalar->data_type();
    }
    throw std::runtime_error("Cannot get data type for unitialized pmt");
}

std::string pmt::type_string() const noexcept {
    if (_scalar != nullptr)
        return std::string(EnumNameData(data_type()));
    else return "Uninitialized";
}



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
