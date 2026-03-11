#include <gtest/gtest.h>
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

// Bug #4: Default SingleTypeGroup creates uniform_int_distribution(1, 0) (UB)
// A Group with fewer than MaxTypes die types leaves default-constructed
// SingleTypeGroup slots (faces_=0, number_=0). Group::operator() calls
// operator() on all slots, constructing an invalid distribution.
TEST(DiceTests, DefaultSingleTypeGroupDoesNotCauseUB) {
  auto d = "1d6"_dice;
  for (int i = 0; i < 100; i++) {
    auto val = d();
    EXPECT_GE(val, 1u);
    EXPECT_LE(val, 6u);
  }
}

// Trivial test so CTest registers the binary
TEST(DiceTests, DiceInRange) {
  auto d = "2d4+3d6+5"_dice;
  int min = 2+3+5;
  int max = 2*4+3*6+5;
  for(auto _ : std::ranges::views::iota(0,1000)){
    auto val = d();
    EXPECT_LE(val,max);
    EXPECT_GE(val,min);
  }
}
