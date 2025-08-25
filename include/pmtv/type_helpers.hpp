#pragma once

#include <cassert>
#include <complex>
#include <concepts>
#include <map>
#include <memory>
#include <memory_resource>
#include <ranges>
#include <span>
#include <variant>
#include <vector>

#include <pmtv/rva_variant.hpp>

#if __has_include(<mdspan>)
  #include <mdspan>
  #define TENSOR_HAVE_MDSPAN 1
#endif

namespace pmtv {

struct tensor_extents_tag {};
struct tensor_data_tag {};

inline constexpr tensor_extents_tag extents_from{};
inline constexpr tensor_data_tag data_from{};

template <class T_>
struct Tensor {
    using T = std::conditional_t<std::same_as<T_, bool>, uint8_t, T_>;
    using value_type = T;

    std::pmr::vector<std::size_t> _extents;
    std::pmr::vector<T> _data;

    static constexpr std::size_t product(std::span<const std::size_t> ex) {
        std::size_t n{1UZ};
        for (auto e : ex) {
            if (e != 0UZ && n > (std::numeric_limits<std::size_t>::max)() / e)
                throw std::length_error("Tensor: extents product overflow");
            n *= e;
        }
        return n;
    }
    static constexpr std::size_t checked_size(std::span<const std::size_t> ex) {
        return product(ex);
    }

    constexpr void bounds_check(std::span<const std::size_t> indices) const {
        if (indices.size() != rank())
            throw std::out_of_range("Tensor::at: incorrect number of indices");
        for (std::size_t d = 0; d < rank(); ++d)
            if (indices[d] >= _extents[d])
                throw std::out_of_range("Tensor::at: index out of bounds");
    }

    // row-major fold from a span
    [[nodiscard]] constexpr std::size_t index_of(std::span<const std::size_t> idx) const noexcept {
        std::size_t lin = 0UZ;
        for (std::size_t d = 0; d < idx.size(); ++d) {
            lin = lin * _extents[d] + idx[d];
        }
        return lin;
    }

    // row-major fold (variadic)
    template <std::integral... Indices>
    [[nodiscard]] constexpr std::size_t index_of(Indices... indices) const noexcept {
        std::array<std::size_t, sizeof...(Indices)> a{ static_cast<std::size_t>(indices)... };
        assert(a.size() == rank());
        return index_of(std::span<const std::size_t>(a));
    }

    Tensor(const Tensor& other, std::pmr::memory_resource* mr = std::pmr::get_default_resource())
    : _extents(other._extents.begin(), other._extents.end(), mr),
      _data(other._data.begin(), other._data.end(), mr) {}

    Tensor(Tensor&& other) noexcept = default;

    Tensor& operator=(const Tensor& other) {
        if (this != &other) {
            _extents = other._extents;
            _data = other._data;
        }
        return *this;
    }
    Tensor& operator=(Tensor&& other) noexcept = default;

    explicit Tensor(std::pmr::memory_resource* mr = std::pmr::get_default_resource())
        : _extents(mr), _data(mr) {}

    template <std::ranges::range Extents>
        requires std::same_as<std::ranges::range_value_t<Extents>, std::size_t>
    explicit Tensor(const Extents& extents,
           std::pmr::memory_resource* mr = std::pmr::get_default_resource())
        : _extents(extents.begin(), extents.end(), mr),
          _data(checked_size(_extents), mr) {}

    Tensor(std::initializer_list<std::size_t> extents,
           std::pmr::memory_resource* mr = std::pmr::get_default_resource())
        : _extents(extents.begin(), extents.end(), mr),
          _data(checked_size(_extents), mr) {}

    template <std::ranges::range Extents, std::ranges::range Data>
    requires (std::same_as<std::ranges::range_value_t<Extents>, std::size_t>
        && std::same_as<std::ranges::range_value_t<Data>, T>)
    Tensor(const Extents& extents, const Data& data, std::pmr::memory_resource* mr = std::pmr::get_default_resource())
        : _extents(extents.begin(), extents.end(), mr), _data(data.begin(), data.end(), mr) {
        if (_data.size() != checked_size(_extents)) {
            throw std::runtime_error("Tensor: data size doesn't match extents product.");
        }
    }

    template<std::ranges::range Range>
    requires std::same_as<std::ranges::range_value_t<Range>, std::size_t>
    Tensor(tensor_extents_tag, const Range& extents, std::pmr::memory_resource* mr = std::pmr::get_default_resource())
    : _extents(extents.begin(), extents.end(), mr), _data(checked_size(_extents), mr) {}

    template<std::ranges::range Range>
    requires std::same_as<std::ranges::range_value_t<Range>, T>
    Tensor(tensor_data_tag, const Range& data, std::pmr::memory_resource* mr = std::pmr::get_default_resource())
        : _extents({std::size_t(std::ranges::size(data))}, mr), _data(data.begin(), data.end(), mr) {}

    Tensor(std::size_t count, const T& value, std::pmr::memory_resource* mr = std::pmr::get_default_resource())
        : _extents({count}, mr), _data(count, value, mr) {}

    template<std::input_iterator InputIt>
    Tensor(InputIt first, InputIt last, std::pmr::memory_resource* mr = std::pmr::get_default_resource())
        : _extents(mr), _data(first, last, mr) { _extents.push_back(_data.size()); }

    // Usage examples:
    /**
    std::vector<std::size_t> v{10, 20, 30};
    Tensor<std::size_t> t1(extents_from, v);  // Clear: extents {10,20,30}
    Tensor<std::size_t> t2(data_from, v);     // Clear: 1D data {10,20,30}
    */


    /**===== std::vector/std::pmr::vector COMPATIBILITY ===== */

    template <std::ranges::range Data>
    requires std::same_as<std::ranges::range_value_t<Data>, T>
    Tensor(std::initializer_list<std::size_t> extents, const Data& data,
        std::pmr::memory_resource* mr = std::pmr::get_default_resource())
    : _extents(extents.begin(), extents.end(), mr), _data(data.begin(), data.end(), mr) {
        if (_data.size() != checked_size(_extents)) {
            throw std::runtime_error("Tensor: data size doesn't match extents product.");
        }
    }

    template<class U, class Allocator>
    requires std::same_as<U, T>
    explicit Tensor(const std::vector<U, Allocator>& vec, std::pmr::memory_resource* mr = std::pmr::get_default_resource())
        : _extents({vec.size()}, mr), _data(vec.begin(), vec.end(), mr) {}

    template<class U>
    requires std::same_as<U, T>
    explicit Tensor(const std::pmr::vector<U>& vec, std::pmr::memory_resource* mr = std::pmr::get_default_resource())
        : _extents({vec.size()}, mr), _data(vec.begin(), vec.end(), mr) {}

    template<class U>
    requires std::same_as<U, T>
    explicit Tensor(std::pmr::vector<U>&& vec, std::pmr::memory_resource* mr = std::pmr::get_default_resource())
        : _extents({vec.size()}, mr), _data(std::move(vec)) {
        if (_data.get_allocator().resource() != mr) {
            _data = std::pmr::vector<T>(std::make_move_iterator(_data.begin()), std::make_move_iterator(_data.end()), mr);
        }
    }

    template<class U, class Allocator>
    requires std::same_as<U, T>
    Tensor& operator=(const std::vector<U, Allocator>& vec) {
        _extents = {vec.size()};
        _data.assign(vec.begin(), vec.end());
        return *this;
    }

    template<class U>
    requires std::same_as<U, T>
    Tensor& operator=(const std::pmr::vector<U>& vec) {
        _extents = {vec.size()};
        _data.assign(vec.begin(), vec.end());
        return *this;
    }

    explicit constexpr operator std::vector<T>() const {
        if (rank() != 1) {
            throw std::runtime_error("Can only convert 1D tensors to std::vector");
        }
        return std::vector<T>(_data.begin(), _data.end());
    }

    explicit constexpr operator std::pmr::vector<T>() const {
        if (rank() != 1) {
            throw std::runtime_error("Can only convert 1D tensors to std::pmr::vector");
        }
        return std::pmr::vector<T>(_data.begin(), _data.end(), _data.get_allocator().resource());
    }

    // 1D vector compatibility functions
    [[nodiscard]] std::size_t capacity() const noexcept { return _data.capacity(); }
    void reserve(std::size_t new_cap) { _data.reserve(new_cap); }
    void shrink_to_fit() { _data.shrink_to_fit();  }
    void clear() noexcept {
        _extents.clear();
        _data.clear();
    }

    // Resize specific dimension
    void resize_dim(std::size_t dim, std::size_t new_extent) {
        if (dim >= rank()) {
            throw std::out_of_range("resize_dim: dimension out of range");
        }

        if (_extents[dim] == new_extent) {
            return;
        }

        _extents[dim] = new_extent;
        std::size_t new_total_size = product(_extents);

        _data.resize(new_total_size); // May need more sophisticated copying for interior dimensions
        // this is a simplified version - the full implementation would need element-wise padding/truncating
    }

    void resize(std::initializer_list<std::size_t> new_extents, const T& value = {}) {
        resize(std::span(new_extents), value);
    }

    template<std::ranges::range Range>
    requires std::same_as<std::ranges::range_value_t<Range>, std::size_t>
    void resize(const Range& new_extents, const T& value = {}) {
        if (new_extents.empty()) { // clear tensor
            clear();
            return;
        }

        std::size_t new_size = product(new_extents);
        _extents.assign(new_extents.begin(), new_extents.end());
        _data.assign(new_size, value);
    }

    [[nodiscard]] T& front() {
        if (empty()) throw std::runtime_error("front() on empty tensor");
        return _data.front();
    }

    [[nodiscard]] const T& front() const {
        if (empty()) throw std::runtime_error("front() on empty tensor");
        return _data.front();
    }

    [[nodiscard]] T& back() {
        if (empty()) throw std::runtime_error("back() on empty tensor");
        return _data.back();
    }

    [[nodiscard]] const T& back() const {
        if (empty()) throw std::runtime_error("back() on empty tensor");
        return _data.back();
    }

    void push_back(const T& value) {
        if (rank() > 1) {
            _extents = {size()};  // Flatten to 1D
        } else if (rank() == 0) {
            _extents = {0};
        }
        _data.push_back(value);
        ++_extents[0];
    }

    void push_back(T&& value) {
        if (rank() > 1) {
            _extents = {size()};
        } else if (rank() == 0) {
            _extents = {0};
        }
        _data.push_back(std::move(value));
        ++_extents[0];
    }

    template<class... Args>
    T& emplace_back(Args&&... args) {
        if (rank() > 1) {
            _extents = {size()};
        } else if (rank() == 0) {
            _extents = {0};
        }
        auto& ref = _data.emplace_back(std::forward<Args>(args)...);
        ++_extents[0];
        return ref;
    }

    void pop_back() {
        if (empty()) throw std::runtime_error("pop_back on empty tensor");

        if (rank() <= 1) {
            _data.pop_back();
            if (rank() == 1) {
                --_extents[0];
                if (_extents[0] == 0) {
                    _extents.clear();
                }
            }
        } else {
            _extents = {size()};
            _data.pop_back();
            --_extents[0];
        }
    }

    // Assignment operations
    template<std::ranges::range Range>
        requires std::same_as<std::ranges::range_value_t<Range>, T>
    Tensor& assign(const Range& range) {
        _extents = {static_cast<std::size_t>(std::ranges::size(range))};
        _data.assign(range.begin(), range.end());
        return *this;
    }

    Tensor& operator=(std::initializer_list<T> initializer_list) {
        assign(initializer_list);
        return *this;
    }

    void assign(std::size_t count, const T& value) {
        _extents = {count};
        _data.assign(count, value);
    }

    void swap(Tensor& other) noexcept {
        _extents.swap(other._extents);
        _data.swap(other._data);
    }

    // --- basic props ---
    [[nodiscard]] constexpr std::size_t rank()   const noexcept { return _extents.size(); }
    [[nodiscard]] constexpr std::size_t size()   const noexcept { return _data.size(); }
    [[nodiscard]] constexpr bool        empty()  const noexcept { return _data.empty(); }
    [[nodiscard]] constexpr std::size_t extent(std::size_t d) const noexcept { return _extents[d]; }
    [[nodiscard]] constexpr std::span<const std::size_t> extentsSpan() const noexcept { return _extents; }

    // --- iterators / STL compat ---
    [[nodiscard]] T*       data()       noexcept { return _data.data(); }
    [[nodiscard]] const T* data() const noexcept { return _data.data(); }
    auto begin() noexcept { return _data.begin(); }
    auto end()   noexcept { return _data.end(); }
    auto begin() const noexcept { return _data.begin(); }
    auto end()   const noexcept { return _data.end(); }
    auto cbegin() const noexcept { return _data.cbegin(); }
    auto cend()   const noexcept { return _data.cend(); }

    [[nodiscard]] constexpr std::span<const size_t> extents() const noexcept { return std::span(_extents); }
    [[nodiscard]] constexpr std::span<T> data_span() noexcept {return std::span(_data); }
    [[nodiscard]] constexpr std::span<const T> data_span() const noexcept {return std::span(_data); }

    // for vector-like compatibility
    [[nodiscard]] T& operator[](std::size_t idx) noexcept {
        assert(rank() == 1);
        return _data[idx];
    }
    constexpr const T& operator[](std::size_t idx) const noexcept {
        assert(rank() == 1);
        return _data[idx];
    }
    // Templated multi-bracket operator - unchecked (similar to vector '[..]' to 'at(..)'
    template <std::integral... Indices>
    [[nodiscard]] T& operator[](Indices... idx)       noexcept { return _data[index_of(idx...)]; }
    template <std::integral... Indices>
    constexpr const T& operator[](Indices... idx) const noexcept { return _data[index_of(idx...)]; }

    // Checked access (throws std::out_of_range)
    [[nodiscard]] T& at(std::span<const std::size_t> indices) {
        bounds_check(indices);
        return _data[index_of(indices)];
    }
    [[nodiscard]] const T& at(std::span<const std::size_t> indices) const {
        bounds_check(indices);
        return _data[index_of(indices)];
    }

    template <std::integral... Indices>
    [[nodiscard]] T& at(Indices... indices) {
        std::array<std::size_t, sizeof...(Indices)> a{ static_cast<std::size_t>(indices)... };
        bounds_check(a);
        return at(std::span<const std::size_t>(a));
    }

    template <std::integral... Indices>
    [[nodiscard]] const T& at(Indices... indices) const {
        std::array<std::size_t, sizeof...(Indices)> a{ static_cast<std::size_t>(indices)... };
        bounds_check(a);
        return at(std::span<const std::size_t>(a));
    }

    constexpr auto operator<=>(const Tensor<T>& other) const noexcept {
        if (auto cmp = _extents <=> other._extents; cmp != 0) return cmp;
        return _data <=> other._data;
    }

    constexpr bool operator==(const Tensor<T>& other) const noexcept {
        return _extents == other._extents && _data == other._data;
    }

    template<class U, class Allocator>
    requires std::same_as<U, T>
    constexpr bool operator==(const std::vector<U, Allocator>& vec) const noexcept {
        return rank() == 1 && _extents[0] == vec.size() && std::ranges::equal(_data, vec);
    }

    template<class U, class Allocator>
    requires std::same_as<U, T>
    constexpr friend bool operator==(const std::vector<U, Allocator>& vec, const Tensor<T>& tensor) noexcept {
        return tensor == vec;
    }

    template<class U>
    requires std::same_as<U, T>
    bool operator==(const std::pmr::vector<U>& vec) const noexcept {
        return rank() == 1 && _extents[0] == vec.size() && std::ranges::equal(_data, vec);
    }

    template<class U>
    requires std::same_as<U, T>
    friend bool operator==(const std::pmr::vector<U>& vec, const Tensor<T>& tensor) noexcept {
        return tensor == vec;
    }

    void fill(const T& v) { std::ranges::fill(_data, v); }



    void reshape(std::initializer_list<std::size_t> newExtents) { reshape(std::span(newExtents)); }

    template<std::ranges::range Range>
    requires std::same_as<std::ranges::range_value_t<Range>, std::size_t>
    void reshape(const Range& newExtents) { // reshape without moving data; preserves total size
        const std::size_t newN = product(newExtents);
        if (newN != size()) throw std::runtime_error("Tensor::reshape: size mismatch");
        _extents.assign(newExtents.begin(), newExtents.end());
    }

    // --- strides (row-major) ---
    [[nodiscard]] std::pmr::vector<std::size_t> strides() const {
        std::pmr::vector<std::size_t> s(_extents.get_allocator().resource());
        s.resize(rank());
        if (rank() == 0) return s;
        std::size_t stride = 1;
        for (std::size_t i = rank(); i-- > 0;) {
            s[i] = stride;
            stride *= _extents[i];
        }
        return s;
    }

#if defined(TENSOR_HAVE_MDSPAN)
    auto view() noexcept {
        using index_t = std::size_t;
        auto e = _extents; // copy to ensure a contiguous buffer for constructor
        // Use layout_right (row-major)
        return std::mdspan<T, std::dextents<index_t, std::dynamic_extent>>(data(), e);
    }
    auto view() const noexcept {
        using index_t = std::size_t;
        auto e = _extents;
        return std::mdspan<const T, std::dextents<index_t, std::dynamic_extent>>(data(), e);
    }
#else
    template<bool is_const>
    struct View {
        using TPtr = std::conditional_t<is_const, const T*, T*>;

    private:
        TPtr ptr_;
        std::span<const std::size_t> extents_;
        std::pmr::vector<std::size_t> strides_;

    public:
        View(TPtr ptr, std::span<const std::size_t> ex, std::pmr::vector<std::size_t> st)
            : ptr_(ptr), extents_(ex), strides_(std::move(st)) {}

        TPtr data() const noexcept { return ptr_; }
        auto extents() const noexcept { return extents_; }
        auto strides() const noexcept { return std::span<const std::size_t>(strides_); }
    };

    [[nodiscard]] auto view() noexcept {
        return View<false>{ data(), extentsSpan(), strides() };
    }
    [[nodiscard]] auto view() const noexcept {
        return View<true>{ data(), extentsSpan(), strides() };
    }
#endif

    // missing: subspan -- intentional for the time being

    template<std::ranges::range Range>
    requires (std::same_as<std::ranges::range_value_t<Range>, T> && !std::same_as<Range, std::initializer_list<T>>)
    Tensor& operator=(const Range& range) {
        if (std::ranges::size(range) != size()) {
            throw std::runtime_error("Range size doesn't match tensor size for assignment");
        }
        std::ranges::copy(range, _data.begin());
        return *this;
    }

    Tensor& operator=(const T& value) {
        std::ranges::fill(_data, value);
        return *this;
    }
};

// ---- CTAD guides ----
template <std::ranges::range Extents, std::ranges::range Data>
Tensor(const Extents&, const Data&, std::pmr::memory_resource* = std::pmr::get_default_resource())
    -> Tensor<std::ranges::range_value_t<Data>>;

template<std::ranges::range Range>
Tensor(tensor_data_tag, const Range&, std::pmr::memory_resource* = std::pmr::get_default_resource())
    -> Tensor<std::ranges::range_value_t<Range>>;

template<std::ranges::range Range>
Tensor(tensor_extents_tag, const Range&, std::pmr::memory_resource* = std::pmr::get_default_resource())
    -> Tensor<std::ranges::range_value_t<Range>>;

template<class T>
Tensor(std::size_t, const T&, std::pmr::memory_resource* = std::pmr::get_default_resource()) -> Tensor<T>;

template<std::input_iterator InputIt>
Tensor(InputIt, InputIt, std::pmr::memory_resource* = std::pmr::get_default_resource())
    -> Tensor<typename std::iterator_traits<InputIt>::value_type>;

template<class T, class Allocator>
Tensor(const std::vector<T, Allocator>&, std::pmr::memory_resource* = std::pmr::get_default_resource()) -> Tensor<T>;

template<class T>
Tensor(const std::pmr::vector<T>&, std::pmr::memory_resource* = std::pmr::get_default_resource()) -> Tensor<T>;

template<class T>
Tensor(std::pmr::vector<T>&&, std::pmr::memory_resource* = std::pmr::get_default_resource()) -> Tensor<T>;

template<class T>
void swap(Tensor<T>& lhs, Tensor<T>& rhs) noexcept {
    lhs.swap(rhs);
}

template <class T>
struct is_pmt_tensor : std::false_type {};

template <class T>
struct is_pmt_tensor<Tensor<T>> : std::true_type {};

template <class T>
concept PmtTensor = is_pmt_tensor<std::remove_cvref_t<T>>::value;


namespace detail {

// Convert a list of types to the full set used for the pmt.
template<template<typename... > class VariantType, typename... Args>
struct as_pmt {
    using type = VariantType<std::monostate,
                             bool,
                             Args...,
                             Tensor<Args>...,
                             std::string,
                             std::vector<std::string>,
                             std::vector<rva::self_t>,
                             std::map<std::string, rva::self_t, std::less<>>
                             >;
};

template<template<typename... > class TemplateType, typename ...T>
struct as_pmt<TemplateType, std::tuple<T...>> {
    using type = typename as_pmt<TemplateType, T...>::type;
};
}


template<template<typename... > class VariantType, class... Args>
using as_pmt_t = typename detail::as_pmt<VariantType, Args...>::type;

// Check if `std::size_t` has the same type as `uint16_t` or `uint32_t` or `uint64_t`.
// If it has the same type, then there is no need to add `std::size_t` the supported types.
// Otherwise, `std::size_t` is added to the supported types.
// This can happen if one builds using Emscripten where `std::size_t` is defined as `unsigned long` and
// `uint32_t` and `uint64_t` are defined as `unsigned int` and `unsigned long long`, respectively.
static constexpr bool support_size_t = !std::is_same_v<std::size_t, uint16_t> && !std::is_same_v<std::size_t, uint32_t> && !std::is_same_v<std::size_t, uint64_t>;

// Note that per the spec, std::complex is undefined for any type other than float, double, or long_double
using default_supported_types_without_size_t = std::tuple<//bool,
        uint8_t, uint16_t, uint32_t, uint64_t,
        int8_t, int16_t, int32_t, int64_t,
        float, double, std::complex<float>, std::complex<double>>;

// Add std::size_t to default_supported_types_without_size_t
using default_supported_types_with_size_t = decltype(std::tuple_cat(std::declval<default_supported_types_without_size_t>(), std::declval<std::tuple<std::size_t>>()));

using default_supported_types = typename std::conditional_t<support_size_t, default_supported_types_with_size_t, default_supported_types_without_size_t>;

// initialisation via type list stored in tuple (N.B. tuple could be extended by user with custom OOT types)
using pmt_var_t = as_pmt_t<rva::variant, default_supported_types>;

using pmt_null = std::monostate;


template <typename T>
concept PmtNull = std::is_same_v<T, std::monostate>;

template <typename T>
concept Complex =
    std::is_same_v<T, std::complex<float>> || std::is_same_v<T, std::complex<double>>;

template <typename T>
concept Scalar = std::same_as<T, bool> || std::integral<T> || std::floating_point<T> || Complex<T>;

template <typename T>
concept UniformVector =
    std::ranges::contiguous_range<T> && Scalar<typename T::value_type>;

// A vector of bool can be optimized to one bit per element, so it doesn't satisfy UniformVector
template <typename T>
concept UniformBoolVector =
    std::ranges::range<T> && std::same_as<typename T::value_type, bool>;

template <typename T>
concept UniformStringVector =
    std::ranges::range<T> && std::same_as<typename T::value_type, std::string>;

template <typename T>
concept PmtMap = std::is_same_v<T, std::map<std::string, pmt_var_t, std::less<>>>;

template <typename T>
concept String = std::is_same_v<T, std::string>;

template <typename T>
concept PmtVector =
    std::ranges::range<T> && std::is_same_v<typename T::value_type, pmt_var_t>;

template <typename T>
concept IsTensor = requires { typename T::value_type; } && std::same_as<T, Tensor<typename T::value_type>>;

} // namespace pmtv
