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

TEST(DungeonMakerTests, ConnectRegions1x1Wall) {
  StaticPositionArr<TerrainType> floor(1, 1);
  floor.fill(TerrainType::Wall);
  DungeonMaker::connectRegions(floor);
  EXPECT_EQ((floor[{0, 0}]), TerrainType::Wall);
}

// --- labelRegions diagonal connectivity ---

static_assert([] {
  // E . E    Two empty cells connected only diagonally
  // . . .    boxDirs includes diagonals, so this should be 1 region
  // . . E
  StaticPositionArr<TerrainType> floor(3, 3);
  floor.fill(TerrainType::Wall);
  floor[{0, 0}] = TerrainType::Empty;
  floor[{1, 1}] = TerrainType::Empty;
  auto info = DungeonMaker::labelRegions(floor);
  return info.numRegions() == 1;
}(),
              "LabelRegions DiagonallyConnected");

static_assert([] {
  // Two empty cells not connected at all (too far apart)
  StaticPositionArr<TerrainType> floor(3, 3);
  floor.fill(TerrainType::Wall);
  floor[{0, 0}] = TerrainType::Empty;
  floor[{2, 2}] = TerrainType::Empty;
  auto info = DungeonMaker::labelRegions(floor);
  return info.numRegions() == 2;
}(),
              "LabelRegions DiagonallyDisconnected");

// --- carveHVCorridor tests ---

TEST(DungeonMakerTests, CarveHVCorridorSamePoint) {
  StaticPositionArr<TerrainType> floor(5, 5);
  floor.fill(TerrainType::Wall);
  DungeonMaker::carveHVCorridor(floor, Position{2, 2}, Position{2, 2});
  EXPECT_EQ((floor[{2, 2}]), TerrainType::Empty);
  // Neighbors should still be walls
  EXPECT_EQ((floor[{1, 2}]), TerrainType::Wall);
  EXPECT_EQ((floor[{3, 2}]), TerrainType::Wall);
  EXPECT_EQ((floor[{2, 1}]), TerrainType::Wall);
  EXPECT_EQ((floor[{2, 3}]), TerrainType::Wall);
}

TEST(DungeonMakerTests, CarveHVCorridorHorizontal) {
  StaticPositionArr<TerrainType> floor(5, 5);
  floor.fill(TerrainType::Wall);
  DungeonMaker::carveHVCorridor(floor, Position{1, 2}, Position{3, 2});
  EXPECT_EQ((floor[{1, 2}]), TerrainType::Empty);
  EXPECT_EQ((floor[{2, 2}]), TerrainType::Empty);
  EXPECT_EQ((floor[{3, 2}]), TerrainType::Empty);
  // Above and below should still be walls
  EXPECT_EQ((floor[{2, 1}]), TerrainType::Wall);
  EXPECT_EQ((floor[{2, 3}]), TerrainType::Wall);
}

TEST(DungeonMakerTests, CarveHVCorridorVertical) {
  StaticPositionArr<TerrainType> floor(5, 5);
  floor.fill(TerrainType::Wall);
  DungeonMaker::carveHVCorridor(floor, Position{2, 1}, Position{2, 3});
  EXPECT_EQ((floor[{2, 1}]), TerrainType::Empty);
  EXPECT_EQ((floor[{2, 2}]), TerrainType::Empty);
  EXPECT_EQ((floor[{2, 3}]), TerrainType::Empty);
  EXPECT_EQ((floor[{1, 2}]), TerrainType::Wall);
  EXPECT_EQ((floor[{3, 2}]), TerrainType::Wall);
}

TEST(DungeonMakerTests, CarveHVCorridorLShaped) {
  StaticPositionArr<TerrainType> floor(5, 5);
  floor.fill(TerrainType::Wall);
  DungeonMaker::carveHVCorridor(floor, Position{0, 0}, Position{3, 2});
  // Horizontal segment: (0,0) to (3,0)
  EXPECT_EQ((floor[{0, 0}]), TerrainType::Empty);
  EXPECT_EQ((floor[{1, 0}]), TerrainType::Empty);
  EXPECT_EQ((floor[{2, 0}]), TerrainType::Empty);
  EXPECT_EQ((floor[{3, 0}]), TerrainType::Empty);
  // Vertical segment: (3,0) to (3,2)
  EXPECT_EQ((floor[{3, 1}]), TerrainType::Empty);
  EXPECT_EQ((floor[{3, 2}]), TerrainType::Empty);
  // Off the L should be wall
  EXPECT_EQ((floor[{0, 1}]), TerrainType::Wall);
  EXPECT_EQ((floor[{1, 1}]), TerrainType::Wall);
}

TEST(DungeonMakerTests, CarveHVCorridorReverse) {
  StaticPositionArr<TerrainType> floor(5, 5);
  floor.fill(TerrainType::Wall);
  DungeonMaker::carveHVCorridor(floor, Position{3, 3}, Position{1, 1});
  // Horizontal segment: (3,3) to (1,3)
  EXPECT_EQ((floor[{3, 3}]), TerrainType::Empty);
  EXPECT_EQ((floor[{2, 3}]), TerrainType::Empty);
  EXPECT_EQ((floor[{1, 3}]), TerrainType::Empty);
  // Vertical segment: (1,3) to (1,1)
  EXPECT_EQ((floor[{1, 2}]), TerrainType::Empty);
  EXPECT_EQ((floor[{1, 1}]), TerrainType::Empty);
}

// --- randomRooms tests ---

TEST(DungeonMakerTests, RandomRoomsSmokeTest) {
  StaticPositionArr<TerrainType> floor(40, 30);
  DungeonMaker::randomRooms(floor, Position{5, 5}, Position{35, 25});
  // Should produce a connected map
  // Set stairs to Empty for region counting
  floor[{5, 5}] = TerrainType::Empty;
  floor[{35, 25}] = TerrainType::Empty;
  EXPECT_EQ(countRegions(floor), 1);
}

TEST(DungeonMakerTests, RandomRoomsStairsAtCorners) {
  StaticPositionArr<TerrainType> floor(30, 30);
  Position up{0, 0};
  Position down{29, 29};
  DungeonMaker::randomRooms(floor, up, down);
  EXPECT_EQ(floor[up], TerrainType::UpStair);
  EXPECT_EQ(floor[down], TerrainType::DownStair);
}

TEST(DungeonMakerTests, RandomRoomsStairsAtEdges) {
  StaticPositionArr<TerrainType> floor(30, 30);
  Position up{0, 15};
  Position down{29, 15};
  DungeonMaker::randomRooms(floor, up, down);
  EXPECT_EQ(floor[up], TerrainType::UpStair);
  EXPECT_EQ(floor[down], TerrainType::DownStair);
}

TEST(DungeonMakerTests, RandomRoomsStairsAtCenter) {
  StaticPositionArr<TerrainType> floor(30, 30);
  Position up{15, 15};
  Position down{10, 10};
  DungeonMaker::randomRooms(floor, up, down);
  EXPECT_EQ(floor[up], TerrainType::UpStair);
  EXPECT_EQ(floor[down], TerrainType::DownStair);
}

TEST(DungeonMakerTests, RandomRoomsSmallFloor) {
  StaticPositionArr<TerrainType> floor(3, 3);
  DungeonMaker::randomRooms(floor, Position{1, 1}, Position{2, 2});
  // Should not crash. Stairs may or may not fit.
  // Just verify no wall-only floor (some empty cells exist)
  bool hasEmpty = false;
  for (auto &t : floor) {
    if (t == TerrainType::Empty || t == TerrainType::UpStair || t == TerrainType::DownStair)
      hasEmpty = true;
  }
  EXPECT_TRUE(hasEmpty);
}

TEST(DungeonMakerTests, RandomRoomsStairsOutOfBounds) {
  StaticPositionArr<TerrainType> floor(20, 20);
  DungeonMaker::randomRooms(floor, Position{-1, -1}, Position{100, 100});
  // Should not crash, stairs out of bounds are skipped
  bool hasEmpty = false;
  for (auto &t : floor) {
    if (t == TerrainType::Empty)
      hasEmpty = true;
  }
  EXPECT_TRUE(hasEmpty);
}

// --- openSimplex wrapper tests ---

TEST(DungeonMakerTests, OpenSimplexPlacesStairs) {
  StaticPositionArr<TerrainType> floor(30, 30);
  Position up{5, 5};
  Position down{25, 25};
  DungeonMaker::openSimplex(floor, up, down, 32, 8, -0.2);
  EXPECT_EQ(floor[up], TerrainType::UpStair);
  EXPECT_EQ(floor[down], TerrainType::DownStair);
}

TEST(DungeonMakerTests, OpenSimplexSameStairPosition) {
  StaticPositionArr<TerrainType> floor(20, 20);
  Position both{10, 10};
  DungeonMaker::openSimplex(floor, both, both, 32, 8, -0.2);
  // DownStair is set last, so it wins
  EXPECT_EQ(floor[both], TerrainType::DownStair);
}

// --- maze tests ---

TEST(DungeonMakerTests, MazeSquareGrid) {
  StaticPositionArr<TerrainType> floor(11, 11);
  DungeonMaker::maze(floor);
  // Every other cell should be empty in the initial grid
  // After maze generation, the maze should be connected
  DungeonMaker::connectRegions(floor);
  EXPECT_EQ(countRegions(floor), 1);
}

TEST(DungeonMakerTests, MazeNonSquareGrid) {
  // This tests the Position{row, col} bug — width != height
  // Position(x,y) has x=col, y=row, but maze passes Position{row, col}
  // On non-square grids this may access out of bounds
  StaticPositionArr<TerrainType> floor(15, 7);
  DungeonMaker::maze(floor);
  // If we get here without crashing, check basic properties
  bool hasEmpty = false;
  bool hasWall = false;
  for (auto &t : floor) {
    if (t == TerrainType::Empty)
      hasEmpty = true;
    if (t == TerrainType::Wall)
      hasWall = true;
  }
  EXPECT_TRUE(hasEmpty);
  EXPECT_TRUE(hasWall);
}

TEST(DungeonMakerTests, MazeEmptyGrid) {
  StaticPositionArr<TerrainType> floor(0, 0);
  DungeonMaker::maze(floor);
  EXPECT_EQ(floor.size(), 0u);
}

TEST(DungeonMakerTests, Maze1x1) {
  StaticPositionArr<TerrainType> floor(1, 1);
  DungeonMaker::maze(floor);
  EXPECT_EQ((floor[{0, 0}]), TerrainType::Empty);
}

// --- perlin / openSimplexRaw smoke tests ---

TEST(DungeonMakerTests, PerlinSmokeTest) {
  StaticPositionArr<TerrainType> floor(20, 20);
  DungeonMaker::perlin(floor, 8, 8, 0.0);
  bool hasWall = false;
  bool hasEmpty = false;
  for (auto &t : floor) {
    if (t == TerrainType::Wall)
      hasWall = true;
    if (t == TerrainType::Empty)
      hasEmpty = true;
  }
  // With threshold 0.0, should have both wall and empty
  EXPECT_TRUE(hasWall);
  EXPECT_TRUE(hasEmpty);
}

TEST(DungeonMakerTests, OpenSimplexRawSmokeTest) {
  StaticPositionArr<TerrainType> floor(20, 20);
  DungeonMaker::openSimplexRaw(floor, 8, 8, 0.0);
  bool hasWall = false;
  bool hasEmpty = false;
  for (auto &t : floor) {
    if (t == TerrainType::Wall)
      hasWall = true;
    if (t == TerrainType::Empty)
      hasEmpty = true;
  }
  EXPECT_TRUE(hasWall);
  EXPECT_TRUE(hasEmpty);
}

TEST(DungeonMakerTests, PerlinFillsAllCells) {
  StaticPositionArr<TerrainType> floor(10, 10);
  floor.fill(TerrainType::UpStair); // Fill with non-Wall/Empty to verify overwrite
  DungeonMaker::perlin(floor, 8, 8, 0.0);
  for (auto &t : floor) {
    EXPECT_TRUE(t == TerrainType::Wall || t == TerrainType::Empty);
  }
}

TEST(DungeonMakerTests, OpenSimplexRawFillsAllCells) {
  StaticPositionArr<TerrainType> floor(10, 10);
  floor.fill(TerrainType::UpStair);
  DungeonMaker::openSimplexRaw(floor, 8, 8, 0.0);
  for (auto &t : floor) {
    EXPECT_TRUE(t == TerrainType::Wall || t == TerrainType::Empty);
  }
}
