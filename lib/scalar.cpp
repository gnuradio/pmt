/*-*-c++-*-*/
/*
 * Copyright 2021 John Sallay
 *
 * SPDX-License-Identifier: LGPL-3.0
 *
 */
#include <pmtf/scalar.hpp>

namespace pmtf {


IMPLEMENT_PMT_SCALAR(int8_t, Int8)
IMPLEMENT_PMT_SCALAR(uint8_t, UInt8)
IMPLEMENT_PMT_SCALAR(int16_t, Int16)
IMPLEMENT_PMT_SCALAR(uint16_t, UInt16)
IMPLEMENT_PMT_SCALAR(int32_t, Int32)
IMPLEMENT_PMT_SCALAR(uint32_t, UInt32)
IMPLEMENT_PMT_SCALAR(int64_t, Int64)
IMPLEMENT_PMT_SCALAR(uint64_t, UInt64)
IMPLEMENT_PMT_SCALAR(bool, Bool)
IMPLEMENT_PMT_SCALAR(float, Float32)
IMPLEMENT_PMT_SCALAR(double, Float64)

IMPLEMENT_PMT_SCALAR_CPLX(std::complex<float>, Complex64)
IMPLEMENT_PMT_SCALAR_CPLX(std::complex<double>, Complex128)

#define WrapConstructImpl(type) \
    template <> wrap::wrap<type>(const type& x) { d_ptr = scalar(x).ptr(); }
// Construct a wrap from a scalar
#define WrapConstructPmtImpl(type) \
    template <> wrap::wrap<scalar<type>>(const scalar<type>& x) { d_ptr = x.ptr(); }

#define EqualsImpl(type) \
    template <> bool operator==<type>(const wrap& x, const type& other) { \
       if (can_be<type>(x)) {                                           \
            auto value = get_scalar<type>(x);                       \
            return x == other;                                          \
        } else                                                          \
            return false;                                               \
    }

#define EqualsPmtImpl(type) \
    template <> bool operator==<scalar<type>>(const wrap& x, const scalar<type>& other) { \
        return x == other.value();  \
    }

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

Apply(WrapConstructImpl)
Apply(WrapConstructPmtImpl)
Apply(EqualsImpl)
Apply(EqualsPmtImpl)


} // namespace pmtf
