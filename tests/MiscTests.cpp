#include <gtest/gtest.h>
import Common;

// #define TEST(a,b) void a##b()

// ============================================================
// DisjointSet
// ============================================================

// fresh set: find_set(i) == i
static_assert([] {
  DisjointSet<int> ds(5);
  return ds.find_set(0) == 0 && ds.find_set(1) == 1 && ds.find_set(4) == 4;
}());

// union_set returns true when sets differ
static_assert([] {
  DisjointSet<int> ds(3);
  return ds.union_set(0, 1);
}());

// union_set returns false when same set
static_assert([] {
  DisjointSet<int> ds(3);
  (void)ds.union_set(0, 1);
  return !ds.union_set(0, 1);
}());

// After union, find_set(a) == find_set(b)
static_assert([] {
  DisjointSet<int> ds(4);
  (void)ds.union_set(1, 3);
  return ds.find_set(1) == ds.find_set(3);
}());

// Transitive: union(0,1) + union(1,2) means find(0)==find(2)
static_assert([] {
  DisjointSet<int> ds(5);
  (void)ds.union_set(0, 1);
  (void)ds.union_set(1, 2);
  return ds.find_set(0) == ds.find_set(2);
}());

// Chain merge all elements
static_assert([] {
  DisjointSet<int> ds(4);
  (void)ds.union_set(0, 1);
  (void)ds.union_set(1, 2);
  (void)ds.union_set(2, 3);
  auto root = ds.find_set(0);
  return ds.find_set(1) == root && ds.find_set(2) == root && ds.find_set(3) == root;
}());

// Size-1 set
static_assert([] {
  DisjointSet<int> ds(1);
  return ds.find_set(0) == 0;
}());

// union_set with itself returns false
static_assert([] {
  DisjointSet<int> ds(3);
  return !ds.union_set(1, 1);
}());

// ============================================================
// OptionalReference
// ============================================================

// Default constructed: empty
static_assert([] {
  OptionalReference<int> opt;
  return !opt.has_value() && !opt;
}());

// From pointer: populated
static_assert([] {
  int x = 42;
  OptionalReference<int> opt(&x);
  return opt.has_value() && *opt == 42;
}());

// From reference: populated
static_assert([] {
  int x = 7;
  OptionalReference<int> opt(x);
  return opt.has_value() && *opt == 7;
}());

// Null pointer: empty
static_assert([] {
  OptionalReference<int> opt(nullptr);
  return !opt.has_value();
}());

// value_or: returns default when empty
static_assert([] {
  OptionalReference<int> opt;
  return opt.value_or(99) == 99;
}());

// value_or: returns value when populated
static_assert([] {
  int x = 10;
  OptionalReference<int> opt(x);
  return opt.value_or(99) == 10;
}());

// Mutation through reference modifies original
static_assert([] {
  int x = 3;
  OptionalReference<int> opt(x);
  *opt = 100;
  return x == 100;
}());

// doIfValue dispatches correctly
TEST(OptionalReference, DoIfValue) {
  int x = 42;
  OptionalReference<int> opt(x);
  bool called = false;
  opt.doIfValue([&](int &v) {
    called = true;
    EXPECT_EQ(v, 42);
  });
  EXPECT_TRUE(called);

  OptionalReference<int> empty;
  bool emptyCalled = false;
  empty.doIfValue([&](int &) { emptyCalled = true; });
  EXPECT_FALSE(emptyCalled);
}

// doIfNoValue dispatches correctly
TEST(OptionalReference, DoIfNoValue) {
  OptionalReference<int> empty;
  bool called = false;
  empty.doIfNoValue([&]() { called = true; });
  EXPECT_TRUE(called);

  int x = 1;
  OptionalReference<int> opt(x);
  bool notCalled = false;
  opt.doIfNoValue([&]() { notCalled = true; });
  EXPECT_FALSE(notCalled);
}

// doIf dispatches correctly
TEST(OptionalReference, DoIf) {
  int x = 10;
  OptionalReference<int> opt(x);
  int result = opt.doIf([]() { return -1; }, [](int &v) { return v * 2; });
  EXPECT_EQ(result, 20);

  OptionalReference<int> empty;
  int emptyResult = empty.doIf([]() { return -1; }, [](int &v) { return v * 2; });
  EXPECT_EQ(emptyResult, -1);
}

// range for iterates over the value once if filled
TEST(OptionalReference, RangeForFilled) {
  int n = 7;
  OptionalReference<int> opt(n);
  bool itered = false;
  for(auto& v : opt){
    EXPECT_EQ(v,7);
    EXPECT_EQ(&v,&n);
    EXPECT_FALSE(itered);
    itered = true;
    v=8;
    EXPECT_EQ(n,8);
  }
}
// range for iterates over nothing if not filled
TEST(OptionalReference, RangeForNotFilled) {
  OptionalReference<int> opt;
  for(auto& _ : opt){
    ADD_FAILURE();
  }
}


// ============================================================
// MustInit
// ============================================================

static_assert([] {
  MustInit<int> m(42);
  int v = m;
  return v == 42;
}());

static_assert([] {
  MustInit<int> m(0);
  int v = m;
  return v == 0;
}());

static_assert([] {
  const MustInit<int> m(7);
  const int &v = m;
  return v == 7;
}());

// ============================================================
// ERRCTOString
// ============================================================

static_assert(ERRCTOString(std::errc::invalid_argument) == "errc: invalid argument");
static_assert(ERRCTOString(std::errc::result_out_of_range) == "errc: result out of range");
static_assert(ERRCTOString(std::errc::address_in_use) == "errc: unimplemented code");

// ============================================================
// ERRCException
// ============================================================

TEST(ERRCException, WhatReturnsCorrectString) {
  ERRCException ex(std::errc::invalid_argument);
  EXPECT_STREQ(ex.what(), "errc: invalid argument");
}

TEST(ERRCException, GetErrorCodeRoundtrips) {
  ERRCException ex(std::errc::result_out_of_range);
  EXPECT_EQ(ex.getErrorCode(), std::errc::result_out_of_range);
}
