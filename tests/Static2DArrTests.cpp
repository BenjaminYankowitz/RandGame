#include <gtest/gtest.h>
import Common;


template<class T>
concept acceptsUnevenArray = requires{ T{{42},{32,53}};};
static_assert(!acceptsUnevenArray<Static2DArr<int>>);

TEST(Static2DArrTests, InitializerListPopulatesAllColumns) {
  Static2DArr<int> arr = {{1, 2, 3}, {4, 5, 6}};
  EXPECT_EQ((arr[0, 0]), 1);
  EXPECT_EQ((arr[0, 1]), 2);
  EXPECT_EQ((arr[0, 2]), 3);
  EXPECT_EQ((arr[1, 0]), 4);
  EXPECT_EQ((arr[1, 1]), 5);
  EXPECT_EQ((arr[1, 2]), 6);
}

TEST(Static2DArrTests, InitializerListSingleRow) {
  Static2DArr<int> arr = {{10, 20, 30}};
  EXPECT_EQ(arr.rows(), 1u);
  EXPECT_EQ(arr.cols(), 3u);
  EXPECT_EQ((arr[0, 0]), 10);
  EXPECT_EQ((arr[0, 1]), 20);
  EXPECT_EQ((arr[0, 2]), 30);
}

TEST(Static2DArrTests, InitializerListSingleElement) {
  Static2DArr<int> arr = {{42}};
  EXPECT_EQ(arr.rows(), 1u);
  EXPECT_EQ(arr.cols(), 1u);
  EXPECT_EQ((arr[0, 0]), 42);
}

TEST(Static2DArrTests, BasicConstruction) {
  Static2DArr<int> arr(3, 4);
  EXPECT_EQ(arr.rows(), 3u);
  EXPECT_EQ(arr.cols(), 4u);
  EXPECT_EQ(arr.size(), 12u);
}