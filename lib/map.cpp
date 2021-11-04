
#include <pmtf/map.hpp>
#include <map>
#include <flatbuffers/flatbuffers.h>

namespace pmtf {

template <>
map_value<std::string>::map_value() : 
    base(Data::MapString)
{
    // Don't need anything here.
}

template <>
map_value<std::string>::map_value(const map_type& val) : 
    base(Data::MapString), 
    _map(val) 
{
    // Don't need anything here.
}

template <>
map_value<std::string>::map_value(const uint8_t* buf, size_t size):
    base(Data::MapString)
{
    set_buffer(buf, size);
    // Possibly make this conversion lazy.
    auto pmt = GetSizePrefixedPmt(_buf);
    auto entries = pmt->data_as_MapString()->entries();
    for (size_t k=0; k<entries->size(); k++)
    {
        _map[entries->Get(k)->key()->str()] = base::from_pmt(entries->Get(k)->value());       
    }
}

template <>
flatbuffers::Offset<void> map_value<std::string>::rebuild_data(flatbuffers::FlatBufferBuilder& fbb)
{
    throw std::runtime_error("This should not get called");
}

template <>
wrap& map<std::string>::operator[](const std::string& key)
{
    return this->d_ptr->value()[key];
}

template <>
void map_value<std::string>::fill_flatbuffer()
{
    _fbb.Reset();
    std::vector<flatbuffers::Offset<MapEntryString>> entries;

    for( auto& [key, val] : _map )
    {
        auto str = _fbb.CreateString(key.c_str());
        auto pmt_offset = val.ptr()->build(_fbb);
        entries.push_back(CreateMapEntryString(_fbb, str, pmt_offset ));
    }

    auto vec = _fbb.CreateVectorOfSortedTables(&entries);
    MapStringBuilder mb(_fbb);
    mb.add_entries(vec);
    _data = mb.Finish().Union();
    build();
}

template <>
void map_value<std::string>::serialize_setup()
{
    fill_flatbuffer();
}

template <> wrap::wrap<std::map<std::string,wrap>>(const std::map<std::string,wrap>& x) { d_ptr = map<std::string>(x).ptr(); }
template <> wrap::wrap<map<std::string>>(const map<std::string>& x) { d_ptr = x.ptr(); }

template class map_value<std::string>;
template class map<std::string>;

}

