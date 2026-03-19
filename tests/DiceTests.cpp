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
  std::size_t min = 2 + 3 + 5;
  std::size_t max = (2 * 4) + (3 * 6) + 5;
  for (auto _ : std::ranges::views::iota(0, 1000)) {
    auto val = d();
    EXPECT_LE(val, max);
    EXPECT_GE(val, min);
  }
}

TEST(DiceTests, SingleDieRange) {
  auto d = "1d6"_dice;
  for (int i = 0; i < 1000; ++i) {
    auto val = d();
    EXPECT_GE(val, 1u);
    EXPECT_LE(val, 6u);
  }
}

TEST(DiceTests, TwoDiceTypesRange) {
  auto d = "1d6+1d8"_dice;
  for (int i = 0; i < 1000; ++i) {
    auto val = d();
    EXPECT_GE(val, 2u);  // min: 1+1
    EXPECT_LE(val, 14u); // max: 6+8
  }
}

TEST(DiceTests, ConstantOnlyRange) {
  auto d = "5"_dice;
  for (int i = 0; i < 100; ++i) {
    auto val = d();
    EXPECT_EQ(val, 5u);
  }
}

TEST(DiceTests, ZeroConstantRange) {
  auto d = "0"_dice;
  for (int i = 0; i < 100; ++i) {
    auto val = d();
    EXPECT_EQ(val, 0u);
  }
}

TEST(DiceTests, DieWithConstantRange) {
  auto d = "2d8+3"_dice;
  for (int i = 0; i < 1000; ++i) {
    auto val = d();
    EXPECT_GE(val, 5u);  // min: 2*1+3
    EXPECT_LE(val, 19u); // max: 2*8+3
  }
}
