#include "TestHeader.h"
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
static_assert([] consteval {
  int x = 42;
  OptionalReference<int> opt(x);
  bool called = false;
  opt.doIfValue([&](int &v) {
    if (v == 42)
      called = true;
  });
  if (!called)
    return false;

  OptionalReference<int> empty;
  bool emptyCalled = false;
  empty.doIfValue([&](int &) { emptyCalled = true; });
  return !emptyCalled;
}());

// doIfNoValue dispatches correctly
static_assert([] consteval {
  OptionalReference<int> empty;
  bool called = false;
  empty.doIfNoValue([&]() { called = true; });
  if (!called)
    return false;

  int x = 1;
  OptionalReference<int> opt(x);
  bool notCalled = false;
  opt.doIfNoValue([&]() { notCalled = true; });
  return !notCalled;
}());

// doIf dispatches correctly
static_assert([] consteval {
  int x = 10;
  OptionalReference<int> opt(x);
  int result = opt.doIf([]() { return -1; }, [](int &v) { return v * 2; });
  if (result != 20)
    return false;

  OptionalReference<int> empty;
  int emptyResult = empty.doIf([]() { return -1; }, [](int &v) { return v * 2; });
  return emptyResult == -1;
}());

// range for iterates over the value once if filled
static_assert([] consteval {
  int n = 7;
  OptionalReference<int> opt(n);
  bool itered = false;
  for (auto &v : opt) {
    if (v != 7)
      return false;
    if (&v != &n)
      return false;
    if (itered)
      return false;
    itered = true;
    v = 8;
    if (n != 8)
      return false;
  }
  return true;
}());

// range for iterates over nothing if not filled
static_assert([] consteval {
  OptionalReference<int> opt;
  for (auto &v : opt) { //NOLINT(readability-use-anyofallof)
    (void)v;
    return false;
  }
  return true;
}());

// all of does not see anything if not filled
static_assert([] consteval {
  OptionalReference<int> opt;
  return std::ranges::all_of(opt,[](int&){return false;});
}());


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
