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

/**
 * @class Tensor
 * @brief A multi-dimensional array container with a flexible compile-time and runtime multi-dimensional extent data storage support
 *
 * @tparam ElementType The type of elements stored (bool is stored as uint8_t internally)
 * @tparam Ex Variable number of extents (use std::dynamic_extent for runtime dimensions)

 * ##BasicUsage Basic Usage Examples:
 * ### fully run-time dynamic Tensor<T>
 * @code
 * Tensor<double> matrix({3, 4}, data);
 * matrix[2, 3] = 42.0;
 * matrix.reshape({2, 6});  // same data, new shape
 * @endcode
 *
 * ### static rank, dynamic extents Tensor<T>
 * @code
 * Tensor<float, std::dynamic_extent, std::dynamic_extent> img({640, 480});
 * img.resize({1920, 1080});  // can change dimensions
 * @endcode
 *
 * ### fully static Tensor<T>
 * @code
 * // all compile-time, zero overhead
 * constexpr Tensor<int, 2, 3> mat{{1, 2, 3}, {4, 5, 6}};
 * constexpr auto elem = mat[1, 2];  // compile-time access
 * static_assert(mat.size() == 6);
 * @endcode
 *
 * ## std::vector compatibility
 * @code
 * std::vector<int> vec{1, 2, 3, 4, 5};
 * Tensor<int> tensor(vec);           // construct from vector
 * tensor = vec;                       // assign from vector
 * auto v2 = static_cast<std::vector<int>>(tensor);  // convert back
 * @endcode
 *
 * ## tagged constructors for disambiguation
 * @code
 * std::vector<size_t> values{10, 20, 30};
 * Tensor<size_t> t1(extents_from, values);  // shape: 10×20×30
 * Tensor<size_t> t2(data_from, values);     // data: {10,20,30}
 * @endcode
 *
 * ## custom polymorphic memory resources support (PMR=
 * @code
 * std::pmr::monotonic_buffer_resource arena(buffer, size);
 * Tensor<double> tensor({1000, 1000}, &arena);  // use arena allocator
 * @endcode
 *
 * ## type and layout conversion
 * @code
 * Tensor<int, 3, 3> static_tensor{...};
 * Tensor<double> dynamic_tensor(static_tensor);  // int→double, static→dynamic
 * @endcode
 *
 * ## mdspan/mdarray compatibility
 * @code
 * auto view = tensor.to_mdspan();  // get mdspan view
 * view(i, j) = value;               // mdspan-style access
 * tensor.stride(0);                 // get stride for dimension
 * @endcode
 *
 * @note row-major (C/C++-style) ordering is used exclusively
 * @note bool tensors store data as uint8_t to avoid vector<bool> issues
 * @note static tensors have compile-time size guarantees and zero overhead
 * @note inspired by @see https://wg21.link/P1684 (mdarray proposal)
 */
template <class ElementType, std::size_t... Ex>
struct Tensor {
    using T = std::conditional_t<std::same_as<ElementType, bool>, uint8_t, ElementType>;
    using value_type = T;

    static constexpr bool        _all_static = sizeof...(Ex) > 0UZ && ((Ex != std::dynamic_extent) && ...);
    static constexpr std::size_t _size_ct    = _all_static ? (Ex * ... * 1UZ) : 0UZ;
    struct no_extents_t {}; // zero-sized tag for a fully static/constexpr case
    using extents_store_t = std::conditional_t<_all_static, no_extents_t,
            std::conditional_t<(sizeof...(Ex) > 0UZ), std::array<std::size_t, sizeof...(Ex)>,
                                                      std::pmr::vector<std::size_t>>>;
    using container_type = std::conditional_t<_all_static, std::array<T, _size_ct>, std::pmr::vector<T>>;
    using pointer = container_type::pointer;
    using const_pointer = container_type::const_pointer;
    using reference = container_type::reference;
    using const_reference = container_type::const_reference;


    [[no_unique_address]] extents_store_t _extents{};
    container_type                        _data{};

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
        {
            throw std::out_of_range("Tensor::at: incorrect number of indices");
        }
        for (std::size_t d = 0; d < rank(); ++d) {
            if (indices[d] >= _extents[d]) {
                throw std::out_of_range("Tensor::at: index out of bounds");
            }
        }
    }

    // row-major fold from a span
    [[nodiscard]] constexpr std::size_t index_of(std::span<const std::size_t> idx) const noexcept {
        std::size_t lin = 0UZ;
        for (std::size_t d = 0UZ; d < idx.size(); ++d) {
            lin = lin * extent(d) + idx[d];
        }
        return lin;
    }

    template <std::integral... Indices>
    [[nodiscard]] constexpr std::size_t index_of(Indices... indices) const noexcept {
        if constexpr (_all_static) {
            static_assert(sizeof...(Indices) == sizeof...(Ex), "Tensor::index_of: incorrect number of indices");
            constexpr std::size_t E[]{Ex...};
            std::size_t idx_array[]{static_cast<std::size_t>(indices)...};
            std::size_t lin = 0UZ;
            for (std::size_t d = 0UZ; d < sizeof...(Ex); ++d) {
                lin = lin * E[d] + idx_array[d];
            }
            return lin;
        } else {
            std::array<std::size_t, sizeof...(Indices)> a{static_cast<std::size_t>(indices)...};
            return index_of(std::span<const std::size_t>(a));
        }
    }

    constexpr Tensor() requires (_all_static) = default;

    Tensor(const Tensor& other, std::pmr::memory_resource* mr = std::pmr::get_default_resource()) requires (!_all_static)
        : _extents(mr), _data(other._data.begin(), other._data.end(), mr) {
        if constexpr (std::is_same_v<extents_store_t, std::pmr::vector<std::size_t>>) {
            _extents.assign(other._extents.begin(), other._extents.end());
        } else {
            _extents = other._extents;
        }
    }

    Tensor(const Tensor& other) requires (_all_static) = default;
    Tensor(Tensor&& other) noexcept = default;
    explicit Tensor(std::pmr::memory_resource* mr = std::pmr::get_default_resource()) noexcept requires (!_all_static) : _extents(mr), _data(mr) {}

    template <std::ranges::range Extents>
    requires (std::same_as<std::ranges::range_value_t<Extents>, std::size_t> && !std::is_same_v<std::remove_cvref_t<Extents>, std::initializer_list<std::size_t>>)
    explicit Tensor(const Extents& extents, std::pmr::memory_resource* mr = std::pmr::get_default_resource()) requires (!_all_static)
        : _extents(mr), _data(mr) {
        if constexpr (std::is_same_v<extents_store_t, std::pmr::vector<std::size_t>>) {
            _extents.assign(std::ranges::begin(extents), std::ranges::end(extents));
        } else {
            std::size_t i = 0;
            for (auto e : extents) {
                _extents[i++] = e;
            }
        }
        _data.resize(checked_size(std::span<const std::size_t>(_extents.data(), _extents.size())));
    }

    Tensor(std::initializer_list<std::size_t> extents, std::pmr::memory_resource* mr = std::pmr::get_default_resource()) requires (!_all_static)
        : _data(mr) {  // _extents{} default-initializes (works for both std::array and pmr::vector)
        if constexpr (std::is_same_v<extents_store_t, std::pmr::vector<std::size_t>>) { // fully dynamic rank and extents
            _extents = std::pmr::vector<std::size_t>(extents.begin(), extents.end(), mr);
        } else { // static rank, dynamic extents (using std::array)
            if (extents.size() != sizeof...(Ex)) {
                throw std::runtime_error("Wrong number of extents for static rank tensor");
            }
            std::copy(extents.begin(), extents.end(), _extents.begin());
        }
        _data.resize(checked_size(std::span<const std::size_t>(_extents.data(), _extents.size())));
    }

    template<typename U>
    requires (!_all_static && sizeof...(Ex) == 1 && ((Ex == std::dynamic_extent) && ...) && std::convertible_to<U, T>)
    Tensor(std::initializer_list<U> values, std::pmr::memory_resource* mr = std::pmr::get_default_resource())
        : _extents{values.size()}, _data(values.begin(), values.end(), mr) {}

    template <std::ranges::range Extents, std::ranges::range Data>
    requires (std::same_as<std::ranges::range_value_t<Extents>, std::size_t> && std::same_as<std::ranges::range_value_t<Data>, T>)
    Tensor(const Extents& extents, const Data& data, std::pmr::memory_resource* mr = std::pmr::get_default_resource()) requires (!_all_static)
        : _extents(mr), _data(std::ranges::begin(data), std::ranges::end(data), mr) {
        if constexpr (std::is_same_v<extents_store_t, std::pmr::vector<std::size_t>>) {
            _extents.assign(std::ranges::begin(extents), std::ranges::end(extents));
        } else {
            std::size_t i = 0UZ;
            for (auto e : extents) {
                _extents[i++] = e;
            }
        }
        if (_data.size() != checked_size(std::span<const std::size_t>(_extents.data(), _extents.size()))) {
            throw std::runtime_error("Tensor: data size doesn't match extents product.");
        }
    }

    template<std::ranges::range Range>
    requires std::same_as<std::ranges::range_value_t<Range>, std::size_t>
    Tensor(tensor_extents_tag, const Range& extents, std::pmr::memory_resource* mr = std::pmr::get_default_resource()) requires (!_all_static)
        : _extents(mr), _data(mr) {
        if constexpr (std::is_same_v<extents_store_t, std::pmr::vector<std::size_t>>) {
            _extents.assign(std::ranges::begin(extents), std::ranges::end(extents));
        } else {
            std::size_t i = 0UZ;
            for (auto e : extents) {
                _extents[i++] = e;
            }
        }
        _data.resize(checked_size(std::span<const std::size_t>(_extents.data(), _extents.size())));
    }

    template<std::ranges::range Range>
    requires std::same_as<std::ranges::range_value_t<Range>, T>
    Tensor(tensor_data_tag, const Range& data, std::pmr::memory_resource* mr = std::pmr::get_default_resource()) requires (!_all_static)
        : _extents(mr), _data(std::ranges::begin(data), std::ranges::end(data), mr) { _extents.push_back(std::ranges::size(data)); }

    Tensor(std::size_t count, const T& value, std::pmr::memory_resource* mr = std::pmr::get_default_resource()) requires (!_all_static)
    : _extents(mr), _data(count, value, mr) { _extents.push_back(count); }

    template<std::input_iterator InputIt>
    Tensor(InputIt first, InputIt last, std::pmr::memory_resource* mr = std::pmr::get_default_resource()) requires (!_all_static)
        : _extents(mr), _data(first, last, mr) { _extents.push_back(_data.size()); }

    template <std::ranges::range Data>
    requires std::same_as<std::ranges::range_value_t<Data>, T>
    Tensor(std::initializer_list<std::size_t> extents, const Data& data, std::pmr::memory_resource* mr = std::pmr::get_default_resource()) requires (!_all_static)
        : _extents(mr), _data(std::ranges::begin(data), std::ranges::end(data), mr) {
        if constexpr (std::is_same_v<extents_store_t, std::pmr::vector<std::size_t>>) {
            _extents.assign(extents.begin(), extents.end());
        } else {
            std::copy(extents.begin(), extents.end(), _extents.begin());
        }
        if (_data.size() != checked_size(std::span<const std::size_t>(_extents.data(), _extents.size()))) {
            throw std::runtime_error("Tensor: data size doesn't match extents product.");
        }
    }

    template<class U, class Allocator>
    requires std::same_as<U, T>
    explicit Tensor(const std::vector<U, Allocator>& vec, std::pmr::memory_resource* mr = std::pmr::get_default_resource()) requires (!_all_static)
        : _extents(mr), _data(vec.begin(), vec.end(), mr) { _extents.push_back(vec.size()); }

    template<class U>
    requires std::same_as<U, T>
    explicit Tensor(std::pmr::vector<U>&& vec, std::pmr::memory_resource* mr = std::pmr::get_default_resource()) requires (!_all_static)
        : _extents(mr), _data(mr) {
            std::size_t sz = vec.size();  // Get size BEFORE moving

            if (vec.get_allocator().resource() == mr) { // same allocator - can actually move
                _data = std::move(vec);
            } else { // different allocators - copy data but use move iterators for efficiency
                _data.reserve(sz);
                _data.assign(std::make_move_iterator(vec.begin()), std::make_move_iterator(vec.end()));
            }
        _extents.push_back(sz); // CRITICAL: need to set extents AFTER moving data
    }

    // flat initializer list for static tensors
    template<typename U>
    requires (_all_static && std::convertible_to<U, T>)
    constexpr Tensor(std::initializer_list<U> values) {
        if (values.size() != _size_ct) {
            throw std::runtime_error("Initializer list size doesn't match tensor size");
        }
        std::copy(values.begin(), values.end(), _data.begin());
    }

    // 2D nested initializer list
    template<typename U>
    requires (_all_static && sizeof...(Ex) == 2 && std::convertible_to<U, T>)
    constexpr Tensor(std::initializer_list<std::initializer_list<U>> values) : _data{} {
        constexpr std::array<std::size_t, 2UZ> dims{Ex...};
        if (values.size() != dims[0UZ]) {
            throw std::runtime_error("Wrong number of rows");
        }

        std::size_t idx = 0UZ;
        for (auto row : values) {
            if (row.size() != dims[1UZ]) {
                throw std::runtime_error("Wrong number of columns");
            }
            for (auto val : row) {
                _data[idx++] = val;
            }
        }
    }

    // 3D nested initializer list
    template<typename U>
    requires (_all_static && sizeof...(Ex) == 3 && std::convertible_to<U, T>)
    constexpr Tensor(std::initializer_list<std::initializer_list<std::initializer_list<U>>> values) : _data{} {
        constexpr std::array<std::size_t, 3> dims{Ex...};
        if (values.size() != dims[0]) {
            throw std::runtime_error("Wrong dimension 0 size");
        }

        std::size_t idx = 0;
        for (auto plane : values) {
            if (plane.size() != dims[1]) {
                throw std::runtime_error("Wrong dimension 1 size");
            }
            for (auto row : plane) {
                if (row.size() != dims[2]) {
                    throw std::runtime_error("Wrong dimension 2 size");
                }
                for (auto val : row) {
                    _data[idx++] = val;
                }
            }
        }
    }

    // conversion constructors from static, semi-static, and fully dynamic Tensor<T, ...> types
    template<typename OtherT, std::size_t... OtherEx>
    requires (std::convertible_to<OtherT, T> && (sizeof...(Ex) == 0 || sizeof...(OtherEx) == 0 || sizeof...(Ex) == sizeof...(OtherEx)))
    explicit Tensor(const Tensor<OtherT, OtherEx...>& other, std::pmr::memory_resource* mr = std::pmr::get_default_resource())
        : _extents([mr]() {
            if constexpr (_all_static) {
                return extents_store_t{}; // no_extents_t - no allocator needed
            } else if constexpr (std::is_same_v<extents_store_t, std::pmr::vector<std::size_t>>) {
                return extents_store_t(mr); // PMR vector with correct resource
            } else {
                return extents_store_t{}; // std::array - no allocator needed
            }
        }()),
        _data([mr]() {
            if constexpr (_all_static) {
                return container_type{}; // std::array - no allocator needed
            } else {
                return container_type(mr); // PMR vector with correct resource
            }
        }()) {
            std::vector<std::size_t> src_dims;
            if constexpr (Tensor<OtherT, OtherEx...>::_all_static) {
                constexpr std::array<std::size_t, sizeof...(OtherEx)> static_dims{OtherEx...};
                src_dims.assign(static_dims.begin(), static_dims.end());
            } else {
                src_dims.assign(other._extents.begin(), other._extents.end());
            }

        if constexpr (_all_static) { // static target - validate dimensions and copy data
            constexpr std::array<std::size_t, sizeof...(Ex)> target_dims{Ex...};
            if (src_dims.size() != sizeof...(Ex)) {
                throw std::runtime_error("Rank mismatch in tensor conversion");
            }
            for (std::size_t i = 0; i < sizeof...(Ex); ++i) {
                if (src_dims[i] != target_dims[i]) {
                    throw std::runtime_error("Dimension mismatch in tensor conversion");
                }
            }
            std::copy(other.begin(), other.end(), _data.begin());
        } else { // dynamic target - _extents and _data already have correct allocator from initializer list
            if constexpr (sizeof...(Ex) > 0) {
                // semi-static: fixed rank, dynamic extents
                if (src_dims.size() != sizeof...(Ex)) {
                    throw std::runtime_error("Rank mismatch in tensor conversion");
                }
                std::copy(src_dims.begin(), src_dims.end(), _extents.begin());
            } else { // fully dynamic: assign dimensions (allocator already correct)
                _extents.assign(src_dims.begin(), src_dims.end());
            }

            // copy data (allocator already correct from member init)
            _data.assign(other.begin(), other.end());
        }
    }

    [[nodiscard]] static constexpr Tensor identity() requires (_all_static) {
        constexpr std::size_t N = (Ex * ...);
        Tensor identity = T(0);
        for (std::size_t i = 0UZ; i < N; ++i) {
            identity[i, i] = T{1};
        }
        return identity;
    }

    Tensor& operator=(Tensor&& other) noexcept = default;

    // Replace the existing template assignment operator with this simplified version
    template<typename OtherT, std::size_t... OtherEx>
    Tensor& operator=(const Tensor<OtherT, OtherEx...>& other) {
        // Handle self-assignment for identical types
        if constexpr (std::is_same_v<Tensor, Tensor<OtherT, OtherEx...>>) {
            if (this == reinterpret_cast<const Tensor*>(&other)) {
                return *this;
            }
        }

        // Create temporary using the conversion constructor and swap with it
        if constexpr (_all_static) {
            // Target is static - use default constructor (no allocator needed)
            Tensor temp(other);
            swap(temp);
        } else {
            // Target is dynamic - preserve this tensor's allocator
            Tensor temp(other, _data.get_allocator().resource());
            swap(temp);
        }

        return *this;
    }

    template<class U, class Allocator>
    requires std::same_as<U, T>
    Tensor& operator=(const std::vector<U, Allocator>& vec) {
        if constexpr (!_all_static) {
            _extents.clear();
            _extents.push_back(vec.size());
            _data.assign(vec.begin(), vec.end());
        }
        return *this;
    }

    template<class U>
    requires std::same_as<U, T>
    Tensor& operator=(const std::pmr::vector<U>& vec) {
        if constexpr (!_all_static) {
            _extents.clear();
            _extents.push_back(vec.size());
            _data.assign(vec.begin(), vec.end());
        }
        return *this;
    }

    Tensor& operator=(const Tensor& other) {
        if (this != &other) {
            if constexpr (!_all_static) { // dynamic tensors, need to handle PMR resources properly
                if (_data.get_allocator().resource() == other._data.get_allocator().resource()) {
                    _extents = other._extents;
                    _data = other._data;
                } else { // different memory resources - need to copy
                    _data.assign(other._data.begin(), other._data.end());
                    if constexpr (std::is_same_v<extents_store_t, std::pmr::vector<std::size_t>>) {
                        _extents.assign(other._extents.begin(), other._extents.end());
                    } else {
                        _extents = other._extents;
                    }
                }
            } else { // static tensors, just copy the data
                _data = other._data;
            }
        }
        return *this;
    }

    explicit constexpr operator std::vector<T>() const {
        if (rank() != 1UZ) {
            throw std::runtime_error("Can only convert 1D tensors to std::vector");
        }
        return std::vector<T>(_data.begin(), _data.end());
    }

    explicit constexpr operator std::pmr::vector<T>() const {
        if (rank() != 1UZ) {
            throw std::runtime_error("Can only convert 1D tensors to std::pmr::vector");
        }
        return std::pmr::vector<T>(_data.begin(), _data.end(), _data.get_allocator().resource());
    }

    // 1D vector compatibility functions
    [[nodiscard]] std::size_t capacity() const noexcept { return _data.capacity(); }
    void reserve(std::size_t new_cap) { _data.reserve(new_cap); }
    void shrink_to_fit() { _data.shrink_to_fit();  }
    void clear() noexcept requires (!_all_static) {
        _extents.clear();
        _data.clear();
    }

    // Resize specific dimension
    void resize_dim(std::size_t dim, std::size_t new_extent) requires (!_all_static) {
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

    [[nodiscard]] reference front() {
        if (empty()) throw std::runtime_error("front() on empty tensor");
        return _data.front();
    }

    [[nodiscard]] const_reference front() const {
        if (empty()) throw std::runtime_error("front() on empty tensor");
        return _data.front();
    }

    [[nodiscard]] reference back() {
        if (empty()) throw std::runtime_error("back() on empty tensor");
        return _data.back();
    }

    [[nodiscard]] const_reference back() const {
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
        if constexpr (_all_static) {
            _data.swap(other._data);
        } else {
            _extents.swap(other._extents);
            _data.swap(other._data);
        }
    }

    // --- basic props ---
    [[nodiscard]] constexpr std::size_t rank() const noexcept requires (!_all_static && sizeof...(Ex) == 0UZ) { return _extents.size(); }
    [[nodiscard]] static consteval std::size_t rank() requires (_all_static || sizeof...(Ex) > 0UZ) { return sizeof...(Ex); }
    [[nodiscard]] constexpr std::size_t size() const noexcept requires (!_all_static) { return _data.size(); }
    [[nodiscard]] static consteval std::size_t size() requires (_all_static) { return _size_ct; }
    [[nodiscard]] constexpr std::size_t container_size() const noexcept { return size(); };
    [[nodiscard]] constexpr bool        empty()  const noexcept { return _data.empty(); }
    [[nodiscard]] constexpr std::size_t extent(std::size_t d) const noexcept {
        if constexpr (_all_static) {
            constexpr std::size_t E[]{Ex...}; return E[d];
        } else {
            return _extents[d];
        }
    }
    [[nodiscard]] constexpr std::span<const std::size_t> extents() const noexcept requires (!_all_static) {
        return std::span<const std::size_t>(_extents.data(), _extents.size());
    }
    [[nodiscard]] std::array<std::size_t, sizeof...(Ex)> extents() const noexcept requires (_all_static) {
        return std::array<std::size_t, sizeof...(Ex)>{Ex...};
    }
    [[nodiscard]] constexpr std::size_t stride(std::size_t r) const noexcept {
        std::size_t s = 1UZ;
        for (std::size_t i = r + 1; i < rank(); ++i) {
            s *= extent(i);
        }
        return s;
    }

    [[nodiscard]] constexpr std::pmr::vector<std::size_t> strides() const {
        std::pmr::vector<std::size_t> s;
        s.resize(rank());
        if (rank() == 0UZ) return s;
        std::size_t stride = 1UZ;
        for (std::size_t i = rank(); i-- > 0UZ;) {
            s[i] = stride;
            stride *= extent(i);
        }
        return s;
    }

    // --- iterators / STL compat ---
    [[nodiscard]] pointer       data()       noexcept { return std::to_address(_data.begin()); }
    [[nodiscard]] const_pointer data() const noexcept { return std::to_address(_data.cbegin()); }
    [[nodiscard]] constexpr pointer container_data() noexcept { return std::to_address(_data.begin()); }
    [[nodiscard]] constexpr const_pointer container_data() const noexcept { return std::to_address(_data.cbegin()); }
    [[nodiscard]] pointer begin() noexcept { return std::to_address(_data.begin()); }
    [[nodiscard]] pointer end()   noexcept { return std::to_address(_data.end()); }
    [[nodiscard]] const_pointer begin() const noexcept { return std::to_address(_data.cbegin()); }
    [[nodiscard]] const_pointer end()   const noexcept { return std::to_address(_data.cend()); }
    [[nodiscard]] const_pointer cbegin() const noexcept { return std::to_address(_data.cbegin()); }
    [[nodiscard]] const_pointer cend()   const noexcept { return std::to_address(_data.cend()); }

    [[nodiscard]] constexpr std::span<const size_t> extents() const noexcept {
        if constexpr (_all_static) {
            static constexpr std::array<std::size_t, sizeof...(Ex)> e{Ex...};
            return std::span<const std::size_t>(e);
        } else {
            return std::span(_extents);
        }
    }
    template <std::size_t idx>
    static consteval std::size_t extent() requires (_all_static) {
        constexpr std::size_t E[]{Ex...}; return E[idx];
    }
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
    template <std::integral... Indices>
    constexpr T& operator[](Indices... idx)       noexcept { return _data[index_of(idx...)]; }
    template <std::integral... Indices>
    constexpr const T& operator[](Indices... idx) const noexcept { return _data[index_of(idx...)]; }

    // checked access (throws std::out_of_range)
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

    template<typename OtherT, std::size_t... OtherEx>
    constexpr bool operator==(const Tensor<OtherT, OtherEx...>& other) const noexcept {
        if constexpr (std::is_same_v<ElementType, OtherT> && sizeof...(Ex) == sizeof...(OtherEx) && ((Ex == OtherEx) && ...)) {
            return _data == other._data; // same type and dimensions - compare data only
        } else { // different types or dimensions - can't be equal
            return false;
        }
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

    constexpr void fill(const T& value) noexcept {
        if constexpr (_all_static) {
            for (auto& elem : _data) {
                elem = value;
            }
        } else {
            std::ranges::fill(_data, value); // constexpr from C++26 onwards
        }
    }

    void reshape(std::initializer_list<std::size_t> newExtents) requires (!_all_static) { reshape(std::span(newExtents)); }

    template<std::ranges::range Range>
    requires (std::same_as<std::ranges::range_value_t<Range>, std::size_t>)
    void reshape(const Range& newExtents) requires (!_all_static) {
        const std::size_t newN = product(std::span(newExtents));
        if (newN != size()) throw std::runtime_error("Tensor::reshape: size mismatch");
        _extents.assign(std::ranges::begin(newExtents), std::ranges::end(newExtents));
    }

#if defined(TENSOR_HAVE_MDSPAN)
    auto to_mdspan() noexcept {
        using index_t = std::size_t;
        auto e = _extents; // copy to ensure a contiguous buffer for constructor
        // Use layout_right (row-major)
        return std::mdspan<T, std::dextents<index_t, std::dynamic_extent>>(data(), e);
    }
    auto to_mdspan() const noexcept {
        using index_t = std::size_t;
        auto e = _extents;
        return std::mdspan<const T, std::dextents<index_t, std::dynamic_extent>>(data(), e);
    }
#else
    template<bool is_const>
    struct View {
        using TPtr = std::conditional_t<is_const, const_pointer, pointer>;

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

    [[nodiscard]] auto to_mdspan() noexcept { return View<false>{ data(), extents(), strides() }; }
    [[nodiscard]] auto to_mdspan() const noexcept { return View<true>{ data(), extents(), strides() }; }
#endif

    // missing: subspan -- intentional for the time being

    template<std::ranges::range Range>
    requires (std::same_as<std::ranges::range_value_t<Range>, T> && !std::same_as<Range, std::initializer_list<T>>)
    constexpr Tensor& operator=(const Range& range) {
        if (std::ranges::size(range) != size()) {
            throw std::runtime_error("Range size doesn't match tensor size for assignment");
        }
        std::ranges::copy(range, _data.begin());
        return *this;
    }

    constexpr Tensor& operator=(const T& value) {
        fill(value);
        return *this;
    }
};

// ---- CTAD guides ----
template<typename T>
Tensor(std::initializer_list<T>) -> Tensor<T>;

template<typename T>
Tensor(std::initializer_list<std::initializer_list<T>>) -> Tensor<T>;

template<typename T, std::size_t N>
Tensor(const std::array<T, N>&) -> Tensor<T>;

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
