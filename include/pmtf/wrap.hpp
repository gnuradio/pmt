/*-*-c++-*-*/
/*
 * Copyright 2021 John Sallay
 *
 * SPDX-License-Identifier: LGPL-3.0
 *
 */
#pragma once

#include <pmtf/base.hpp>
//#include <pmtf/vector.hpp>
//#include <pmtf/scalar.hpp>

namespace pmtf {

/**
 * @brief Class to hold any kind of pmt object.
 * 
 * It really holds a pointer to a pmt object.  It has convenience functions to
 * make it easier to interact with the pmt.
*/
class wrap {
  // Generic class to wrap a pmt.
  // Should accept the following types in the constructor:
  // The constructors are going to look a little weird, but it is important
  // that they are structured the way that they are.
  //
  // If we write a "normal" constructor, then we end up with circular references.
  // A map has wrap keys, but a map can also be a value.
  // We are going to use templates to avoid this problem.  We will define generic
  // constructors here, and then specialize them in the various hpp files.  That
  // means that we don't have to define everything here (or know about everything
  // here)
  public:
    /**
     * @ brief Construct an empty wrap.
     *
     * Note that is has a nullptr, not a null pmt.  Don't try to access it.
     */
    wrap() : d_ptr(nullptr) {}
    /**
     * @ brief declare template constructor.
     * 
     * Do not define the constructor.  It will cause a compiler error unless the
     * type T is specialized in a different header file.
     */
    template <class T>
    wrap(const T& value);
    /**
     * @ brief Construct a wrap from a std::vector.
     *
     * Copy an std::vector into a vector.
     * When we upgrade to c++20, allow for span.  That way it can be more types.
     */
    /*template <class T, class alloc>
    wrap(const std::vector<T, alloc>& x) {
        auto value = vector(x);
        d_ptr = value.ptr();
    }*/
    
    /**
     * @ brief Construct a wrap from a "scalar" value.
     *
     * A scalar is any type defined in scalar.hpp.  (e.g. float)
     * Note that this is a catch all, and it will fail if, for example a std::deque
     * is passed in.
     * When we upgrade to c++20, use a concept to limit this constructor.
     */
    /*template <class T>
    wrap(const T& x) {
        auto value = scalar(x);
        d_ptr = value.ptr();  
    };*/
    wrap(base::sptr x): d_ptr(x) {}
    //template <class T>
    //wrap(const std::map<std::string, T>& x);
    operator typename base::sptr() const { return d_ptr; }
    typename base::sptr ptr() const { return d_ptr; }
  private:
        base::sptr d_ptr;
};

// This needs to be specialized in each of the other header files.
template <class T>
bool operator==(const wrap& x, const T& other);

template <class T>
bool operator!=(const wrap& x, const T& other) {
    return !operator==(x, other);
}

/*map<std::string> get_map(const wrap& x) {
    if (x.ptr()->data_type() == Data::PmtMap) {
        return map<std::string>(std::dynamic_pointer_cast<map<std::string>>(x.ptr()));
    else
        throw std::runtime_erro("Cannot convert to map");
}




template <class T>
bool operator==(const wrap& x, const T& other) {
    if (can_be<T>(x)) {
        auto value = get_scalar<T>(x);
        return x == other;
    } else
        return false;
}

template <class T>
bool operator!=(const wrap& x, const T& other) {
    return !operator==(x, other);
}*/

//bool operator==(const wrap& x, const wrap& other) {
//    return false;
    //throw std::runtime_error("Not implemented Yet"); 
//}

/*template <class T>
vector<T> get_vector(const wrap& x) {
    if (x.ptr()->is_vector())
        return vector<T>(std::dynamic_pointer_cast<pmt_vector_value<T>>(x.ptr()));
    else
        throw std::runtime_error("Cannot cast pmt to vector<T>");
}*/

std::ostream& operator<<(std::ostream& os, const wrap& x);

/*template <class U>
bool operator==(const wrap& x1, const U& x2) {
    // We need to try and make the type match up.
    if constexpr(std::is_arithmetic_v<U>)
        if (x.ptr()->is_scalar())
            return get_scalar<U>
    return x1.ptr() == x2;
}*/
/*template <class U>
bool operator!=(const wrap& x1, const U& x2) {
    return !(x1 == x2);
}*/
}
//#include <pmtf/map.hpp>

/* How to handle a generic container?
1) Have it hold a pmt ptr.  It has to dynamically ask anything it wants to know.
    }
}

std::ostream& operator<<(std::ostream& os, const wrap& x);

How to handle a generic container?
1) Have it hold a pmt ptr.  It has to dynamically ask anything it wants to know.
2) We could do a hybrid vector class that takes in any pmt_vector_value.  Then I can cast a
  pmt to a vector.  What would a vector iterator do?  You would still have to know the 
  type to do anything useful.
  

*/




