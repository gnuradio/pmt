#include <pmtf/pmtf_string.hpp>
#include <map>

namespace pmtf {

flatbuffers::Offset<void> pmt_string_value::rebuild_data(flatbuffers::FlatBufferBuilder& fbb)
{
    // fbb.Reset();
    return CreatePmtStringDirect(fbb, value().c_str()).Union();
}


void pmt_string_value::set_value(const std::string& val)
{
    _data = CreatePmtStringDirect(_fbb, val.c_str()).Union();
    build();
}

pmt_string_value::pmt_string_value(const std::string& val)
    : pmt_base(Data::PmtString)
{
    set_value(val);
}

pmt_string_value::pmt_string_value(const uint8_t *buf)
    : pmt_base(Data::PmtString)
{
    auto data = GetPmt(buf)->data_as_PmtString()->value();
    set_value(*((const std::string*)data));
}

pmt_string_value::pmt_string_value(const pmtf::Pmt* fb_pmt)
    : pmt_base(Data::PmtString)
{
    auto data = fb_pmt->data_as_PmtString()->value();
    set_value(*((const std::string*)data));
}

std::string pmt_string_value::value() const
{
    auto pmt = GetSizePrefixedPmt(_fbb.GetBufferPointer());
    return std::string(pmt->data_as_PmtString()->value()->str());
}

char* pmt_string_value::writable_elements()                                 
{                                                                                   
    auto pmt =                                                                      
        GetMutablePmt(buffer_pointer() + 4); /* assuming size prefix is 32 bit */   
    auto mutable_obj = ((pmtf::VectorInt8*)pmt->mutable_data())                 
                           ->mutable_value()
                           ->Data();                                   
    return (char*)(mutable_obj); /* hacky cast */                               
}                                                                                   

const char* pmt_string_value::elements() const                                
{                                                                                 
    auto pmt = GetSizePrefixedPmt(_buf);                                          
    auto fb_vec = pmt->data_as_PmtString()->value();                         
    return (const char*)(fb_vec->Data());                                           
}                                                                                 
} // namespace pmtf
