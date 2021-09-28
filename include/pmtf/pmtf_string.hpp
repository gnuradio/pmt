#pragma once

#include <pmtf/pmtf_generated.h>
#include <pmtf/pmtf.hpp>
#include <complex>
#include <iostream>
#include <map>
#include <memory>
#include <typeindex>
#include <typeinfo>


namespace pmtf {

class pmt_string_value : public pmt_base
{
public:
    typedef std::shared_ptr<pmt_string_value> sptr;
    static sptr make(const std::string& value)
    {
        return std::make_shared<pmt_string_value>(value);
    }
    static sptr from_buffer(const uint8_t* buf)
    {
        return std::make_shared<pmt_string_value>(buf);
    }
    static sptr from_pmt(const pmtf::Pmt *fb_pmt)
    {
        return std::make_shared<pmt_string_value>(fb_pmt);
    }


    void set_value(const std::string& val);
    std::string value() const;
    char* writable_elements();
    const char* elements() const;

    void operator=(const std::string& other) // copy assignment
    {
        set_value(other);
    }

    bool operator==(const std::string& other) { return other == value(); }
    bool operator!=(const std::string& other) { return other != value(); }

    flatbuffers::Offset<void> rebuild_data(flatbuffers::FlatBufferBuilder& fbb);

    pmt_string_value(const std::string& val);
    pmt_string_value(const uint8_t* buf);
    pmt_string_value(const pmtf::Pmt *fb_pmt);
    void print(std::ostream& os) const { os << value(); }
};

class pmt_string {
  public:
    using value_type = char;
    using reference = char&;
    using size_type = size_t;
    using sptr = pmt_string_value::sptr;
    pmt_string(const std::string& str):
        d_ptr(pmt_string_value::make(str)) {}
    pmt_string(const std::string& str, size_t pos, size_t len = std::string::npos):
        d_ptr(pmt_string_value::make(std::string(str, pos, len))) {}
    pmt_string(const char* s):
        d_ptr(pmt_string_value::make(std::string(s))) {}
    pmt_string(const char* s, size_t n):
        d_ptr(pmt_string_value::make(std::string(s, n))) {}

    pmt_string(sptr p):
        d_ptr(p) {}

    // TODO: Think about real iterators instead of pointers.
    value_type* begin() const { return d_ptr->writable_elements(); }
    value_type* end() const { return d_ptr->writable_elements() + size(); }
    const value_type* cbegin() { return d_ptr->writable_elements(); }
    const value_type* cend() { return d_ptr->writable_elements() + size(); }

    reference operator[] (size_type n) {
        // operator[] doesn't do bounds checking, use at for that
        // TODO: implement at
        auto data = d_ptr->writable_elements();
        return data[n];
    }
    const char& operator[] (size_type n) const {
        const char* data = d_ptr->elements();
        const char& x = data[n];
        return x;
    }
    
    sptr ptr() { return d_ptr; }
    size_type size() const { return d_ptr->size(); }
    auto data_type() { return d_ptr->data_type(); }
    
  private:
    sptr d_ptr;
};


// When we switch to c++20, make this a concept.
template <class U>
bool operator==(const pmt_string& x, const U& other) {
    if (other.size() != x.size()) return false;
    auto my_val = x.begin();
    for (auto&& val : other) {
        if (*my_val != val) return false;
        my_val++;
    }
    return true;
}

inline bool operator==(const pmt_string& x, const char other[]) {
    size_t index = 0;
    while (other[index] != 0 && index < x.size()) {
        if (other[index] != x[index]) return false;
        index++;
    }
    return index == x.size() || x[index] == 0;
}

inline std::ostream& operator<<(std::ostream& os, const pmt_string& value) {
    for (auto& v: value)
        os << v;
    return os;
}

} // namespace pmtf
