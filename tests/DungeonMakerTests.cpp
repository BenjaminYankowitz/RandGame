#include "TestHeader.h"
import DungeonMaker;
import Common;
import GameTypes;
import std;

namespace {
constexpr int countRegions(StaticPositionArr<TerrainType> &floor) {
  return DungeonMaker::labelRegions(floor).numRegions();
}
} // namespace

// --- labelRegions tests ---

static_assert([] {
  StaticPositionArr<TerrainType> floor(3, 3);
  floor.fill(TerrainType::Empty);
  auto info = DungeonMaker::labelRegions(floor);
  if (info.numRegions() != 1)
    return false;
  return std::ranges::all_of(info.regionOf, [](auto i) { return i == 0; });
}(),
              "LabelRegions AllEmpty");

static_assert([] {
  StaticPositionArr<TerrainType> floor(3, 3);
  floor.fill(TerrainType::Wall);
  auto info = DungeonMaker::labelRegions(floor);
  if (info.numRegions() != 0)
    return false;
  return std::ranges::all_of(info.regionOf, [](auto i) { return i == -1; });
}(),
              "LabelRegions AllWalls");

static_assert([] {
  // E W E
  // E W E
  StaticPositionArr<TerrainType> floor(3, 2);
  floor.fill(TerrainType::Empty);
  floor[{1, 0}] = TerrainType::Wall;
  floor[{1, 1}] = TerrainType::Wall;
  auto info = DungeonMaker::labelRegions(floor);
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
  StaticPositionArr<TerrainType> floor(0, 0);
  auto info = DungeonMaker::labelRegions(floor);
  return info.numRegions() == 0;
}(),
              "LabelRegions ZeroSize");

// --- findCandidates tests ---

static_assert([] {
  StaticPositionArr<TerrainType> floor(3, 3);
  floor.fill(TerrainType::Wall);
  DungeonMaker::carveCorridor(floor, Position{1, 1}, Position{1, 1});
  return floor[{1, 1}] == TerrainType::Empty && floor[{0, 0}] == TerrainType::Wall && floor[{2, 2}] == TerrainType::Wall;
}(),
              "CarveCorridor SamePoint");

static_assert([] {
  StaticPositionArr<TerrainType> floor(3, 1);
  floor.fill(TerrainType::Empty);
  DungeonMaker::carveCorridor(floor, Position{0, 0}, Position{2, 0});
  for (int c = 0; c < 3; c++)
    if (floor[{c, 0}] != TerrainType::Empty)
      return false;
  return true;
}(),
              "CarveCorridor DoesNotOverwriteExistingEmpty");

// --- findEdges tests ---

static_assert([] {
  // All empty — no walls adjacent, so no edges
  StaticPositionArr<TerrainType> floor(3, 3);
  floor.fill(TerrainType::Empty);
  return DungeonMaker::findEdges(floor).empty();
}(),
              "FindEdges AllEmpty");

static_assert([] {
  // All walls — no empty cells, so no edges
  StaticPositionArr<TerrainType> floor(3, 3);
  floor.fill(TerrainType::Wall);
  return DungeonMaker::findEdges(floor).empty();
}(),
              "FindEdges AllWalls");

static_assert([] {
  // Single empty cell surrounded by walls
  // W W W
  // W E W
  // W W W
  StaticPositionArr<TerrainType> floor(3, 3);
  floor.fill(TerrainType::Wall);
  floor[{1, 1}] = TerrainType::Empty;
  auto edges = DungeonMaker::findEdges(floor);
  return edges.size() == 1 && edges[0] == Position{1, 1};
}(),
              "FindEdges SingleEmptySurroundedByWalls");

static_assert([] {
  // E W E — two empty cells each adjacent to wall
  StaticPositionArr<TerrainType> floor(3, 1);
  floor[{0, 0}] = TerrainType::Empty;
  floor[{1, 0}] = TerrainType::Wall;
  floor[{2, 0}] = TerrainType::Empty;
  auto edges = DungeonMaker::findEdges(floor);
  return edges.size() == 2;
}(),
              "FindEdges TwoEdgeCells");

static_assert([] {
  // Zero size grid
  StaticPositionArr<TerrainType> floor(0, 0);
  return DungeonMaker::findEdges(floor).empty();
}(),
              "FindEdges ZeroSize");

// --- connectRegions integration tests ---

TEST(DungeonMakerTests, ConnectRegionsEmptyGrid) {
  StaticPositionArr<TerrainType> floor(5, 5);
  floor.fill(TerrainType::Empty);
  DungeonMaker::connectRegions(floor);
  EXPECT_EQ(countRegions(floor), 1);
}

TEST(DungeonMakerTests, ConnectRegionsAllWalls) {
  StaticPositionArr<TerrainType> floor(5, 5);
  floor.fill(TerrainType::Wall);
  DungeonMaker::connectRegions(floor);
  for (auto &i : floor)
    EXPECT_EQ(i, TerrainType::Wall);
}

TEST(DungeonMakerTests, ConnectRegionsSingleCell) {
  StaticPositionArr<TerrainType> floor(1, 1);
  floor.fill(TerrainType::Empty);
  DungeonMaker::connectRegions(floor);
  EXPECT_EQ((floor[{0, 0}]), TerrainType::Empty);
}

TEST(DungeonMakerTests, ConnectRegionsZeroSizeWidth) {
  StaticPositionArr<TerrainType> floor(0, 5);
  DungeonMaker::connectRegions(floor);
  EXPECT_EQ(floor.size(), 0u);
}

TEST(DungeonMakerTests, ConnectRegionsZeroSizeHeight) {
  StaticPositionArr<TerrainType> floor(5, 0);
  DungeonMaker::connectRegions(floor);
  EXPECT_EQ(floor.size(), 0u);
}

TEST(DungeonMakerTests, ConnectRegionsTwoRegionsSeparatedByWall) {
  StaticPositionArr<TerrainType> floor(5, 5);
  floor.fill(TerrainType::Empty);
  for (int r = 0; r < 5; r++)
    floor[{2, r}] = TerrainType::Wall;
  EXPECT_EQ(countRegions(floor), 2);
  DungeonMaker::connectRegions(floor);
  EXPECT_EQ(countRegions(floor), 1);
}

TEST(DungeonMakerTests, ConnectRegionsThreeDisconnectedRegions) {
  StaticPositionArr<TerrainType> floor(7, 7);
  floor.fill(TerrainType::Wall);
  for (int r = 0; r < 3; r++)
    for (int c = 0; c < 3; c++)
      floor[{c, r}] = TerrainType::Empty;
  for (int r = 0; r < 3; r++)
    for (int c = 4; c < 7; c++)
      floor[{c, r}] = TerrainType::Empty;
  for (int r = 4; r < 7; r++)
    for (int c = 0; c < 7; c++)
      floor[{c, r}] = TerrainType::Empty;
  EXPECT_EQ(countRegions(floor), 3);
  DungeonMaker::connectRegions(floor);
  EXPECT_EQ(countRegions(floor), 1);
}

TEST(DungeonMakerTests, ConnectRegionsAlreadyConnected) {
  StaticPositionArr<TerrainType> floor(5, 5);
  floor.fill(TerrainType::Empty);
  floor[{0, 0}] = TerrainType::Wall;
  floor[{1, 0}] = TerrainType::Wall;
  int emptyCount = 0;
  for (int r = 0; r < 5; r++)
    for (int c = 0; c < 5; c++)
      if (floor[{c, r}] == TerrainType::Empty)
        emptyCount++;
  EXPECT_EQ(countRegions(floor), 1);
  DungeonMaker::connectRegions(floor);
  EXPECT_EQ(countRegions(floor), 1);
  int emptyCountAfter = 0;
  for (int r = 0; r < 5; r++)
    for (int c = 0; c < 5; c++)
      if (floor[{c, r}] == TerrainType::Empty)
        emptyCountAfter++;
  EXPECT_EQ(emptyCount, emptyCountAfter);
}

TEST(DungeonMakerTests, ConnectRegionsSeparatedBySingleWall) {
  StaticPositionArr<TerrainType> floor({{TerrainType::Empty, TerrainType::Wall, TerrainType::Empty}});
  EXPECT_EQ(countRegions(floor), 2);
  DungeonMaker::connectRegions(floor);
  EXPECT_EQ(countRegions(floor), 1);
}

TEST(DungeonMakerTests, ConnectRegionsSeparatedByDoubleWall) {
  StaticPositionArr<TerrainType> floor(4, 3);
  for (int r = 0; r < 3; r++) {
    floor[{0, r}] = TerrainType::Empty;
    floor[{1, r}] = TerrainType::Wall;
    floor[{2, r}] = TerrainType::Wall;
    floor[{3, r}] = TerrainType::Empty;
  }
  EXPECT_EQ(countRegions(floor), 2);
  DungeonMaker::connectRegions(floor);
  EXPECT_EQ(countRegions(floor), 1);
}
