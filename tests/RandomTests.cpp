#include "TestHeader.h"
import Common;

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

TEST(RandomTests, UniformIntMinEqualsMax) {
  for (int i = 0; i < 100; ++i) {
    EXPECT_EQ(Rnd::uniform_int(7, 7), 7);
  }
}

TEST(RandomTests, ShuffleEmptyVector) {
  std::vector<int> v;
  Rnd::shuffle(v);
  EXPECT_TRUE(v.empty());
}

TEST(RandomTests, ShuffleSingleElement) {
  std::vector<int> v = {42};
  Rnd::shuffle(v);
  EXPECT_EQ(v.size(), 1u);
  EXPECT_EQ(v[0], 42);
}
