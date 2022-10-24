#pragma once

#include <pmtv/base.hpp>
#include <ranges>
#include <span>

namespace pmtv {

template <class T> class uniform_vector :  public pmt {
public:
  uniform_vector() : pmt(std::vector<T>()) {}
  // Constuct with unitialized memory
  uniform_vector(size_t size) : pmt(std::vector<T>(size)) {}
  // Fill Constructor
  explicit uniform_vector(size_t size, const T &set_value)
      : pmt(std::vector<T>(size, set_value)) {}
  // Range Constuctor
  // Need the IsNotInteger to not conflict with the fill constructor
  template <class InputIterator>
  uniform_vector(InputIterator first, InputIterator last)
      : pmt(std::vector<T>(first, last)) {}
  // Copy from vector Constructor
  uniform_vector(const std::vector<T> &value) : pmt(value) {}
  // Copy Constructor
  uniform_vector(const uniform_vector<T> &value) : pmt(value) {}
  uniform_vector(const pmt &value) : pmt(value) {}
  // Initializer list Constructor
  uniform_vector(std::initializer_list<T> il)
      : pmt(std::vector<T>(il.begin(), il.size())) {}

  uniform_vector &operator=(const T &value) { _value = value; }
  T *data() { return std::get<std::shared_ptr<std::vector<T>>>(_value)->data(); }
  const T *data() const {
    return std::shared_ptr<std::vector<T>>(_value).data();
  }
  size_t size() const { 
    return std::get<std::shared_ptr<std::vector<T>>>(_value.base())->size(); 
  }
  // const pmt& get_pmt_buffer() const { return _buf; }
  const typename std::vector<T>::iterator begin() const {
    return std::get<std::shared_ptr<std::vector<T>>>(_value)->begin();
  }
  const typename std::vector<T>::iterator end() const {
    return std::get<std::shared_ptr<std::vector<T>>>(_value)->end();
  }
  // typename std::span<T>::const_iterator begin() const { return
  // std::shared_ptr<std::vector<T>>(_value).begin(); } typename
  // std::span<T>::const_iterator end() const { return
  // std::shared_ptr<std::vector<T>>(_value).end(); }
  T &operator[](size_t n) {
    // operator[] doesn't do bounds checking, use at for that
    // TODO: implement at
    return this->data()[n];
  }
  const T &operator[](size_t n) const { return this->data()[n]; }

    // template <class U>
    // bool operator==(const U& other) const { return _buf ==  other; }
    // template <class U>
    // bool operator!=(const U& other) const { return !operator==(other); }

  /*** Traits ****/
    using span_type = typename std::span<T>;
    using const_span_type = typename std::span<const T>;
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using size_type = size_t;

    using iterator = typename span_type::iterator;
    // using const_iterator = typename span_type::const_iterator;
  
private:
};


template <class T, class U>
using IsNotVectorT = std::enable_if_t<!std::is_same_v<uniform_vector<T>, U>, bool>;

// // Reversed case.  This allows for x == y and y == x
// template <class T, class U, typename = IsNotVectorT<T, U> >
// bool operator==(const U& y, const uniform_vector<T>& x) {
//     return x.operator==(y);
// }

// template<>
// bool PmtEqual(const std::vector<T>& arg, const uniform_vector<T>& other) {


// }

} // namespace pmtv