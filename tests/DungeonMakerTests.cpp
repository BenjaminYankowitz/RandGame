#include "TestHeader.h"
import DungeonMaker;
import Common;
import std;

constexpr int Wall = 1;
constexpr int Empty = 0;

namespace {
constexpr int countRegions(StaticPositionArr<int> &floor) {
  return DungeonMaker::labelRegions<Wall, Empty>(floor).numRegions();
}
} // namespace

// --- labelRegions tests ---

static_assert([] {
  StaticPositionArr<int> floor(3, 3);
  floor.fill(Empty);
  auto info = DungeonMaker::labelRegions<Wall, Empty>(floor);
  if (info.numRegions() != 1)
    return false;
  return std::ranges::all_of(info.regionOf, [](auto i) { return i == 0; });
}(),
              "LabelRegions AllEmpty");

static_assert([] {
  StaticPositionArr<int> floor(3, 3);
  floor.fill(Wall);
  auto info = DungeonMaker::labelRegions<Wall, Empty>(floor);
  if (info.numRegions() != 0)
    return false;
  return std::ranges::all_of(info.regionOf, [](auto i) { return i == -1; });
}(),
              "LabelRegions AllWalls");

static_assert([] {
  // E W E
  // E W E
  StaticPositionArr<int> floor(3, 2);
  floor.fill(Empty);
  floor[{1, 0}] = Wall;
  floor[{1, 1}] = Wall;
  auto info = DungeonMaker::labelRegions<Wall, Empty>(floor);
  if (info.numRegions() != 2)
    return false;
  if (info.regionOf[{0, 0}] != info.regionOf[{0, 1}])
    return false;
  if (info.regionOf[{2, 0}] != info.regionOf[{2, 1}])
    return false;
  if (info.regionOf[{0, 0}] == info.regionOf[{2, 0}])
    return false;
  if (info.regionOf[{1, 0}] != -1)
    return false;
  return true;
}(),
              "LabelRegions TwoRegions");

static_assert([] {
  StaticPositionArr<int> floor(0, 0);
  auto info = DungeonMaker::labelRegions<Wall, Empty>(floor);
  return info.numRegions() == 0;
}(),
              "LabelRegions ZeroSize");

// --- findCandidates tests ---

static_assert([] {
  StaticPositionArr<int> floor(3, 3);
  floor.fill(Wall);
  DungeonMaker::carveCorridor<Empty>(floor, Position{1, 1}, Position{1, 1});
  return floor[{1, 1}] == Empty && floor[{0, 0}] == Wall && floor[{2, 2}] == Wall;
}(),
              "CarveCorridor SamePoint");

static_assert([] {
  StaticPositionArr<int> floor(3, 1);
  floor.fill(Empty);
  DungeonMaker::carveCorridor<Empty>(floor, Position{0, 0}, Position{2, 0});
  for (int c = 0; c < 3; c++)
    if (floor[{c, 0}] != Empty)
      return false;
  return true;
}(),
              "CarveCorridor DoesNotOverwriteExistingEmpty");

// --- findEdges tests ---

static_assert([] {
  // All empty — no walls adjacent, so no edges
  StaticPositionArr<int> floor(3, 3);
  floor.fill(Empty);
  return DungeonMaker::findEdges<Wall, Empty>(floor).empty();
}(),
              "FindEdges AllEmpty");

static_assert([] {
  // All walls — no empty cells, so no edges
  StaticPositionArr<int> floor(3, 3);
  floor.fill(Wall);
  return DungeonMaker::findEdges<Wall, Empty>(floor).empty();
}(),
              "FindEdges AllWalls");

static_assert([] {
  // Single empty cell surrounded by walls
  // W W W
  // W E W
  // W W W
  StaticPositionArr<int> floor(3, 3);
  floor.fill(Wall);
  floor[{1, 1}] = Empty;
  auto edges = DungeonMaker::findEdges<Wall, Empty>(floor);
  return edges.size() == 1 && edges[0] == Position{1, 1};
}(),
              "FindEdges SingleEmptySurroundedByWalls");

static_assert([] {
  // E W E — two empty cells each adjacent to wall
  StaticPositionArr<int> floor(3, 1);
  floor[{0, 0}] = Empty;
  floor[{1, 0}] = Wall;
  floor[{2, 0}] = Empty;
  auto edges = DungeonMaker::findEdges<Wall, Empty>(floor);
  return edges.size() == 2;
}(),
              "FindEdges TwoEdgeCells");

static_assert([] {
  // Zero size grid
  StaticPositionArr<int> floor(0, 0);
  return DungeonMaker::findEdges<Wall, Empty>(floor).empty();
}(),
              "FindEdges ZeroSize");

// --- connectRegions integration tests ---

TEST(DungeonMakerTests, ConnectRegionsEmptyGrid) {
  StaticPositionArr<int> floor(5, 5);
  floor.fill(Empty);
  DungeonMaker::connectRegions<Wall, Empty>(floor);
  EXPECT_EQ(countRegions(floor), 1);
}

TEST(DungeonMakerTests, ConnectRegionsAllWalls) {
  StaticPositionArr<int> floor(5, 5);
  floor.fill(Wall);
  DungeonMaker::connectRegions<Wall, Empty>(floor);
  for (auto &i : floor)
    EXPECT_EQ(i, Wall);
}

TEST(DungeonMakerTests, ConnectRegionsSingleCell) {
  StaticPositionArr<int> floor(1, 1);
  floor.fill(Empty);
  DungeonMaker::connectRegions<Wall, Empty>(floor);
  EXPECT_EQ((floor[{0, 0}]), Empty);
}

TEST(DungeonMakerTests, ConnectRegionsZeroSizeWidth) {
  StaticPositionArr<int> floor(0, 5);
  DungeonMaker::connectRegions<Wall, Empty>(floor);
  EXPECT_EQ(floor.size(), 0u);
}

TEST(DungeonMakerTests, ConnectRegionsZeroSizeHeight) {
  StaticPositionArr<int> floor(5, 0);
  DungeonMaker::connectRegions<Wall, Empty>(floor);
  EXPECT_EQ(floor.size(), 0u);
}

TEST(DungeonMakerTests, ConnectRegionsTwoRegionsSeparatedByWall) {
  StaticPositionArr<int> floor(5, 5);
  floor.fill(Empty);
  for (int r = 0; r < 5; r++)
    floor[{2, r}] = Wall;
  EXPECT_EQ(countRegions(floor), 2);
  DungeonMaker::connectRegions<Wall, Empty>(floor);
  EXPECT_EQ(countRegions(floor), 1);
}

TEST(DungeonMakerTests, ConnectRegionsThreeDisconnectedRegions) {
  StaticPositionArr<int> floor(7, 7);
  floor.fill(Wall);
  for (int r = 0; r < 3; r++)
    for (int c = 0; c < 3; c++)
      floor[{c, r}] = Empty;
  for (int r = 0; r < 3; r++)
    for (int c = 4; c < 7; c++)
      floor[{c, r}] = Empty;
  for (int r = 4; r < 7; r++)
    for (int c = 0; c < 7; c++)
      floor[{c, r}] = Empty;
  EXPECT_EQ(countRegions(floor), 3);
  DungeonMaker::connectRegions<Wall, Empty>(floor);
  EXPECT_EQ(countRegions(floor), 1);
}

TEST(DungeonMakerTests, ConnectRegionsAlreadyConnected) {
  StaticPositionArr<int> floor(5, 5);
  floor.fill(Empty);
  floor[{0, 0}] = Wall;
  floor[{1, 0}] = Wall;
  int emptyCount = 0;
  for (int r = 0; r < 5; r++)
    for (int c = 0; c < 5; c++)
      if (floor[{c, r}] == Empty)
        emptyCount++;
  EXPECT_EQ(countRegions(floor), 1);
  DungeonMaker::connectRegions<Wall, Empty>(floor);
  EXPECT_EQ(countRegions(floor), 1);
  int emptyCountAfter = 0;
  for (int r = 0; r < 5; r++)
    for (int c = 0; c < 5; c++)
      if (floor[{c, r}] == Empty)
        emptyCountAfter++;
  EXPECT_EQ(emptyCount, emptyCountAfter);
}

TEST(DungeonMakerTests, ConnectRegionsSeparatedBySingleWall) {
  StaticPositionArr<int> floor({{Empty,Wall,Empty}});
  EXPECT_EQ(countRegions(floor), 2);
  DungeonMaker::connectRegions<Wall, Empty>(floor);
  EXPECT_EQ(countRegions(floor), 1);
}

TEST(DungeonMakerTests, ConnectRegionsSeparatedByDoubleWall) {
  StaticPositionArr<int> floor(4, 3);
  for (int r = 0; r < 3; r++) {
    floor[{0, r}] = Empty;
    floor[{1, r}] = Wall;
    floor[{2, r}] = Wall;
    floor[{3, r}] = Empty;
  }
  EXPECT_EQ(countRegions(floor), 2);
  DungeonMaker::connectRegions<Wall, Empty>(floor);
  EXPECT_EQ(countRegions(floor), 1);
}
