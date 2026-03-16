#include "TestHeader.h"
import Common;
import std;

TEST(RandomTests, UniformRealFloatReturnsFloat) {
  auto result = Rnd::uniform_real<float>(0.0f, 1.0f);
  EXPECT_TRUE((std::is_same_v<decltype(result), float>));
}

TEST(RandomTests, UniformRealDoubleReturnsDouble) {
  auto result = Rnd::uniform_real<double>(0.0, 1.0);
  EXPECT_TRUE((std::is_same_v<decltype(result), double>));
}

TEST(RandomTests, Uniform01ReturnsDouble) {
  auto result = Rnd::uniform_01();
  EXPECT_TRUE((std::is_same_v<decltype(result), double>));
}
