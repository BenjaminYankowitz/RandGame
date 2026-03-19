#include "TestHeader.h"
import Common;
import std;

TEST(RandomTests, UniformIntInRange) {
  for (int i = 0; i < 1000; ++i) {
    auto val = Rnd::uniform_int(5, 10);
    EXPECT_GE(val, 5);
    EXPECT_LE(val, 10);
  }
}

TEST(RandomTests, RndInRange) {
  for (int i = 0; i < 1000; ++i) {
    auto val = Rnd::rnd(6);
    EXPECT_GE(val, 0);
    EXPECT_LE(val, 5);
  }
}

TEST(RandomTests, FlipReturnsBothValues) {
  bool seenTrue = false;
  bool seenFalse = false;
  for (int i = 0; i < 1000; ++i) {
    if (Rnd::flip())
      seenTrue = true;
    else
      seenFalse = true;
  }
  EXPECT_TRUE(seenTrue);
  EXPECT_TRUE(seenFalse);
}

TEST(RandomTests, UniformRealInRange) {
  for (int i = 0; i < 1000; ++i) {
    auto val = Rnd::uniform_real<float>(2.0f, 5.0f);
    EXPECT_GE(val, 2.0f);
    EXPECT_LE(val, 5.0f);
  }
}

TEST(RandomTests, Uniform01InRange) {
  for (int i = 0; i < 1000; ++i) {
    auto val = Rnd::uniform_01();
    EXPECT_GE(val, 0.0);
    EXPECT_LE(val, 1.0);
  }
}
