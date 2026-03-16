#include "TestHeader.h"
import Common;

struct Accessor {
public:
  using element_type = bool;
  using data_handle_type = const char *;
  using reference = element_type;
  constexpr static reference access(data_handle_type arr, std::size_t n) {
    return arr[n] == ' ' || arr[n] == 'e' || arr[n] == 's';
  }
};
using spanT = std::mdspan<bool, std::extents<int, std::dynamic_extent, std::dynamic_extent>, std::layout_right, Accessor>;

static constexpr bool testMap(std::string_view map, int height, int width, int expectedLen) {
  if (static_cast<int>(map.size()) != height * width) {
    return false;
  }
  Position start{-1, -1};
  Position end{-1, -1};
  for (int row = 0; row < static_cast<int>(height); row++) {
    for (int col = 0; col < static_cast<int>(width); col++) {
      const char c = map[row * width + col];
      if (c == 's' || c == 'b') {
        start = {col, row};
      } else if (c == 'e' || c == 'f') {
        end = {col, row};
      }
    }
  }
  if (start == Position{-1, -1}) {
    return false;
  }
  if (end == Position{-1, -1}) {
    return false;
  }
  int len = 0;
  spanT mapView(map.data(), height, width);
  Position cSpot = start;
  while (len < expectedLen && cSpot != end) {
    auto dir = FindPath::findPath(mapView, cSpot, end);
    if (dir != capDir(dir)) {
      return false;
    }
    cSpot += dir;
    if (!cSpot.within({width - 1, height - 1})) {
      return false;
    }
    if (cSpot != end && !mapView[cSpot.y, cSpot.x]) {
      return false;
    }
    len++;
  }
  if (cSpot != end) {
    return false;
  }
  if (len != expectedLen) {
    return false;
  }
  return true;
}

static_assert(testMap("\
s  \
xx \
e  \
",
                      3, 3, 4));

// Unreachable goal returns Dir{0,0}
static constexpr bool testUnreachable(std::string_view map, int height, int width) {
  Position start{-1, -1};
  Position end{-1, -1};
  for (int row = 0; row < height; row++) {
    for (int col = 0; col < width; col++) {
      const char c = map[row * width + col];
      if (c == 's')
        start = {col, row};
      else if (c == 'e')
        end = {col, row};
    }
  }
  spanT mapView(map.data(), height, width);
  auto dir = FindPath::findPath(mapView, start, end);
  return dir == Dir{0, 0};
}

static_assert(testUnreachable(
    "s x"
    "xxx"
    "x e",
    3, 3));

TEST(TestPath, BasicPathAroundWall) {
  EXPECT_TRUE(testMap(
      "s  "
      "xx "
      "e  ",
      3, 3, 4));
}

TEST(TestPath, UnreachableGoal) {
  EXPECT_TRUE(testUnreachable(
      "s x"
      "xxx"
      "x e",
      3, 3));
}
