/*-*-c++-*-*/
/*
 * Copyright 2021 John Sallay
 *
 * SPDX-License-Identifier: LGPL-3.0
 *
 */
#include <pmtf/string.hpp>
#include <map>

namespace pmtf {
    gsl::span<char> string::value() {
        std::shared_ptr<base_buffer> scalar = _get_buf();
        auto buf = scalar->data_as<type>()->value();
        return gsl::span<char>(const_cast<char*>(buf->data()), buf->size());
    }
    std::string_view string::value() const {
        std::shared_ptr<base_buffer> scalar = _get_buf();
        auto buf = scalar->data_as<type>()->value();
        return std::string_view(buf->data(), buf->size());
    }
    string& string::operator=(const std::string& value) {
        _MakeString(value.data(), value.size());
        return *this;
    }
    string& string::operator=(const char value[]) {
        _MakeString(&value[0], std::string(value).size());
        return *this;
    }
    char& string::operator[] (size_type n) {
        // operator[] doesn't do bounds checking, use at for that
        // TODO: implement at
        return data()[n];
    }
    const char& string::operator[] (size_type n) const {
        return data()[n];
    }
    
    void string::_MakeString(const char* data, size_t size) {
        flatbuffers::FlatBufferBuilder fbb;
        auto offset = traits::Create(fbb, fbb.CreateString(data, size)).Union();
        auto pmt = CreatePmt(fbb, data_type(), offset);
        fbb.FinishSizePrefixed(pmt);
        _get_buf() = std::make_shared<base_buffer>(fbb.Release());
    }


    template <> pmt::pmt(const string& x) { *this = x.get_pmt_buffer(); }
    template <> pmt::pmt(const std::string& x) { *this = string(x).get_pmt_buffer(); }
    template <> pmt::pmt(const char* x) { *this = string(x).get_pmt_buffer(); }


    template <> pmt& pmt::operator=<std::string>(const std::string& x)
        { return operator=(string(x).get_pmt_buffer()); } 
    template <> pmt& pmt::operator=<string>(const string& x)
        { return operator=(x.get_pmt_buffer()); } 

} // namespace pmtf
