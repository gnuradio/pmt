/*-*-c++-*-*/
/*
 * Copyright 2021 John Sallay
 *
 * SPDX-License-Identifier: LGPL-3.0
 *
 */
#pragma once

#include <pmtf/pmtf_generated.h>
#include <pmtf/base.hpp>
#include <complex>
#include <ostream>
#include <map>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <type_traits>


namespace pmtf {

template <class T>
inline flatbuffers::Offset<void> CreateScalar(flatbuffers::FlatBufferBuilder& fbb, const T& value);

template <class T> struct scalar_traits;
template <> struct scalar_traits<uint8_t> { using traits = ScalarUInt8::Traits; };
template <> struct scalar_traits<uint16_t> { using traits = ScalarUInt16::Traits; };
template <> struct scalar_traits<uint32_t> { using traits = ScalarUInt32::Traits; };
template <> struct scalar_traits<uint64_t> { using traits = ScalarUInt64::Traits; };
template <> struct scalar_traits<int8_t> { using traits = ScalarInt8::Traits; };
template <> struct scalar_traits<int16_t> { using traits = ScalarInt16::Traits; };
template <> struct scalar_traits<int32_t> { using traits = ScalarInt32::Traits; };
template <> struct scalar_traits<int64_t> { using traits = ScalarInt64::Traits; };
template <> struct scalar_traits<float> { using traits = ScalarFloat32::Traits; };
template <> struct scalar_traits<double> { using traits = ScalarFloat64::Traits; };
template <> struct scalar_traits<std::complex<float>> { using traits = ScalarComplex64::Traits; };
template <> struct scalar_traits<std::complex<double>> { using traits = ScalarComplex128::Traits; };

template <class T>
class scalar {
public:
    using traits = typename scalar_traits<T>::traits;
    using type = typename traits::type;
    scalar() { _Create(T(0)); }
    scalar(const T& value) { _Create(value); }
    scalar(const scalar<T>& other) { _Create(other.value()); }
    scalar(const pmt& other): _buf(other) {} 
    ~scalar() {}
    T value() const {
        std::shared_ptr<base_buffer> scalar = _get_buf();
        if constexpr(is_complex<T>::value)
            return *reinterpret_cast<const T*>(scalar->data_as<type>()->value());
        else
            return scalar->data_as<type>()->value();
    }
    static constexpr Data data_type() { return DataTraits<type>::enum_value; }
    scalar& operator=(const T& value) {
        std::shared_ptr<base_buffer> scalar = _get_buf();
        if constexpr(is_complex<T>::value) {
            auto mv = scalar->data_as<type>()->mutable_value();
            mv->mutate_re(value.real());
            mv->mutate_im(value.imag());
        } else
            scalar->data_as<type>()->mutate_value(value);
        return *this;        
    }
    scalar& operator=(const scalar<T>& value) {
        return this->operator=(value.value());
    }
    const pmt& get_pmt_buffer() const { return _buf; }
    void print(std::ostream& os) const { os << value(); }
    // Cast operators
    //! Cast to a T value.
    //! Explicit means that the user must do something like T(scalar<T>(val));
    //! Implicit conversions can cause lots of problems, so we are avoiding them.
    explicit operator T() const { return value(); }
    //! Cast to another type
    //! Will cause a compilation failure if we can't do the cast.
    template <class U>
    explicit operator U() const { return U(value()); }
private:
    void _Create(const T& value) {
        flatbuffers::FlatBufferBuilder fbb(128);
        flatbuffers::Offset<void> offset;
        //auto offset = traits::Create(fbb, value).Union();
        if constexpr(is_complex<T>::value) {
            auto ptr = reinterpret_cast<const typename scalar_type<T>::type*>(&value);
            offset = traits::Create(fbb, ptr).Union();
        } else {
            offset = traits::Create(fbb, value).Union();
        }
        auto pmt = CreatePmt(fbb, data_type(), offset);
        fbb.FinishSizePrefixed(pmt);
        _get_buf() = std::make_shared<base_buffer>(fbb.Release());
    }
    std::shared_ptr<base_buffer>& _get_buf() { return _buf._scalar; }
    const std::shared_ptr<base_buffer> _get_buf() const { return _buf._scalar; }
    pmt _buf;
    
};

template <class T>
struct is_scalar : std::false_type {};

template <class T>
struct is_scalar<scalar<T>> : std::true_type {};

// In C++20 we can replace this with a concept.
#define IMPLEMENT_SCALAR_PMT(type) \
template <> inline pmt& pmt::operator=<type>(const type& x) \
    { return operator=(scalar(x).get_pmt_buffer()); } \
template <> inline pmt& pmt::operator=<scalar<type>>(const scalar<type>& x) \
    { return operator=(x.get_pmt_buffer()); } \
template <> inline pmt::pmt<type>(const type& x) \
    { operator=(scalar(x).get_pmt_buffer()); } \
template <> inline pmt::pmt<scalar<type>>(const scalar<type>& x) \
    { operator=(x.get_pmt_buffer()); }

IMPLEMENT_SCALAR_PMT(uint8_t)
IMPLEMENT_SCALAR_PMT(uint16_t)
IMPLEMENT_SCALAR_PMT(uint32_t)
IMPLEMENT_SCALAR_PMT(uint64_t)
IMPLEMENT_SCALAR_PMT(int8_t)
IMPLEMENT_SCALAR_PMT(int16_t)
IMPLEMENT_SCALAR_PMT(int32_t)
IMPLEMENT_SCALAR_PMT(int64_t)
IMPLEMENT_SCALAR_PMT(float)
IMPLEMENT_SCALAR_PMT(double)
IMPLEMENT_SCALAR_PMT(std::complex<float>)
IMPLEMENT_SCALAR_PMT(std::complex<double>)


template <class T, class U>
bool operator==(const scalar<T>& x, const U& y) {
    // U is a plain old data type (scalar<float> == float)
    if constexpr(std::is_same_v<T, U>)
        return x.value() == y;
    // U is another pmt::scalar
    else if constexpr(is_scalar<U>::value)
        return x.value() == y.value();
    // Can U be converted to T?
    else if constexpr(std::is_convertible_v<U, T>)
        return x.value() == T(y);
    return false;
}

template <class T>
std::ostream& operator<<(std::ostream& os, const scalar<T>& x) {
    os << x.value();
    return os;
}

template <class T>
scalar<T> get_scalar(const pmt& p) {
    if (p.data_type() == scalar<T>::data_type())
        return scalar<T>(p);
    // This error message stinks.  Fix it.
    throw std::runtime_error("Can't convert pmt to this type");
}

// These structures allow us to write template functions that depend on the
// flatbuffer data type.  This allows us to do things like verify that the
// datatype is correct when we want to interpret a pmt as another type.
template <> struct cpp_type<Data::ScalarInt8> { using type=int8_t; };
template <> struct cpp_type<Data::ScalarInt16> { using type=int16_t; };
template <> struct cpp_type<Data::ScalarInt32> { using type=int32_t; };
template <> struct cpp_type<Data::ScalarInt64> { using type=int64_t; };
template <> struct cpp_type<Data::ScalarUInt8> { using type=uint8_t; };
template <> struct cpp_type<Data::ScalarUInt16> { using type=uint16_t; };
template <> struct cpp_type<Data::ScalarUInt32> { using type=uint32_t; };
template <> struct cpp_type<Data::ScalarUInt64> { using type=uint64_t; };
template <> struct cpp_type<Data::ScalarFloat32> { using type=float; };
template <> struct cpp_type<Data::ScalarFloat64> { using type=double; };
template <> struct cpp_type<Data::ScalarComplex64> { using type=std::complex<float>; };
template <> struct cpp_type<Data::ScalarComplex128> { using type=std::complex<double>; };
template <> struct cpp_type<Data::ScalarBool> { using type=bool; };
/**
 * @brief Class holds the implementation of a scalar pmt.
 *
 * The scalar types are defined in scalar.cpp.  This class should not be
 * used directly. It would be nice to move it to the .cpp and instantiate all
 * of the templates in there. This would involve a fairly large refactoring of
 * the code.
 */
/*template <class T>
class scalar_value : public base
{
public:
    typedef std::shared_ptr<scalar_value> sptr;
    static sptr make(const T value) { return std::make_shared<scalar_value<T>>(value); }
    static sptr from_buffer(const uint8_t* buf)
    {
        return std::make_shared<scalar_value<T>>(buf);
    }
    static sptr from_pmt(const pmtf::Pmt* fb_pmt)
    {
        return std::make_shared<scalar_value<T>>(fb_pmt);
    }

    void set_value(const T& val);
    T value();
    const T value() const;

    scalar_value& operator=(const T& other) // copy assignment
    {
        set_value(other);
        return *this;
    }
    scalar_value& operator=(const scalar_value& other)
    {
        if (this == &other) return *this;
        this->set_value(other.value());
        return *this;
    }

    flatbuffers::Offset<void> rebuild_data(flatbuffers::FlatBufferBuilder& fbb);

    scalar_value(const T& val);
    scalar_value(const uint8_t* buf);
    scalar_value(const pmtf::Pmt* fb_pmt);

    bool is_scalar() const noexcept { return true; }
    void print(std::ostream& os) const { os << value(); }
    
};

// These structures allow us to see if a arbitrary type is a scalar_value
// or not.
template <class T>
struct is_pmt_scalar_value : std::false_type {};

template <class T>
struct is_pmt_scalar_value<scalar_value<T>> : std::true_type {};*/

/**
 * @brief compare scalar_value against something else
 *
 * Allow for comparisons against other pmt scalars and other types.
 * For example scalar_value<int>(4) == 4.0 will be true.
 */
/*template <class T, class U>
bool operator==(const scalar_value<T>& x, const U& y) {
    if constexpr(std::is_same_v<T, U>)
        return x.value() == y;
    else if constexpr(is_pmt_scalar_value<U>::value)
        return x.value() == y.value();
    else if constexpr(std::is_convertible_v<U, T>)
        return x.value() == T(y);
    return false;
}

// These structures allow us to write template functions that depend on the
// flatbuffer data type.  This allows us to do things like verify that the
// datatype is correct when we want to interpret a pmt as another type.
template <> struct cpp_type<Data::ScalarInt8> { using type=int8_t; };
template <> struct cpp_type<Data::ScalarInt16> { using type=int16_t; };
template <> struct cpp_type<Data::ScalarInt32> { using type=int32_t; };
template <> struct cpp_type<Data::ScalarInt64> { using type=int64_t; };
template <> struct cpp_type<Data::ScalarUInt8> { using type=uint8_t; };
template <> struct cpp_type<Data::ScalarUInt16> { using type=uint16_t; };
template <> struct cpp_type<Data::ScalarUInt32> { using type=uint32_t; };
template <> struct cpp_type<Data::ScalarUInt64> { using type=uint64_t; };
template <> struct cpp_type<Data::ScalarFloat32> { using type=float; };
template <> struct cpp_type<Data::ScalarFloat64> { using type=double; };
template <> struct cpp_type<Data::ScalarComplex64> { using type=std::complex<float>; };
template <> struct cpp_type<Data::ScalarComplex128> { using type=std::complex<double>; };
template <> struct cpp_type<Data::ScalarBool> { using type=bool; };*/

/**
 * @brief "Print" out a scalar_value
 */
/*template <class T>
std::ostream& operator<<(std::ostream& os, const scalar_value<T>& value) {
    os << value;
    return os;
}*/

/**
 * @brief Wrapper class around a smart pointer to a scalar_value.
 *
 * This is the interface that should be used for dealing with scalar values.
 */
/*template <class T>
class scalar {
public:
    using sptr = typename scalar_value<T>::sptr;
    //! Construct a scalar from a scalar value
    scalar(const T& val): d_ptr(scalar_value<T>::make(val)) {}
    //! Construct a scalar from a scalar_value pointer.
    scalar(sptr ptr):
        d_ptr(ptr) {}
    //! Copy constructor.
    scalar(const scalar<T>& x):
        d_ptr(x.d_ptr) {}
   
    //! Get at the smart pointer.
    sptr ptr() const { return d_ptr; }
    bool operator==(const T& val) const { return *d_ptr == val;}
    bool operator==(const scalar<T>& val) const { return *d_ptr == *val.d_ptr; }
    auto data_type() { return d_ptr->data_type(); }
    T value() const { return d_ptr->value(); }


    // Make it act like a pointer.  Probably need a better way
    // to think about it.
    T& operator*() const { return *d_ptr; }
    // Cast operators
    //! Cast to a T value.
    //! Explicit means that the user must do something like T(scalar<T>(val));
    //! Implicit conversions can cause lots of problems, so we are avoiding them.
    explicit operator T() const { return d_ptr->value(); }
    //! Cast to another type
    //! Will cause a compilation failure if we can't do the cast.
    template <class U>
    explicit operator U() const { return U(d_ptr->value()); }
    
private:
    sptr d_ptr;
};

template <class T>
std::ostream& operator<<(std::ostream& os, const scalar<T>& value) {
    os << *(value.ptr());
    return os;
}

template <class T, Data dt>
scalar<T> _get_pmt_scalar(const wrap& x) {
    if constexpr(std::is_same_v<typename cpp_type<dt>::type, T>)
        return scalar<T>(std::dynamic_pointer_cast<scalar_value<T>>(x.ptr()));
    else
        throw std::runtime_error("Cannot convert scalar types");
}

template <class T>
scalar<T> get_scalar(const wrap& x) {
    // Make sure that this is the right type.
    switch(auto dt = x.ptr()->data_type()) {
        case Data::ScalarFloat32: return _get_pmt_scalar<T, Data::ScalarFloat32>(x);
        case Data::ScalarFloat64: return _get_pmt_scalar<T, Data::ScalarFloat64>(x);
        case Data::ScalarComplex64: return _get_pmt_scalar<T, Data::ScalarComplex64>(x);
        case Data::ScalarComplex128: return _get_pmt_scalar<T, Data::ScalarComplex128>(x);
        case Data::ScalarInt8: return _get_pmt_scalar<T, Data::ScalarInt8>(x);
        case Data::ScalarInt16: return _get_pmt_scalar<T, Data::ScalarInt16>(x);
        case Data::ScalarInt32: return _get_pmt_scalar<T, Data::ScalarInt32>(x);
        case Data::ScalarInt64: return _get_pmt_scalar<T, Data::ScalarInt64>(x);
        case Data::ScalarUInt8: return _get_pmt_scalar<T, Data::ScalarUInt8>(x);
        case Data::ScalarUInt16: return _get_pmt_scalar<T, Data::ScalarUInt16>(x);
        case Data::ScalarUInt32: return _get_pmt_scalar<T, Data::ScalarUInt32>(x);
        case Data::ScalarUInt64: return _get_pmt_scalar<T, Data::ScalarUInt64>(x);
        case Data::ScalarBool: return _get_pmt_scalar<T, Data::ScalarBool>(x);
        default:
            throw std::runtime_error("Cannot convert non scalar pmt.");
    }
}

template <class T>
T get_scalar_value(const wrap& x) {
    return get_scalar<T>(x).ptr()->value();
}
// Fix this later with SFINAE
template <class T, Data dt>
bool _can_be(const wrap& x) {
    auto value = get_scalar<typename cpp_type<dt>::type>(x);
    return std::is_convertible_v<typename cpp_type<dt>::type, T>;
}

template <class T>
bool can_be(const wrap& x) {
    switch(auto dt = x.ptr()->data_type()) {
        case Data::ScalarFloat32: return _can_be<T, Data::ScalarFloat32>(x);
        case Data::ScalarFloat64: return _can_be<T, Data::ScalarFloat64>(x);
        case Data::ScalarComplex64: return _can_be<T, Data::ScalarComplex64>(x);
        case Data::ScalarComplex128: return _can_be<T, Data::ScalarComplex128>(x);
        case Data::ScalarInt8: return _can_be<T, Data::ScalarInt8>(x);
        case Data::ScalarInt16: return _can_be<T, Data::ScalarInt16>(x);
        case Data::ScalarInt32: return _can_be<T, Data::ScalarInt32>(x);
        case Data::ScalarInt64: return _can_be<T, Data::ScalarInt64>(x);
        case Data::ScalarUInt8: return _can_be<T, Data::ScalarUInt8>(x);
        case Data::ScalarUInt16: return _can_be<T, Data::ScalarUInt16>(x);
        case Data::ScalarUInt32: return _can_be<T, Data::ScalarUInt32>(x);
        case Data::ScalarUInt64: return _can_be<T, Data::ScalarUInt64>(x);
        case Data::ScalarBool: return _can_be<T, Data::ScalarBool>(x);
        //case Data::PmtString: return _can_be<T, Data::PmtString>(x);
        default: return false;
    }
    
}

template <class T, Data dt>
T _get_as(const wrap& x) {
    auto value = get_scalar<typename cpp_type<dt>::type>(x);
    if constexpr(std::is_convertible_v<typename cpp_type<dt>::type, T>)
        return T(value.ptr()->value());
    else
        throw std::runtime_error("Cannot convert types");
}

template <class T>
T get_as(const wrap& x) {
    switch(auto dt = x.ptr()->data_type()) {
        case Data::ScalarFloat32: return _get_as<T, Data::ScalarFloat32>(x);
        case Data::ScalarFloat64: return _get_as<T, Data::ScalarFloat64>(x);
        case Data::ScalarComplex64: return _get_as<T, Data::ScalarComplex64>(x);
        case Data::ScalarComplex128: return _get_as<T, Data::ScalarComplex128>(x);
        case Data::ScalarInt8: return _get_as<T, Data::ScalarInt8>(x);
        case Data::ScalarInt16: return _get_as<T, Data::ScalarInt16>(x);
        case Data::ScalarInt32: return _get_as<T, Data::ScalarInt32>(x);
        case Data::ScalarInt64: return _get_as<T, Data::ScalarInt64>(x);
        case Data::ScalarUInt8: return _get_as<T, Data::ScalarUInt8>(x);
        case Data::ScalarUInt16: return _get_as<T, Data::ScalarUInt16>(x);
        case Data::ScalarUInt32: return _get_as<T, Data::ScalarUInt32>(x);
        case Data::ScalarUInt64: return _get_as<T, Data::ScalarUInt64>(x);
        case Data::ScalarBool: return _get_as<T, Data::ScalarBool>(x);
    }
}

// Define constructors for wrap for the scalar types
// In c++20, I think we could do this with a concept.
// I'm struggling to get SFINAE working.  I'm not sure if it is possible here, so I'm using macros.  Sorry.
// Construct a wrap from a scalar type
#define WrapConstruct(type) \
    template <> wrap::wrap<type>(const type& x);
// Construct a wrap from a scalar
#define WrapConstructPmt(type) \
    template <> wrap::wrap<scalar<type>>(const scalar<type>& x);

#define Equals(type) \
    template <> bool operator==<type>(const wrap& x, const type& other);

#define EqualsPmt(type) \
    template <> bool operator==<scalar<type>>(const wrap& x, const scalar<type>& other);

#define Apply(func) \
func(uint8_t) \
func(uint16_t) \
func(uint32_t) \
func(uint64_t) \
func(int8_t) \
func(int16_t) \
func(int32_t) \
func(int64_t) \
func(bool) \
func(float) \
func(double) \
func(std::complex<float>) \
func(std::complex<double>)

Apply(WrapConstruct)
Apply(WrapConstructPmt)
Apply(Equals)
Apply(EqualsPmt)

#undef WrapConstruct
#undef WrapConstantPmt
#undef Equals
#undef EqualsPmt
#undef Apply

#define IMPLEMENT_PMT_SCALAR(datatype, fbtype)                      \
    template <>                                                     \
    datatype scalar_value<datatype>::value()                          \
    {                                                               \
        auto pmt = GetSizePrefixedPmt(_fbb.GetBufferPointer());     \
        return pmt->data_as_Scalar##fbtype()->value();              \
    }                                                               \
                                                                    \
    template <>                                                     \
    const datatype scalar_value<datatype>::value() const              \
    {                                                               \
        auto pmt = GetSizePrefixedPmt(_fbb.GetBufferPointer());     \
        return pmt->data_as_Scalar##fbtype()->value();              \
    }                                                               \
                                                                    \
    template <>                                                     \
    flatbuffers::Offset<void> scalar_value<datatype>::rebuild_data(   \
        flatbuffers::FlatBufferBuilder& fbb)                        \
    {                                                               \
        Scalar##fbtype##Builder sb(fbb);                            \
        auto val = value();                                         \
        sb.add_value(val);                                          \
        return sb.Finish().Union();                                 \
    }                                                               \
                                                                    \
    template <>                                                     \
    void scalar_value<datatype>::set_value(const datatype& val)       \
    {                                                               \
        Scalar##fbtype##Builder sb(_fbb);                           \
        sb.add_value(val);                                          \
        _data = sb.Finish().Union();                                \
        build();                                                    \
    }                                                               \
                                                                    \
    template <>                                                     \
    scalar_value<datatype>::scalar_value(const datatype& val)           \
        : base(Data::Scalar##fbtype)                            \
    {                                                               \
        set_value(val);                                             \
    }                                                               \
                                                                    \
    template <>                                                     \
    scalar_value<datatype>::scalar_value(const uint8_t* buf)            \
        : base(Data::Scalar##fbtype)                            \
    {                                                               \
        auto data = GetPmt(buf)->data_as_Scalar##fbtype()->value(); \
        set_value(data);                                            \
    }                                                               \
                                                                    \
    template <>                                                     \
    scalar_value<datatype>::scalar_value(const pmtf::Pmt* fb_pmt)       \
        : base(Data::Scalar##fbtype)                            \
    {                                                               \
        auto data = fb_pmt->data_as_Scalar##fbtype()->value();      \
        set_value(data);                                            \
    }                                                               \
                                                                    \
    template class scalar_value<datatype>;


#define IMPLEMENT_PMT_SCALAR_CPLX(datatype, fbtype)                    \
    template <>                                                        \
    datatype scalar_value<datatype>::value()                             \
    {                                                                  \
        auto pmt = GetSizePrefixedPmt(_fbb.GetBufferPointer());        \
        return *((datatype*)(pmt->data_as_Scalar##fbtype()->value())); \
    }                                                                  \
                                                                       \
    template <>                                                        \
    const datatype scalar_value<datatype>::value() const                 \
    {                                                                  \
        auto pmt = GetSizePrefixedPmt(_fbb.GetBufferPointer());        \
        return *((datatype*)(pmt->data_as_Scalar##fbtype()->value())); \
    }                                                                  \
                                                                       \
    template <>                                                        \
    flatbuffers::Offset<void> scalar_value<datatype>::rebuild_data(      \
        flatbuffers::FlatBufferBuilder& fbb)                           \
    {                                                                  \
        Scalar##fbtype##Builder sb(fbb);                               \
        auto val = value();                                            \
        sb.add_value((fbtype*)&val);                                   \
        return sb.Finish().Union();                                    \
    }                                                                  \
                                                                       \
    template <>                                                        \
    void scalar_value<datatype>::set_value(const datatype& val)          \
    {                                                                  \
        Scalar##fbtype##Builder sb(_fbb);                              \
        sb.add_value((fbtype*)&val);                                   \
        _data = sb.Finish().Union();                                   \
        build();                                                       \
    }                                                                  \
                                                                       \
    template <>                                                        \
    scalar_value<datatype>::scalar_value(const datatype& val)              \
        : base(Data::Scalar##fbtype)                               \
    {                                                                  \
        set_value(val);                                                \
    }                                                                  \
                                                                       \
    template <>                                                        \
    scalar_value<datatype>::scalar_value(const uint8_t* buf)               \
        : base(Data::Scalar##fbtype)                               \
    {                                                                  \
        auto data = GetPmt(buf)->data_as_Scalar##fbtype()->value();    \
        set_value(*((const datatype*)data));                           \
    }                                                                  \
                                                                       \
    template <>                                                        \
    scalar_value<datatype>::scalar_value(const pmtf::Pmt* fb_pmt)          \
        : base(Data::Scalar##fbtype)                               \
    {                                                                  \
        auto data = fb_pmt->data_as_Scalar##fbtype()->value();         \
        set_value(*((const datatype*)data));                           \
    }                                                                  \
                                                                       \
                                                                       \
    template class scalar_value<datatype>;*/

} // namespace pmtf
