#include "TestHeader.h"
import Common;

// -- Empty path --
static_assert(std::ranges::equal(PathIterable({0, 0}), std::to_array<Dir>({{0, 0}})));

// -- Horizontal paths --
static_assert(std::ranges::equal(PathIterable({1, 0}),
                                 std::to_array<Dir>({{0, 0}, {1, 0}})));

static_assert(std::ranges::equal(PathIterable({3, 0}),
                                 std::to_array<Dir>({{0, 0}, {1, 0}, {2, 0}, {3, 0}})));

// -- Vertical paths --
static_assert(std::ranges::equal(PathIterable({0, 1}),
                                 std::to_array<Dir>({{0, 0}, {0, 1}})));

static_assert(std::ranges::equal(PathIterable({0, 3}),
                                 std::to_array<Dir>({{0, 0}, {0, 1}, {0, 2}, {0, 3}})));

// -- Diagonal paths --
static_assert(std::ranges::equal(PathIterable({2, 2}),
                                 std::to_array<Dir>({{0, 0}, {1, 1}, {2, 2}})));

static_assert(std::ranges::equal(PathIterable({3, 3}),
                                 std::to_array<Dir>({{0, 0}, {1, 1}, {2, 2}, {3, 3}})));

// -- Negative axis-aligned --
static_assert(std::ranges::equal(PathIterable({-3, 0}),
                                 std::to_array<Dir>({{0, 0}, {-1, 0}, {-2, 0}, {-3, 0}})));

static_assert(std::ranges::equal(PathIterable({0, -2}),
                                 std::to_array<Dir>({{0, 0}, {0, -1}, {0, -2}})));

// -- Negative diagonal --
static_assert(std::ranges::equal(PathIterable({-2, -2}),
                                 std::to_array<Dir>({{0, 0}, {-1, -1}, {-2, -2}})));
                                 
// -- Non-uniform path sequences --
static_assert(std::ranges::equal(PathIterable({3, 2}),
                                 std::to_array<Dir>({{0, 0}, {1, 0}, {1, 1}, {2, 1}, {2, 2}, {3, 2}})));
static_assert(std::ranges::equal(PathIterable({3, 1}),
                                 std::to_array<Dir>({{0, 0}, {1, 0}, {2, 1}, {3, 1}})));
