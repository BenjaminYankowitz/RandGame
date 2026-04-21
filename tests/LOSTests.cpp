#include "TestHeader.h"
import Common;

struct BoolMap {
  const StaticPositionArr<bool> *real;
  [[nodiscard]] constexpr int extent(int n) const noexcept {
    switch (n) {
    case 0:
      return real->rows();
    case 1:
      return real->cols();
    default:
      std::unreachable();
    }
  }
  [[nodiscard]] bool operator[](Position p) const { return (*real)[p]; }
};

// Helper: check that allInLineOfSight returns exactly the set of positions
// for which inLineOfSight returns true.
void verifyMatch(const StaticPositionArr<bool> &map, Position start) {
  BoolMap bmap{&map};
  auto result = LineOfSight::allInLineOfSight(bmap, start);

  // Build set from vector for fast lookup
  auto posHash = [](Position p) { return std::hash<int>()(p.x) ^ (std::hash<int>()(p.y) << 16); };
  auto posEq = [](Position a, Position b) { return a == b; };
  std::unordered_set<Position, decltype(posHash), decltype(posEq)> resultSet(result.begin(), result.end(), 0, posHash, posEq);

  EXPECT_EQ(static_cast<std::uint64_t>(std::ranges::distance(result)), resultSet.size()); // no duplicates

  for (auto pos : map.indexIter()) {
    bool expected = LineOfSight::inLineOfSight(bmap, start, pos);
    bool inResult = resultSet.contains(pos);
    ASSERT_EQ(expected, inResult);
  }
}

// Open map — everything should be visible
TEST(LOSTests, OpenMap) {
  constexpr int W = 10;
  constexpr int H = 10;
  StaticPositionArr<bool> map(W, H);
  map.fill(true);

  verifyMatch(map, Position{5, 5});
}

// Map with border walls
TEST(LOSTests, BorderWalls) {
  constexpr int W = 12;
  constexpr int H = 12;
  StaticPositionArr<bool> map(W, H);
  map.fill(true);

  for (int x = 0; x < W; ++x) {
    map[Position{x, 0}] = false;
    map[Position{x, H - 1}] = false;
  }
  for (int y = 0; y < H; ++y) {
    map[Position{0, y}] = false;
    map[Position{W - 1, y}] = false;
  }

  verifyMatch(map, Position{6, 6});
}

// Map with an interior wall blocking LOS
TEST(LOSTests, InteriorWall) {
  constexpr int W = 15;
  constexpr int H = 10;
  StaticPositionArr<bool> map(W, H);
  map.fill(true);

  // Vertical wall at x=7 from y=2 to y=7
  for (int y = 2; y <= 7; ++y)
    map[Position{7, y}] = false;

  verifyMatch(map, Position{3, 5});
}

// Start in a corner
TEST(LOSTests, CornerStart) {
  constexpr int W = 8;
  constexpr int H = 8;
  StaticPositionArr<bool> map(W, H);
  map.fill(true);

  verifyMatch(map, Position{0, 0});
}

// Small room with a door
TEST(LOSTests, RoomWithDoor) {
  constexpr int W = 20;
  constexpr int H = 15;
  StaticPositionArr<bool> map(W, H);
  map.fill(true);

  // Room walls: box from (5,3) to (12,9)
  for (int x = 5; x <= 12; ++x) {
    map[Position{x, 3}] = false;
    map[Position{x, 9}] = false;
  }
  for (int y = 3; y <= 9; ++y) {
    map[Position{5, y}] = false;
    map[Position{12, y}] = false;
  }
  // Door at (8, 3)
  map[Position{8, 3}] = true;

  // From outside the room
  verifyMatch(map, Position{8, 1});
  // From inside the room
  verifyMatch(map, Position{8, 6});
}

// Multiple positions on the same map (the LOSVisualizer map)
TEST(LOSTests, ComplexMap) {
  constexpr int W = 40;
  constexpr int H = 20;
  StaticPositionArr<bool> map(W, H);
  map.fill(true);

  // Border walls
  for (int x = 0; x < W; x++) {
    map[Position{x, 0}] = false;
    map[Position{x, H - 1}] = false;
  }
  for (int y = 0; y < H; y++) {
    map[Position{0, y}] = false;
    map[Position{W - 1, y}] = false;
  }

  // Interior walls
  for (int y = 3; y < 12; y++)
    map[Position{10, y}] = false;
  for (int x = 10; x < 20; x++)
    map[Position{x, 8}] = false;
  for (int y = 5; y < 15; y++)
    map[Position{25, y}] = false;
  for (int x = 15; x < 25; x++)
    map[Position{x, 14}] = false;

  verifyMatch(map, Position{5, 10});
  verifyMatch(map, Position{20, 5});
  verifyMatch(map, Position{30, 10});
}
