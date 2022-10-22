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
  typename std::vector<T>::iterator begin() {
    return std::get<std::shared_ptr<std::vector<T>>>(_value)->begin();
  }
  typename std::vector<T>::iterator end() {
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

private:
};

} // namespace pmtv