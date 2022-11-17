#pragma once

#include <map>
#include <pmtv/base.hpp>
#include <ranges>
#include <span>

namespace pmtv {

/**
 * @brief map of keys of type string to pmts
 *
 */
class map : public pmt {
public:
  using key_type = std::string;
  using mapped_type = _pmt_storage;
  using value_type = std::pair<const key_type, mapped_type>;
  using reference = value_type &;
  using const_reference = const value_type &;
  using map_type = std::map<key_type, mapped_type>;
  using size_type = size_t;
  using map_sptr = std::shared_ptr<map_type>;

  using iterator = map_type::iterator;
  using const_iterator = map_type::const_iterator;
  // Construct empty map
  map() : pmt(map_type{}) {}
  // Copy from std map
  map(const map_type &other) : pmt(other) {}

  // Copy from std map
  /*map(const std::map<std::string, _pmt_storage> &other) : pmt(map_type{}) {
    for (auto &[k, v] : other) {
        // FIXME - the [] operator seems to be returning 
        // the variant by value, not by reference
        this->operator[](k) = v;
        // auto x = this->operator[](k);
        // x = v.storage();
    }
  }*/

  //     // Copy from pmt
  //     template <class T, typename = IsPmt<T>>
  //     map(const T& other) {
  //         if (other.data_type() != data_type())
  //             throw ConversionError(other, "map");
  //         _map = other;
  //     }
  //     map(std::initializer_list<value_type> il) {
  //         _MakeEmptyMap();
  //         for (auto& [k, v]: il)
  //             this->operator[](k) = v;
  //     }
  //     //template <class T>
  //     //map(std::map<string
  //     ~map() {}

  /**************************************************************************
   * Iterators
   **************************************************************************/
  typename map_type::iterator begin() noexcept { return _get_map()->begin(); }
  typename map_type::const_iterator begin() const noexcept {
    return _get_map()->begin();
  }
  typename map_type::iterator end() noexcept { return _get_map()->end(); }
  typename map_type::const_iterator end() const noexcept {
    return _get_map()->end();
  }

  /**************************************************************************
   * Element Access
   **************************************************************************/
  mapped_type &at(const key_type &key) { return _get_map()->at(key); }
  const mapped_type &at(const key_type &key) const {
    return _get_map()->at(key);
  }
  mapped_type &operator[](const key_type &key) {
    return _get_map()->operator[](key);
  }

  //     size_t size() const { return _get_map()->size(); }
  //     size_t count(const key_type& key) const { return
  //     _get_map()->count(key); }

  //     static constexpr Data data_type() { return
  //     DataTraits<type>::enum_value; } const pmt& get_pmt_buffer() const {
  //     return _map; }

  //     //! Equality Comparisons
  //     // Declared as class members so that we don't do implicit
  //     conversions. template <class U> bool operator==(const U& x) const;
  //     template <class U>
  //     bool operator!=(const U& x) const { return !(operator==(x));}
  //     void pre_serial_update() const {
  //         // It may look odd to declare this function as const when it
  //         modifies
  //         // count.  But count is part of the internal interface, so to the
  //         // user, this is a const function.
  //         std::shared_ptr<base_buffer> scalar = _map._scalar;
  //         scalar->data_as<type>()->mutate_count(_get_map()->size());
  //     }

private:
  map_sptr _get_map() { return std::get<map_sptr>(_value); }
  const map_sptr _get_map() const { return std::get<map_sptr>(_value); }
};

// template <class T, class U>
// using IsNotVectorT = std::enable_if_t<!std::is_same_v<uniform_vector<T>, U>,
// bool>;

// // Reversed case.  This allows for x == y and y == x
// template <class T, class U, typename = IsNotVectorT<T, U> >
// bool operator==(const U& y, const uniform_vector<T>& x) {
//     return x.operator==(y);
// }

// template<>
// bool PmtEqual(const std::vector<T>& arg, const uniform_vector<T>& other) {

// }

template <class T> using IsMap = std::enable_if_t<std::is_same_v<map, T>, bool>;

// Need to have map operator here because it has pmts in it.
template <class T, IsMap<T> = true>
std::ostream &operator<<(std::ostream &os, const T &value) {
  os << "{ ";
  bool first = true;
  for (const auto &[k, v] : value) {
    if (!first)
      os << ", ";
    first = false;
    os << k << ": " << pmt(v);
  }
  os << " }";
  return os;
}

} // namespace pmtv
