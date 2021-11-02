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

flatbuffers::Offset<void> string_value::rebuild_data(flatbuffers::FlatBufferBuilder& fbb)
{
    // fbb.Reset();
    return CreatePmtStringDirect(fbb, value().c_str()).Union();
}


void string_value::set_value(const char* val)
{
    _data = CreatePmtStringDirect(_fbb, val).Union();
    build();
}

void string_value::set_value(const std::string& val)
{
    _data = CreatePmtStringDirect(_fbb, val.c_str()).Union();
    build();
}

string_value::string_value(const std::string& val)
    : pmt_base(Data::PmtString)
{
    set_value(val);
}

string_value::string_value(const uint8_t *buf)
    : pmt_base(Data::PmtString)
{
    auto data = GetPmt(buf)->data_as_PmtString()->value();
    set_value(*((const std::string*)data));
}

string_value::string_value(const pmtf::Pmt* fb_pmt)
    : pmt_base(Data::PmtString)
{
    auto data = fb_pmt->data_as_PmtString()->value();
    set_value(data->c_str());
}

std::string string_value::value() const
{
    auto pmt = GetSizePrefixedPmt(_fbb.GetBufferPointer());
    return std::string(pmt->data_as_PmtString()->value()->str());
}

char* string_value::writable_elements()                                 
{                                                                                   
    auto pmt =                                                                      
        GetMutablePmt(buffer_pointer() + 4); /* assuming size prefix is 32 bit */   
    auto mutable_obj = ((pmtf::VectorInt8*)pmt->mutable_data())                 
                           ->mutable_value()
                           ->Data();                                   
    return (char*)(mutable_obj); /* hacky cast */                               
}                                                                                   

const char* string_value::elements() const                                
{                                                                                 
    auto pmt = GetSizePrefixedPmt(_buf);                                          
    auto fb_vec = pmt->data_as_PmtString()->value();                         
    return (const char*)(fb_vec->Data());                                           
}         

string get_string(const wrap& x) {
    // Make sure that this is the right type.
    switch(x.ptr()->data_type()) {
        case Data::PmtString:
             return string(std::dynamic_pointer_cast<string_value>(x.ptr()));
        default:
            throw std::runtime_error("Cannot convert non string pmt.");
    }
}

template <> wrap::wrap<std::string>(const std::string& x) { d_ptr = string(x).ptr(); }
template <> wrap::wrap<string>(const string& x) { d_ptr = x.ptr(); }
// template <> wrap::wrap<char *>(const char *x) { d_ptr = string(x).ptr(); }

} // namespace pmtf
