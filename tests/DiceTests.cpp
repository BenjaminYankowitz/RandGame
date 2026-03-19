#include "TestHeader.h"
import Common;

using namespace Dice::Literals;

// Valid: 1 die type (within MaxTypes=2)
static_assert([] consteval {
  auto d = "1d6"_dice;
  (void)d;
  return true;
}());

// Valid: 2 die types (at MaxTypes limit)
static_assert([] consteval {
  auto d = "1d6+1d8"_dice;
  (void)d;
  return true;
}());

// Valid: 1 die type + constant
static_assert([] consteval {
  auto d = "1d6+5"_dice;
  (void)d;
  return true;
}());

// Valid: 1 die type with multiple dice + constant
static_assert([] consteval {
  auto d = "2d8+3"_dice;
  (void)d;
  return true;
}());

// Trivial test if dice are in correct range
TEST(DiceTests, DiceInRange) {
  auto d = "2d4+3d6+5"_dice;
  for (auto _ : std::ranges::views::iota(0, 1000)) {
    auto val = d();
    EXPECT_LE(val, d.max());
    EXPECT_GE(val, d.min());
  }
}

TEST(DiceTests, SingleDieRange) {
  auto d = "1d6"_dice;
  for (int i = 0; i < 1000; ++i) {
    auto val = d();
    EXPECT_GE(val, d.min());
    EXPECT_LE(val, d.max());
  }
}

TEST(DiceTests, TwoDiceTypesRange) {
  auto d = "1d6+1d8"_dice;
  for (int i = 0; i < 1000; ++i) {
    auto val = d();
    EXPECT_GE(val, d.min());
    EXPECT_LE(val, d.max());
  }
}

TEST(DiceTests, ConstantOnlyRange) {
  auto d = "5"_dice;
  for (int i = 0; i < 100; ++i) {
    auto val = d();
    EXPECT_EQ(val, d.min());
  }
}

TEST(DiceTests, ZeroConstantRange) {
  auto d = "0"_dice;
  for (int i = 0; i < 100; ++i) {
    auto val = d();
    EXPECT_EQ(val, d.min());
  }
}

TEST(DiceTests, DieWithConstantRange) {
  auto d = "2d8+3"_dice;
  for (int i = 0; i < 1000; ++i) {
    auto val = d();
    EXPECT_GE(val, d.min());
    EXPECT_LE(val, d.max());
  }
}

// SingleTypeGroup min/max
static_assert([] consteval {
  auto d = "1d6"_diceST;
  return d.min() == 1 && d.max() == 6;
}());

static_assert([] consteval {
  auto d = "3d8"_diceST;
  return d.min() == 3 && d.max() == 24;
}());

static_assert([] consteval {
  auto d = "d20"_diceST;
  return d.min() == 1 && d.max() == 20;
}());

// Group min/max
static_assert([] consteval {
  auto d = "1d6"_dice;
  return d.min() == 1 && d.max() == 6;
}());

static_assert([] consteval {
  auto d = "2d8+3"_dice;
  return d.min() == 5 && d.max() == 19;
}());

static_assert([] consteval {
  auto d = "1d6+1d8"_dice;
  return d.min() == 2 && d.max() == 14;
}());

static_assert([] consteval {
  auto d = "5"_dice;
  return d.min() == 5 && d.max() == 5;
}());

static_assert([] consteval {
  auto d = "0"_dice;
  return d.min() == 0 && d.max() == 0;
}());

static_assert([] consteval {
  auto d = "2d4+3d6+5"_dice;
  return d.min() == 10 && d.max() == 31;
}());
