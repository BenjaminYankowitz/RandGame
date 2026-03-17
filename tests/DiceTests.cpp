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
  std::size_t min = 2+3+5;
  std::size_t max = (2*4)+(3*6)+5;
  for(auto _ : std::ranges::views::iota(0,1000)){
    auto val = d();
    EXPECT_LE(val,max);
    EXPECT_GE(val,min);
  }
}
