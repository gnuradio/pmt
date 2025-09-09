// qa_tensor.cpp - Reorganized and comprehensive test suite
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory_resource>
#include <numeric>
#include <ranges>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include <pmtv/pmt.hpp>

#include <print>

using pmtv::Tensor;

// ============================================================================
// BASIC FUNCTIONALITY (Types, Construction, Properties)
// ============================================================================

// ---- tiny constexpr det (2×2, 3×3) using [] ----
template <class T, std::size_t N> requires (N == 2)
constexpr T det(const Tensor<T, N, N>& A) {
    return A[0,0]*A[1,1] - A[0,1]*A[1,0];
}
template <class T, std::size_t N> requires (N == 3)
constexpr T det(const Tensor<T, N, N>& A) {
    return  A[0,0]*(A[1,1]*A[2,2] - A[1,2]*A[2,1])
          - A[0,1]*(A[1,0]*A[2,2] - A[1,2]*A[2,0])
          + A[0,2]*(A[1,0]*A[2,1] - A[1,1]*A[2,0]);
}

TEST(TensorBasic, UserAPI) {
    // Case 1: fully dynamic rank & extents
    std::vector<double> dynData{1,2,3,4,5,6};
    Tensor<double> A({3UZ,2UZ}, dynData);
    std::println("A: rank={}, size={}, A[2,1]={}", A.rank(), A.size(), A[2,1]);

    // Case 2: rank=1 with dynamic extent → data-only ctor
    Tensor<double, std::dynamic_extent> B({1,2,3,4,5});
    std::println("B: rank={}, size={}, B[4]={}", B.rank(), B.size(), B[4]);

    // Case 3: fully static 2×3, ultra-compact
    constexpr Tensor<double, 2UZ, 3UZ> C({
        1, 2, 3, // row 0
        4, 5, 7  // row 1
    });
    static_assert(Tensor<double, 2UZ, 3UZ>::rank() == 2);
    static_assert(Tensor<double, 2UZ, 3UZ>::extent<1>() == 3);
    static_assert(sizeof(Tensor<double,2,3>) == sizeof(std::array<double,6>));

    std::println("C: size={}, C[1,2]={}", C.size(), C[1,2]);

    // constexpr proof for flat ctor
    constexpr Tensor<double, 2UZ, 2UZ> D({1,2,3,4});
    static_assert(det(D) == -2);
    std::println("det(D) = {}", det(D));
}

TEST(TensorBasics, DefaultConstruction) {
    Tensor<int> tensor;
    EXPECT_EQ(tensor.rank(), 0UZ);
    EXPECT_EQ(tensor.size(), 0UZ);
    EXPECT_TRUE(tensor.empty());
    EXPECT_EQ(tensor.capacity(), 0UZ);
}

TEST(TensorConstruction, StaticConstexprConstruction) {
    constexpr Tensor<int, 2UZ, 2UZ> tensor{1, 2, 3, 4};
    static_assert(tensor.size() == 4);
    static_assert(decltype(tensor)::size() == 4);
    static_assert((tensor[0, 0]) == 1);
}

TEST(TensorBasic, TypeSizes){
    static_assert(sizeof(std::array<double, 3UZ>) >= 3UZ * sizeof(double));
    static_assert(sizeof(std::array<std::size_t, 3UZ>) >= 3UZ * sizeof(std::size_t));
    static_assert(sizeof(std::vector<double>) >= (sizeof(std::size_t) /* size */ + sizeof(std::size_t) /* capacity */ + sizeof(double*) /* begin() */));
    static_assert(sizeof(std::pmr::vector<std::size_t>) >= (sizeof(std::vector<double>) + sizeof(double*) /* PMR allocator */ ));
    static_assert(sizeof(std::pmr::vector<std::size_t>) > sizeof(std::vector<std::size_t>)); // usually 32 bytes on x86_64
    static_assert(sizeof(Tensor<double>) == (sizeof(std::pmr::vector<std::size_t>) + sizeof(std::pmr::vector<double>)));
    static_assert(sizeof(Tensor<double, std::dynamic_extent>) == (sizeof(std::array<std::size_t, 1UZ>) + sizeof(std::pmr::vector<double>)));
    static_assert(sizeof(Tensor<double, 3UZ, 2UZ>) == 3UZ * 2UZ * sizeof(double));
}

// Add to TensorBasic tests
TEST(TensorBasic, StaticTensorConstexpr) {
    // Test fully static tensor constexpr operations
    constexpr Tensor<int, 2UZ, 3UZ> mat{1, 2, 3, 4, 5, 6};
    static_assert((mat[1, 2]) == 6UZ);
    static_assert(mat.size() == 6UZ);
    static_assert(Tensor<int, 2UZ, 3UZ>::rank() == 2UZ);
    static_assert(Tensor<int, 2UZ, 3UZ>::size() == 6UZ);

    // Test nested initializer list for 2D
    constexpr Tensor<int, 2UZ, 3UZ> mat2{{1, 2, 3}, {4, 5, 6}};
    static_assert(mat2[0, 0] == 1);
    static_assert(mat2[1, 2] == 6);
}

TEST(TensorBasic, SemiStaticTensor) {
    Tensor<int, std::dynamic_extent, std::dynamic_extent> tensor({3UZ, 4UZ});
    EXPECT_EQ(tensor.rank(), 2UZ);
    static_assert(Tensor<int, std::dynamic_extent, std::dynamic_extent>::rank() == 2UZ);
    EXPECT_EQ(tensor.size(), 12UZ);

    auto extents = tensor.extents();
    EXPECT_EQ(extents[0], 3UZ);
    EXPECT_EQ(extents[1], 4UZ);
}

TEST(TensorConversion, CrossTypeConstructors) {
    // static to dynamic
    constexpr Tensor<int, 2UZ, 3UZ> static_tensor{1, 2, 3, 4, 5, 6};
    Tensor<int> dynamic_tensor(static_tensor);
    EXPECT_EQ(dynamic_tensor.rank(), 2UZ);
    EXPECT_EQ(dynamic_tensor.size(), 6UZ);
    EXPECT_TRUE(std::ranges::equal(dynamic_tensor, static_tensor));

    // assignment operator
    Tensor<int> dynamic_tensor2;
    dynamic_tensor2 = static_tensor;
    EXPECT_TRUE(std::ranges::equal(dynamic_tensor2, static_tensor));

    // dynamic to static with size check
    Tensor<double> source({2UZ, 2UZ});
    std::iota(source.begin(), source.end(), 1.0);
    Tensor<double, 2UZ, 2UZ> target(source);
    EXPECT_TRUE(std::ranges::equal(target, source));

    // assignment operator
    Tensor<double, 2UZ, 2UZ> target2;
    target2 = source;
    EXPECT_TRUE(std::ranges::equal(target, source));

    // Wrong size should throw
    Tensor<double> wrong_size({2UZ, 3UZ});
    EXPECT_THROW((Tensor<double, 2UZ, 2UZ>(wrong_size)), std::runtime_error);

    // assignment operator
    Tensor<double, 2UZ, 2UZ> dest;
    EXPECT_THROW((dest = wrong_size), std::runtime_error);

    // Type conversion
    Tensor<int, 2UZ, 2UZ> int_tensor{1, 2, 3, 4};
    Tensor<double, 2UZ, 2UZ> double_tensor(int_tensor);
    EXPECT_EQ((double_tensor[0, 0]), 1.0);
}

// Missing assignment operator tests - add these to your suite

TEST(TensorAssignment, StaticToStaticSameTypeDifferentSize) {
    Tensor<int, 2, 2> source{1, 2, 3, 4};
    Tensor<int, 3, 3> target;

    // Should throw due to size mismatch
    EXPECT_THROW(target = source, std::runtime_error);
}

TEST(TensorAssignment, StaticToStaticDifferentType) {
    Tensor<float, 2, 2> source{1.5f, 2.5f, 3.5f, 4.5f};
    Tensor<int, 2, 2> target;

    target = source;
    EXPECT_EQ((target[0, 0]), 1);  // truncation
    EXPECT_EQ((target[1, 1]), 4);
}

TEST(TensorAssignment, DynamicToStaticSizeCheck) {
    Tensor<double> source({3UZ, 3UZ}, std::vector{1., 2., 3., 4., 5., 6., 7., 8., 9.});
    Tensor<double, 2UZ, 2UZ> target;

    // Size mismatch should throw
    EXPECT_THROW(target = source, std::runtime_error);

    // Correct size should work
    Tensor<double> correct_source({2UZ, 2UZ}, std::vector{1., 2., 3., 4.});
    EXPECT_NO_THROW(target = correct_source);
}

TEST(TensorAssignment, SelfAssignmentSafety) {
    Tensor<int, 2UZ, 2UZ> static_tensor{1, 2, 3, 4};
    Tensor<int> dynamic_tensor({2UZ, 2UZ}, std::vector{5, 6, 7, 8});

    auto static_copy = static_tensor;
    auto dynamic_copy = dynamic_tensor;

    // safe self-assignment
    static_tensor = static_tensor;
    dynamic_tensor = dynamic_tensor;

    EXPECT_EQ(static_tensor, static_copy);
    EXPECT_EQ(dynamic_tensor, dynamic_copy);
}

TEST(TensorAssignment, ChainedAssignment) {
    Tensor<int, 2, 2> source{1, 2, 3, 4};
    Tensor<int, 2, 2> target1, target2, target3;

    // Should support chaining
    target3 = target2 = target1 = source;

    EXPECT_EQ(target1, source);
    EXPECT_EQ(target2, source);
    EXPECT_EQ(target3, source);
}

TEST(TensorMethods, StaticTensorLimitations) {
    constexpr Tensor<int, 3, 3> static_tensor{1, 2, 3, 4, 5, 6, 7, 8, 9};

    // These should work
    EXPECT_EQ(static_tensor.rank(), 2UZ);
    EXPECT_EQ(static_tensor.size(), 9UZ);
    EXPECT_EQ(static_tensor.extent(0), 3UZ);

    // Can't clear, reshape, or resize static tensors
    // These would be compile errors if uncommented:
    // static_tensor.clear();  // ERROR: requires (!_all_static)
    // static_tensor.reshape({9});  // ERROR: requires (!_all_static)
}

TEST(TensorEdgeCases, MixedOperations) {
    // Start with static tensor
    Tensor<int, 2, 3> static_t{1, 2, 3, 4, 5, 6};

    // Convert to dynamic
    Tensor<int> dynamic_t(static_t);

    // Modify dynamic
    dynamic_t.push_back(7);
    EXPECT_EQ(dynamic_t.rank(), 1UZ);  // Flattened
    EXPECT_EQ(dynamic_t.size(), 7UZ);

    // Can't assign back to static (size mismatch)
    // This would throw at runtime:
    // static_t = dynamic_t;  // ERROR: can't change static tensor size
}

TEST(TensorEdgeCases, PMRResourcePropagation) {
    std::array<std::byte, 1024> buffer;
    std::pmr::monotonic_buffer_resource resource(buffer.data(), buffer.size());

    // Create tensor with a custom resource
    Tensor<int> t1({2, 3}, &resource);

    // Copy constructor should use the same resource
    Tensor<int> t2(t1, &resource);
    EXPECT_EQ(t2._data.get_allocator().resource(), &resource);

    // Converting constructor with resource
    Tensor<double> t3(t1, &resource);
    EXPECT_EQ(t3._data.get_allocator().resource(), &resource);
}

TEST(TensorBasic, TypeConversion) {
    // static to dynamic
    Tensor<double, 2UZ, 3UZ> static_tensor{{1, 2, 3}, {4, 5, 6}};
    Tensor<double> dynamic_tensor(static_tensor);  // converts to fully dynamic

    // dynamic to static (with runtime check)
    Tensor<float> source({2UZ, 2UZ});
    Tensor<float, 2UZ, 2UZ> target(source);  // throws if dimensions don't match

    // type conversion
    Tensor<int, 3UZ, 3UZ> int_tensor{/*...*/};
    Tensor<double, 3UZ, 3UZ> double_tensor(int_tensor);  // int -> double conversion
}

TEST(TensorBasics, TypeTraits) {
    // Test bool -> uint8_t conversion
    static_assert(std::same_as<typename Tensor<bool>::value_type, std::uint8_t>,
                  "Tensor<bool> should store as uint8_t to avoid vector<bool> quirks");
    static_assert(std::same_as<typename Tensor<int>::value_type, int>);
    static_assert(std::same_as<typename Tensor<float>::value_type, float>);

    // Test concept
    static_assert(pmtv::PmtTensor<Tensor<int>>);
    static_assert(!pmtv::PmtTensor<std::vector<int>>);
}

TEST(TensorTypes, SizeCalculations) {
    // Verify memory layout expectations
    static_assert(sizeof(Tensor<double, 3, 3>) == 9 * sizeof(double));
    static_assert(sizeof(Tensor<double>) < sizeof(Tensor<double, 3UZ, 3UZ>));

    // Test constexpr size/rank calculations
    static_assert(Tensor<int, 2UZ, 3UZ>::size() == 6UZ);
    static_assert(Tensor<int, 2UZ, 3UZ>::rank() == 2UZ);
    static_assert(Tensor<int, 2UZ, 3UZ>::extent<1UZ>() == 3UZ);
}

TEST(TensorBasics, ExtentsConstruction) {
    // Single dimension
    Tensor<int> vec({5UZ});
    EXPECT_EQ(vec.rank(), 1UZ);
    EXPECT_EQ(vec.size(), 5UZ);
    EXPECT_EQ(vec.extent(0UZ), 5UZ);

    // Multi-dimensional
    Tensor<int> matrix({3UZ, 4UZ});
    EXPECT_EQ(matrix.rank(), 2UZ);
    EXPECT_EQ(matrix.size(), 12UZ);
    EXPECT_EQ(matrix.extent(0UZ), 3UZ);
    EXPECT_EQ(matrix.extent(1UZ), 4UZ);

    // 3D tensor
    Tensor<double> tensor3d({2UZ, 3UZ, 4UZ});
    EXPECT_EQ(tensor3d.rank(), 3UZ);
    EXPECT_EQ(tensor3d.size(), 24UZ);
}

TEST(TensorBasics, CountValueConstruction) {
    Tensor<double> tensor(5UZ, 42.0);
    EXPECT_EQ(tensor.rank(), 1UZ);
    EXPECT_EQ(tensor.size(), 5UZ);
    EXPECT_TRUE(std::ranges::all_of(tensor, [](double x) { return x == 42.0; }));
}

TEST(TensorBasics, IteratorConstruction) {
    std::vector<int> data{10, 20, 30, 40};
    Tensor<int> tensor(data.begin(), data.end());
    EXPECT_EQ(tensor.rank(), 1UZ);
    EXPECT_EQ(tensor.size(), 4UZ);
    EXPECT_TRUE(std::ranges::equal(tensor, data));
}

TEST(TensorBasics, ExtentsDataConstruction) {
    std::vector<int> data{1, 2, 3, 4, 5, 6};

    // Valid construction
    Tensor<int> tensor({2UZ, 3UZ}, data);
    EXPECT_EQ(tensor.rank(), 2UZ);
    EXPECT_EQ(tensor.size(), 6UZ);
    EXPECT_EQ((tensor[0, 0]), 1);
    EXPECT_EQ((tensor[1, 2]), 6);

    // Size mismatch should throw
    EXPECT_THROW((Tensor<int>({2UZ, 2UZ}, data)), std::runtime_error);
}

// ============================================================================
// DISAMBIGUATION AND TAGGED CONSTRUCTORS
// ============================================================================

TEST(TensorDisambiguation, TaggedConstructors) {
    std::vector<std::size_t> vals{10, 20, 30};

    // extents_from: tensor with shape 10x20x30
    Tensor<std::size_t> tensor1(pmtv::extents_from, vals);
    EXPECT_EQ(tensor1.rank(), 3UZ);
    EXPECT_EQ(tensor1.extent(0), 10UZ);
    EXPECT_EQ(tensor1.size(), 6000UZ);

    // data_from: 1D tensor with data {10,20,30}
    Tensor<std::size_t> tensor2(pmtv::data_from, vals);
    EXPECT_EQ(tensor2.rank(), 1UZ);
    EXPECT_EQ(tensor2.size(), 3UZ);
    EXPECT_EQ(tensor2[0], 10UZ);
    EXPECT_EQ(tensor2[2], 30UZ);
}

TEST(TensorDisambiguation, NonSizeTTypes) {
    // For non-size_t types, regular constructors work unambiguously
    std::vector<int> data{1, 2, 3, 4};

    Tensor<int> tensor1(data);  // This works fine (extents interpretation)
    EXPECT_EQ(tensor1.rank(), 1UZ);  // Actually creates 1D tensor due to explicit constructor

    Tensor<int> tensor2(pmtv::data_from, data);
    EXPECT_EQ(tensor2.rank(), 1UZ);
    EXPECT_TRUE(std::ranges::equal(tensor2, data));
}

// ============================================================================
// STD::VECTOR COMPATIBILITY
// ============================================================================

TEST(TensorVectorCompat, VectorConstruction) {
    std::vector<int> vec{1, 2, 3, 4, 5};

    // Explicit construction
    Tensor<int> tensor(vec);
    EXPECT_EQ(tensor.rank(), 1UZ);
    EXPECT_EQ(tensor.size(), 5UZ);
    EXPECT_TRUE(std::ranges::equal(tensor, vec));

    // PMR vector
    std::pmr::vector<int> pmr_vec{10, 20, 30};
    Tensor<int> tensor2(pmr_vec);
    EXPECT_EQ(tensor2.size(), 3UZ);
    EXPECT_EQ(tensor2[1], 20);
}

TEST(TensorVectorCompat, VectorAssignment) {
    std::vector<double> vec{1.5, 2.5, 3.5};
    Tensor<double> tensor({2, 2});  // Start as 2D

    // Assignment reshapes tensor to match vector
    tensor = vec;
    EXPECT_EQ(tensor.rank(), 1UZ);
    EXPECT_EQ(tensor.size(), 3UZ);
    EXPECT_TRUE(std::ranges::equal(tensor, vec));
}

TEST(TensorVectorCompat, VectorConversion) {
    Tensor<int> tensor({5UZ});
    std::iota(tensor.begin(), tensor.end(), 1);

    // Convert to std::vector
    auto vec = static_cast<std::vector<int>>(tensor);
    EXPECT_TRUE(std::ranges::equal(tensor, vec));

    // Multi-dimensional should throw
    Tensor<int> matrix({2UZ, 3UZ});
    EXPECT_THROW(std::ignore = static_cast<std::vector<int>>(matrix), std::runtime_error);
}

TEST(TensorVectorCompat, CrossTypeComparisons) {
    std::vector<int> vec{1, 2, 3, 4};
    Tensor<int> tensor(vec);

    // Symmetric comparison
    EXPECT_TRUE(tensor == vec);
    EXPECT_TRUE(vec == tensor);

    // Different sizes
    std::vector<int> diff_vec{1, 2, 3};
    EXPECT_FALSE(tensor == diff_vec);

    // Multi-dimensional vs vector
    Tensor<int> matrix({2UZ, 2UZ});
    std::iota(matrix.begin(), matrix.end(), 1);
    EXPECT_FALSE(matrix == vec);  // Different rank
}

TEST(TensorSTL, IteratorCategories) {
    Tensor<int> tensor({3, 4});

    // Verify iterator types
    static_assert(std::random_access_iterator<decltype(tensor.begin())>);
    static_assert(std::contiguous_iterator<decltype(tensor.begin())>);

    // Test iterator operations
    auto it = tensor.begin();
    EXPECT_EQ(it + 5, tensor.begin() + 5);  // Random access
    EXPECT_EQ(std::to_address(it), tensor.data());  // Contiguous
}

// ============================================================================
// CTAD (Class Template Argument Deduction)
// ============================================================================

TEST(TensorCTAD, BasicDeduction) {
    // From vector
    std::vector<double> vec{1.0, 2.0, 3.0};
    Tensor tensor1(vec);
    static_assert(std::same_as<decltype(tensor1), Tensor<double>>);

    // Count + value
    Tensor tensor2(5UZ, 42);
    static_assert(std::same_as<decltype(tensor2), Tensor<int>>);

    // Iterator range
    Tensor tensor3(vec.begin(), vec.end());
    static_assert(std::same_as<decltype(tensor3), Tensor<double>>);
}

TEST(TensorCTAD, TaggedDeduction) {
    std::vector<float> data{1.0f, 2.0f, 3.0f};

    // Tagged constructors
    Tensor tensor1(pmtv::data_from, data);
    static_assert(std::same_as<decltype(tensor1), Tensor<float>>);

    std::vector<std::size_t> extents{3UZ, 4UZ};
    Tensor tensor2(pmtv::extents_from, extents);
    static_assert(std::same_as<decltype(tensor2), Tensor<std::size_t>>);
}

// ============================================================================
// CORE OPERATIONS (Indexing, Access, Iteration)
// ============================================================================

TEST(TensorAccess, SingleIndexAccess) {
    Tensor<int> tensor({5UZ});
    std::iota(tensor.begin(), tensor.end(), 10);

    // Operator[] (unchecked)
    EXPECT_EQ(tensor[0], 10);
    EXPECT_EQ(tensor[4], 14);

    // at() (checked)
    EXPECT_NO_THROW(std::ignore = tensor.at(0));

    // Front/back
    EXPECT_EQ(tensor.front(), 10);
    EXPECT_EQ(tensor.back(), 14);
}

TEST(TensorAccess, MultiIndexAccess) {
    Tensor<int> tensor({2UZ, 3UZ});

    // Fill with pattern: tensor[i,j] = 10*i + j
    for (std::size_t i = 0UZ; i < 2UZ; ++i) {
        for (std::size_t j = 0UZ; j < 3UZ; ++j) {
            tensor[i, j] = static_cast<int>(10UZ * i + j);
        }
    }

    // Test access
    EXPECT_EQ((tensor[0UZ, 0UZ]), 0);
    EXPECT_EQ((tensor[0UZ, 2UZ]), 2);
    EXPECT_EQ((tensor[1UZ, 0UZ]), 10);
    EXPECT_EQ((tensor[1UZ, 2UZ]), 12);
}

TEST(TensorAccess, VariadicAtMethods) {
    Tensor<int> tensor({3UZ, 4UZ, 2UZ});
    std::iota(tensor.begin(), tensor.end(), 0);

    // Bounds-checked access
    EXPECT_EQ(tensor.at(0, 0, 0), 0);
    EXPECT_EQ(tensor.at(1, 2, 1), (tensor[1, 2, 1]));

    // Out of bounds
    EXPECT_THROW(std::ignore = tensor.at(3, 0, 0), std::out_of_range);
    EXPECT_THROW(std::ignore = tensor.at(0, 4, 0), std::out_of_range);

    // Wrong arity
    EXPECT_THROW(std::ignore = tensor.at(0, 0), std::out_of_range);
}

TEST(TensorAccess, SpanBasedAccess) {
    Tensor<int> tensor({2UZ, 3UZ});
    std::iota(tensor.begin(), tensor.end(), 0);

    // Span-based access
    std::array<std::size_t, 2UZ> indices{1, 2};
    EXPECT_EQ(tensor.at(indices), (tensor[1, 2]));

    // Wrong span size
    std::array<std::size_t, 1> wrong_size{0};
    EXPECT_THROW(std::ignore = tensor.at(std::span(wrong_size)), std::out_of_range);
}

TEST(TensorIteration, STLCompatibility) {
    Tensor<int> tensor({2UZ, 3UZ});
    std::iota(tensor.begin(), tensor.end(), 1);

    // STL algorithms
    int sum = std::accumulate(tensor.begin(), tensor.end(), 0);
    EXPECT_EQ(sum, 21);  // 1+2+3+4+5+6

    // Ranges
    EXPECT_TRUE(std::ranges::all_of(tensor, [](int x) { return x > 0; }));

    // Row-major order verification
    std::vector<int> expected{1, 2, 3, 4, 5, 6};
    EXPECT_TRUE(std::ranges::equal(tensor, expected));
}

TEST(TensorIteration, DataSpanAccess) {
    Tensor<int> tensor({2, 3});
    std::iota(tensor.begin(), tensor.end(), 0);

    // Data span access
    auto span = tensor.data_span();
    EXPECT_EQ(span.size(), 6UZ);
    EXPECT_EQ(span[0], 0);
    EXPECT_EQ(span[5], 5);

    // Const version
    const auto& const_tensor = tensor;
    auto const_span = const_tensor.data_span();
    EXPECT_EQ(const_span.size(), 6UZ);
}

// ============================================================================
// SHAPE OPERATIONS (Reshape, Resize)
// ============================================================================

TEST(TensorShape, BasicReshape) {
    Tensor<int> tensor({2UZ, 3UZ});
    std::iota(tensor.begin(), tensor.end(), 0);

    // Reshape to different layout (same total size)
    tensor.reshape({3UZ, 2UZ});
    EXPECT_EQ(tensor.rank(), 2UZ);
    EXPECT_EQ(tensor.extent(0), 3UZ);
    EXPECT_EQ(tensor.extent(1), 2UZ);
    EXPECT_EQ(tensor.size(), 6UZ);

    // Verify row-major interpretation: [0,1,2,3,4,5] -> [[0,1],[2,3],[4,5]]
    EXPECT_EQ((tensor[0, 0]), 0);
    EXPECT_EQ((tensor[0, 1]), 1);
    EXPECT_EQ((tensor[2, 1]), 5);
}

TEST(TensorShape, ReshapeErrors) {
    Tensor<int> tensor({2UZ, 3UZ});

    // Size mismatch should throw
    EXPECT_THROW(tensor.reshape({2UZ, 4UZ}), std::runtime_error);
    EXPECT_THROW(tensor.reshape({7UZ}), std::runtime_error);
}

TEST(TensorShape, MultiDimensionalResize) {
    Tensor<int> tensor;

    // Resize to multi-dimensional
    tensor.resize({2UZ, 3UZ, 4UZ}, 42);
    EXPECT_EQ(tensor.rank(), 3UZ);
    EXPECT_EQ(tensor.size(), 24UZ);
    EXPECT_EQ((tensor[0, 0, 0]), 42);

    // Change shape entirely
    tensor.resize({6UZ, 4UZ});
    EXPECT_EQ(tensor.rank(), 2UZ);
    EXPECT_EQ(tensor.size(), 24UZ);

    // Clear with empty resize
    tensor.resize({});
    EXPECT_TRUE(tensor.empty());
    EXPECT_EQ(tensor.rank(), 0UZ);
}

TEST(TensorShape, DimensionSpecificResize) {
    Tensor<int> tensor({3UZ, 4UZ});
    std::iota(tensor.begin(), tensor.end(), 0);

    // Resize specific dimension
    EXPECT_EQ(tensor.extent(1UZ), 4UZ);
    tensor.resize_dim(1UZ, 6UZ);  // 3x4 -> 3x6
    EXPECT_EQ(tensor.extent(0UZ), 3UZ);
    EXPECT_EQ(tensor.extent(1UZ), 6UZ);
    EXPECT_EQ(tensor.size(), 18UZ);

    // Invalid dimension
    EXPECT_THROW(tensor.resize_dim(5UZ, 10UZ), std::out_of_range);
}

TEST(TensorShape, Strides) {
    Tensor<int> tensor({3UZ, 4UZ, 2UZ});
    auto strides = tensor.strides();

    EXPECT_EQ(strides.size(), 3UZ);
    EXPECT_EQ(strides[0], 8UZ);  // 4*2
    EXPECT_EQ(strides[1], 2UZ);  // 2
    EXPECT_EQ(strides[2], 1UZ);  // 1
}

// ============================================================================
// ASSIGNMENT AND MODIFICATION
// ============================================================================

TEST(TensorAssignment, RangeAssignment) {
    Tensor<int> tensor({2UZ, 3UZ});

    // Assignment from std::vector (reshapes)
    std::vector<int> vec_data{1, 2, 3, 4, 5, 6};
    tensor = vec_data;
    EXPECT_EQ(tensor.rank(), 1UZ);
    EXPECT_TRUE(std::ranges::equal(tensor, vec_data));

    // Assignment from array (preserves shape if size matches)
    tensor.resize({2UZ, 3UZ});
    std::array<int, 6> arr_data{10, 11, 12, 13, 14, 15};
    tensor = arr_data;
    EXPECT_EQ(tensor.rank(), 2UZ);  // Shape preserved
    EXPECT_TRUE(std::ranges::equal(tensor, arr_data));
}

TEST(TensorAssignment, ValueAssignment) {
    Tensor<int> tensor({2UZ, 3UZ});

    // Fill with single value
    tensor = 99;
    EXPECT_TRUE(std::ranges::all_of(tensor, [](int x) { return x == 99; }));
}

TEST(TensorAssignment, AssignMethod) {
    Tensor<int> tensor;

    // assign from range
    std::vector<int> data{1, 2, 3, 4};
    tensor.assign(data);
    EXPECT_TRUE(std::ranges::equal(tensor, data));

    // assign count + value
    tensor.assign(3UZ, 99);
    EXPECT_EQ(tensor.size(), 3UZ);
    EXPECT_TRUE(std::ranges::all_of(tensor, [](int x) { return x == 99; }));
}

TEST(TensorModification, VectorLikeOperations) {
    Tensor<int> tensor;

    // Build up tensor
    tensor.push_back(10);
    tensor.push_back(20);
    tensor.emplace_back(30);

    EXPECT_EQ(tensor.size(), 3UZ);
    EXPECT_EQ(tensor.rank(), 1UZ);
    EXPECT_EQ(tensor.front(), 10);
    EXPECT_EQ(tensor.back(), 30);

    // Pop elements
    tensor.pop_back();
    EXPECT_EQ(tensor.size(), 2UZ);
    EXPECT_EQ(tensor.back(), 20);
}

TEST(TensorModification, MultiDimToVectorConversion) {
    // Multi-dimensional tensor flattens on push
    Tensor<int> matrix({2UZ, 3UZ});
    std::iota(matrix.begin(), matrix.end(), 0);

    matrix.push_back(100);
    EXPECT_EQ(matrix.rank(), 1UZ);
    EXPECT_EQ(matrix.size(), 7UZ);
    EXPECT_EQ(matrix.back(), 100);
}

TEST(TensorModification, Fill) {
    Tensor<int> tensor({2UZ, 3UZ});
    tensor.fill(42);
    EXPECT_TRUE(std::ranges::all_of(tensor, [](int x) { return x == 42; }));
}

// ============================================================================
// COMPARISONS
// ============================================================================

TEST(TensorComparison, EqualityOperator) {
    Tensor<int> A({2UZ, 2UZ});
    Tensor<int> B({2UZ, 2UZ});
    std::iota(A.begin(), A.end(), 0);
    std::iota(B.begin(), B.end(), 0);

    EXPECT_TRUE(A == B);

    // Different data
    B[0, 0] = 100;
    EXPECT_FALSE(A == B);

    // Different shape
    Tensor<int> C({2UZ, 3UZ});
    EXPECT_FALSE(A == C);
}

TEST(TensorComparison, SpaceshipOperator) {
    Tensor<int> A({2UZ, 2UZ});
    Tensor<int> B({2UZ, 2UZ});
    std::iota(A.begin(), A.end(), 0);
    std::iota(B.begin(), B.end(), 0);

    // Equal tensors
    EXPECT_EQ(A <=> B, std::strong_ordering::equal);

    // Different shapes (compares extents first)
    Tensor<int> C({3UZ, 2UZ});
    EXPECT_NE(A <=> C, std::strong_ordering::equal);

    // Different data
    B[0, 0] = 100;
    EXPECT_NE(A <=> B, std::strong_ordering::equal);
}

// ============================================================================
// ADVANCED FEATURES (MDspan, PMR, Views)
// ============================================================================

TEST(TensorAdvanced, PMRSupport) {
    std::array<std::byte, 4096UZ> buffer;
    std::pmr::monotonic_buffer_resource arena(buffer.data(), buffer.size());

    Tensor<int> tensor({4UZ, 4UZ}, &arena);
    std::iota(tensor.begin(), tensor.end(), 0);

    EXPECT_EQ(tensor.size(), 16UZ);
    EXPECT_EQ((tensor[3, 3]), 15);

    // Verify memory resource is used
    EXPECT_EQ(tensor._data.get_allocator().resource(), &arena);
}

TEST(TensorAdvanced, Views) {
    #if !defined(TENSOR_HAVE_MDSPAN)
    Tensor<int> tensor({2UZ, 3UZ});
    std::iota(tensor.begin(), tensor.end(), 0);

    // Non-const view
    auto view = tensor.to_mdspan();
    EXPECT_EQ(view.data(), tensor.data());
    EXPECT_EQ(view.extents().size(), 2UZ);
    EXPECT_EQ(view.strides().size(), 2UZ);

    // Const view
    const auto& const_tensor = tensor;
    auto const_view = const_tensor.to_mdspan();
    EXPECT_EQ(const_view.data(), tensor.data());
    #endif
}

#if __has_include(<mdspan>)
TEST(TensorAdvanced, MdspanIntegration) {
    Tensor<int> tensor({2, 3});
    std::iota(tensor.begin(), tensor.end(), 0);

    auto view = tensor.to_mdspan();
    EXPECT_EQ(view.extent(0), 2UZ);
    EXPECT_EQ(view.extent(1), 3UZ);

    // Test access through mdspan
    EXPECT_EQ(view(1, 2), (tensor[1, 2]));
}
#endif

TEST(TensorAdvanced, Swap) {
    Tensor<int> A({2UZ, 2UZ});
    Tensor<int> B({3UZ, 3UZ});
    std::iota(A.begin(), A.end(), 0);
    std::iota(B.begin(), B.end(), 10);

    auto A_copy = A;
    auto B_copy = B;

    // Member swap
    A.swap(B);
    EXPECT_EQ(A, B_copy);
    EXPECT_EQ(B, A_copy);

    // Free function swap
    swap(A, B);
    EXPECT_EQ(A, A_copy);
    EXPECT_EQ(B, B_copy);
}

// ============================================================================
// EDGE CASES AND ERROR HANDLING
// ============================================================================

TEST(TensorEdgeCases, EmptyTensor) {
    Tensor<int> tensor;

    // Operations on empty tensor
    EXPECT_THROW(std::ignore = tensor.front(), std::runtime_error);
    EXPECT_THROW(std::ignore = tensor.back(), std::runtime_error);
    EXPECT_THROW(tensor.pop_back(), std::runtime_error);

    // Should be able to reserve on empty
    EXPECT_NO_THROW(tensor.reserve(100UZ));
    EXPECT_GE(tensor.capacity(), 100UZ);
}

TEST(TensorEdgeCases, SingleElement) {
    Tensor<int> tensor({1UZ});
    tensor[0] = 42;

    EXPECT_EQ(tensor.size(), 1UZ);
    EXPECT_EQ(tensor.front(), 42);
    EXPECT_EQ(tensor.back(), 42);
    EXPECT_EQ(tensor[0], 42);

    // Should be able to pop
    tensor.pop_back();
    EXPECT_TRUE(tensor.empty());
}

TEST(TensorEdgeCases, ZeroDimensions) {
    // Tensor with zero in one dimension
    Tensor<int> tensor({3UZ, 0UZ, 4UZ});
    EXPECT_EQ(tensor.size(), 0UZ);
    EXPECT_EQ(tensor.rank(), 3UZ);
}

TEST(TensorEdgeCases, BoolTensorBehavior) {
    // Verify bool -> uint8_t conversion works correctly
    Tensor<bool> tensor(5UZ, true);

    // Should store as uint8_t internally
    static_assert(std::same_as<decltype(tensor)::value_type, uint8_t>);

    // But behave like bool
    EXPECT_TRUE(tensor[0]);
    tensor[0] = false;
    EXPECT_FALSE(tensor[0]);
}

TEST(TensorErrors, OverflowDetection) {
    // Overflow in extents product
    const std::size_t big = std::numeric_limits<std::size_t>::max() / 2UZ + 1UZ;
    EXPECT_THROW((Tensor<int>({big, 3})), std::length_error);
}

TEST(TensorErrors, BoundsChecking) {
    Tensor<int> tensor({2UZ, 3UZ});

    // Various bounds violations
    EXPECT_THROW(std::ignore = tensor.at(2, 0), std::out_of_range);
    EXPECT_THROW(std::ignore = tensor.at(0, 3), std::out_of_range);
    EXPECT_THROW(std::ignore = tensor.at(0, 0, 0), std::out_of_range);  // Wrong arity

    // Span-based bounds checking
    std::array<std::size_t, 2UZ> bad_idx{2UZ, 0UZ};
    EXPECT_THROW(std::ignore = tensor.at(std::span(bad_idx)), std::out_of_range);
}

// ============================================================================
// PART 11: MEMORY MANAGEMENT AND PERFORMANCE
// ============================================================================

TEST(TensorMemory, CapacityManagement) {
    Tensor<int> tensor;

    // Reserve capacity
    tensor.reserve(1000UZ);
    EXPECT_GE(tensor.capacity(), 1000UZ);

    // Add elements without reallocation
    for (int i = 0; i < 500; ++i) {
        tensor.push_back(i);
    }
    EXPECT_EQ(tensor.size(), 500UZ);
    EXPECT_GE(tensor.capacity(), 1000UZ);

    // Shrink
    tensor.shrink_to_fit();
    // Note: shrink_to_fit is not guaranteed to reduce capacity
}

TEST(TensorMemory, MoveSemantics) {
    Tensor<int> source({100UZ});
    std::iota(source.begin(), source.end(), 0);
    auto original = std::vector<int>(source.begin(), source.end());

    // Move construction
    Tensor<int> moved(std::move(source));
    EXPECT_TRUE(std::ranges::equal(moved, original));

    // Move assignment
    Tensor<int> target;
    target = std::move(moved);
    EXPECT_TRUE(std::ranges::equal(target, original));
}

// ============================================================================
// PART 12: STRESS TESTS
// ============================================================================

TEST(TensorStress, LargeOperations) {
    Tensor<int> tensor({1000UZ, 1000UZ});
    EXPECT_EQ(tensor.size(), 1000000UZ);

    // Multiple reshapes
    tensor.reshape({2000UZ, 500UZ});
    EXPECT_EQ(tensor.size(), 1000000UZ);

    tensor.reshape({100UZ, 100UZ, 100UZ});
    EXPECT_EQ(tensor.size(), 1000000UZ);

    tensor.reshape({1000000UZ});
    EXPECT_EQ(tensor.rank(), 1UZ);
}

TEST(TensorStress, ManyOperations) {
    Tensor<int> tensor;

    // Many push operations
    for (int i = 0; i < 10000; ++i) {
        tensor.push_back(i);
    }
    EXPECT_EQ(tensor.size(), 10000UZ);

    // verify data integrity
    for (int i = 0; i < 10000; ++i) {
        EXPECT_EQ(tensor[i], i);
    }

    // many pop operations
    for (int i = 0; i < 5000; ++i) {
        tensor.pop_back();
    }
    EXPECT_EQ(tensor.size(), 5000UZ);
}

TEST(TensorBoundary, MaximumDimensions) {
    // Test with many dimensions (stress test rank handling)
    std::vector<std::size_t> many_dims(10, 2);  // 10D tensor of 2^10 = 1024 elements
    Tensor<int> high_rank_tensor(many_dims);

    EXPECT_EQ(high_rank_tensor.rank(), 10);
    EXPECT_EQ(high_rank_tensor.size(), 1024);

    // Access with many indices - create vector first, then span
    std::vector<std::size_t> zero_indices(10, 0);
    high_rank_tensor.at(std::span<const std::size_t>(zero_indices)) = 42;
    EXPECT_EQ(high_rank_tensor.at(std::span<const std::size_t>(zero_indices)), 42);
}

TEST(TensorBoundary, SingleElementTensorOperations) {
    // 1x1x1x1 tensor (many dimensions, one element)
    Tensor<int> single_elem({1, 1, 1, 1});
    single_elem[0, 0, 0, 0] = 99;

    EXPECT_EQ(single_elem.size(), 1);
    EXPECT_EQ(single_elem.front(), 99);
    EXPECT_EQ(single_elem.back(), 99);

    // Reshape to different single-element shapes
    single_elem.reshape({1});
    EXPECT_EQ(single_elem[0], 99);
}

TEST(TensorBoundary, ExtentEdgeCases) {
    // Extent size 1 in various positions
    Tensor<int> tensor1({1, 5, 1});
    EXPECT_EQ(tensor1.size(), 5);

    // Very large single dimension
    if constexpr (sizeof(std::size_t) >= 8) {  // Only on 64-bit systems
        const std::size_t large_size = 1UL << 20;  // 1M elements
        Tensor<char> large_tensor(large_size, 'x');
        EXPECT_EQ(large_tensor.size(), large_size);
        EXPECT_EQ(large_tensor.front(), 'x');
        EXPECT_EQ(large_tensor.back(), 'x');
    }
}


TEST(TensorBoundary, IndexingEdgeCases) {
    Tensor<int> tensor({3, 4, 5});
    std::iota(tensor.begin(), tensor.end(), 0);

    // Test all corner indices - use parentheses to avoid macro parsing issues
    EXPECT_EQ((tensor[0, 0, 0]), 0);                         // First element
    EXPECT_EQ((tensor[2, 3, 4]), int(tensor.size() - 1));   // Last element

    // Test stride boundaries
    EXPECT_EQ((tensor[1, 0, 0]), 20);  // Second "plane"
    EXPECT_EQ((tensor[0, 1, 0]), 5);   // Second "row"
    EXPECT_EQ((tensor[0, 0, 1]), 1);   // Second "column"
}

TEST(TensorBoundary, AllocatorEdgeCases) {
    auto* null_resource = std::pmr::null_memory_resource();

    // static tensor should work (no allocation)
    EXPECT_NO_THROW((Tensor<int, 2, 2>{}));

    // dynamic tensor should fail on allocation attempt
    EXPECT_THROW((Tensor<int>({2, 2}, null_resource)), std::bad_alloc);
}

TEST(TensorConstruction, NestedInitializerLists2D) {
    // Your implementation supports 3D nested lists but you don't test them
    Tensor<int, 2UZ, 3UZ> tensor{{1, 2, 3}, {4, 5, 6}};

    EXPECT_EQ((tensor[0, 0]), 1);
    EXPECT_EQ((tensor[1, 0]), 4);
}

TEST(TensorConstruction, NestedInitializerLists3D) {
    // Your implementation supports 3D nested lists but you don't test them
    Tensor<int, 2UZ, 2UZ, 2UZ> tensor{{{1, 2}, {3, 4}}, {{5, 6}, {7, 8}}};

    EXPECT_EQ((tensor[0, 0, 0]), 1);
    EXPECT_EQ((tensor[0, 1, 1]), 4);
    EXPECT_EQ((tensor[1, 0, 0]), 5);
    EXPECT_EQ((tensor[1, 1, 1]), 8);

    // Wrong dimensions should throw
    EXPECT_THROW((Tensor<int, 2, 2, 2>{{{1, 2, 3}, {4, 5, 6}}}), std::runtime_error);
}

TEST(TensorConstruction, MoveConstructorForVector) {
    std::pmr::vector<int> vec{1, 2, 3, 4, 5};
    Tensor<int> tensor(std::move(vec));

    EXPECT_EQ(tensor.rank(), 1UZ);
    EXPECT_EQ(tensor.size(), 5UZ);
    EXPECT_EQ(tensor[0], 1);
    // The vector should have been moved (though data pointer check is implementation-dependent)
}

TEST(TensorConstruction, AllocatorMismatchInMove) {
    std::array<std::byte, 1024UZ> buffer1{};
    std::array<std::byte, 1024UZ> buffer2{};
    std::pmr::monotonic_buffer_resource resource1(buffer1.data(), buffer1.size());
    std::pmr::monotonic_buffer_resource resource2(buffer2.data(), buffer2.size());

    std::pmr::vector<int> vec({1, 2, 3, 4}, &resource1);

    // Moving with different allocator should copy, not move
    Tensor<int> tensor(std::move(vec), &resource2);

    EXPECT_EQ(tensor.size(), 4);
    EXPECT_EQ(tensor._data.get_allocator().resource(), &resource2);
}

TEST(TensorConstruction, InitializerListSizeErrors) {
    // Static tensor with wrong size initializer
    EXPECT_THROW((Tensor<int, 2, 2>{1, 2, 3}), std::runtime_error);  // Too few
    EXPECT_THROW((Tensor<int, 2, 2>{1, 2, 3, 4, 5}), std::runtime_error);  // Too many
}

// Also fix the nodiscard warning:
TEST(TensorBoundary, ZeroSizedDimensions) {
    Tensor<int> tensor({3, 0, 2});
    EXPECT_EQ(tensor.size(), 0);
    EXPECT_EQ(tensor.rank(), 3);
    EXPECT_TRUE(tensor.empty());

    Tensor<int> all_zero({0, 0, 0});
    EXPECT_EQ(all_zero.size(), 0);

    // Operations on zero-sized tensors
    EXPECT_NO_THROW(tensor.reshape({0}));

    // Fix nodiscard warning by using std::ignore
    EXPECT_THROW(std::ignore = tensor.front(), std::runtime_error);
}

TEST(TensorErrors, ConversionErrors) {
    // Rank mismatch in conversion constructor
    Tensor<int, 2, 2> source{1, 2, 3, 4};
    // Semi-static rank mismatch
    Tensor<int> dynamic_source({2, 2, 2});
    EXPECT_THROW((Tensor<int, std::dynamic_extent, std::dynamic_extent>(dynamic_source)), std::runtime_error);
}

TEST(TensorErrors, ExtentsDataMismatch) {
    std::vector<int> data{1, 2, 3, 4, 5, 6};

    // Product doesn't match data size
    EXPECT_THROW((Tensor<int>({2, 4}, data)), std::runtime_error);  // 8 != 6
    EXPECT_THROW((Tensor<int>({3, 3}, data)), std::runtime_error);  // 9 != 6

    // Empty extents with data
    EXPECT_THROW((Tensor<int>({}, data)), std::runtime_error);
}

TEST(TensorErrors, IndexCalculationOverflow) {
    // This might not throw in practice but tests edge case
    const std::size_t max_val = std::numeric_limits<std::size_t>::max();

    // Very large dimensions that could overflow in index calculation
    EXPECT_THROW((Tensor<char>({max_val, max_val})), std::length_error);
}

TEST(TensorErrors, StaticTensorOperationErrors) {
    Tensor<int, 2, 3> static_tensor{1, 2, 3, 4, 5, 6};

    // Operations that should be compile-time errors are tested via concepts
    // But runtime errors for inappropriate static tensor usage:

    // Converting to vector with wrong rank
    Tensor<int, 2, 3> matrix{1, 2, 3, 4, 5, 6};
    EXPECT_THROW(std::ignore = static_cast<std::vector<int>>(matrix), std::runtime_error);
}

TEST(TensorErrors, PMRResourceExhaustion) {
    auto* null_resource = std::pmr::null_memory_resource(); // always throws
    EXPECT_THROW({ Tensor<int> tensor({10}, null_resource); }, std::bad_alloc);

    // test with a vector construction that should fail
    EXPECT_THROW({
        Tensor<double> tensor(100, 3.14, null_resource);  // 100 elements with null allocator
    }, std::bad_alloc);

    EXPECT_THROW({
        Tensor<int> tensor(null_resource);
        tensor.push_back(42);  // fails on first allocation
    }, std::bad_alloc);
}
