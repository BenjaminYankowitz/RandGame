#include "TestHeader.h"
import DungeonMaker;
import Common;
import std;


constexpr int Wall = 1;
constexpr int Empty = 0;


static int countRegions(Static2DArr<int> &floor) {
  return DungeonMaker::labelRegions<Wall,Empty>(floor).numRegions();
}


// --- labelRegions tests ---

TEST(LabelRegions, AllEmpty) {
  Static2DArr<int> floor(3, 3);
  floor.fill(Empty);
  auto info = DungeonMaker::labelRegions<Wall, Empty>(floor);
  EXPECT_EQ(info.numRegions(), 1);
  EXPECT_EQ(info.representatives.size(), 1u);
  // All tiles belong to region 0
  for (std::size_t r = 0; r < 3; r++)
    for (std::size_t c = 0; c < 3; c++)
      EXPECT_EQ((info.regionOf[r, c]), 0);
}

TEST(LabelRegions, AllWalls) {
  Static2DArr<int> floor(3, 3);
  floor.fill(Wall);
  auto info = DungeonMaker::labelRegions<Wall, Empty>(floor);
  EXPECT_EQ(info.numRegions(), 0);
  EXPECT_TRUE(info.representatives.empty());
  for (std::size_t r = 0; r < 3; r++)
    for (std::size_t c = 0; c < 3; c++)
      EXPECT_EQ((info.regionOf[r, c]), -1);
}

TEST(LabelRegions, TwoRegions) {
  // E W E
  // E W E
  Static2DArr<int> floor(2, 3);
  floor.fill(Empty);
  floor[0, 1] = Wall;
  floor[1, 1] = Wall;
  auto info = DungeonMaker::labelRegions<Wall, Empty>(floor);
  EXPECT_EQ(info.numRegions(), 2);
  EXPECT_EQ(info.representatives.size(), 2u);
  // Left region tiles share one id, right region shares another
  EXPECT_EQ((info.regionOf[0, 0]), (info.regionOf[1, 0]));
  EXPECT_EQ((info.regionOf[0, 2]), (info.regionOf[1, 2]));
  EXPECT_NE((info.regionOf[0, 0]), (info.regionOf[0, 2]));
  // Wall tiles are unlabeled
  EXPECT_EQ((info.regionOf[0, 1]), -1);
}

TEST(LabelRegions, DiagonalNotConnected) {
  // E W
  // W E
  Static2DArr<int> floor(2, 2);
  floor[0, 0] = Empty;
  floor[0, 1] = Wall;
  floor[1, 0] = Wall;
  floor[1, 1] = Empty;
  auto info = DungeonMaker::labelRegions<Wall, Empty>(floor);
  EXPECT_EQ(info.numRegions(), 2);
  EXPECT_NE((info.regionOf[0, 0]), (info.regionOf[1, 1]));
}

TEST(LabelRegions, RepresentativeIsFirstTileInRegion) {
  // W E E
  // W W W
  // E E W
  Static2DArr<int> floor(3, 3);
  floor.fill(Wall);
  floor[0, 1] = Empty;
  floor[0, 2] = Empty;
  floor[2, 0] = Empty;
  floor[2, 1] = Empty;
  auto info = DungeonMaker::labelRegions<Wall, Empty>(floor);
  EXPECT_EQ(info.numRegions(), 2);
  // First region scanned is (r=0,c=1) -> Position(x=1,y=0)
  EXPECT_EQ(info.representatives[0].x, 1);
  EXPECT_EQ(info.representatives[0].y, 0);
  // Second region scanned is (r=2,c=0) -> Position(x=0,y=2)
  EXPECT_EQ(info.representatives[1].x, 0);
  EXPECT_EQ(info.representatives[1].y, 2);
}

TEST(LabelRegions, ZeroSize) {
  Static2DArr<int> floor(0, 0);
  auto info = DungeonMaker::labelRegions<Wall, Empty>(floor);
  EXPECT_EQ(info.numRegions(), 0);
  EXPECT_TRUE(info.representatives.empty());
}

// --- findCandidates tests ---

TEST(FindCandidates, NoCandidatesAllEmpty) {
  Static2DArr<int> floor(3, 3);
  floor.fill(Empty);
  auto info = DungeonMaker::labelRegions<Wall, Empty>(floor);
  auto candidates = DungeonMaker::findCandidates<Wall>(floor, info);
  EXPECT_TRUE(candidates.empty());
}

TEST(FindCandidates, NoCandidatesThickWall) {
  // E W W E
  Static2DArr<int> floor(1, 4);
  floor[0, 0] = Empty;
  floor[0, 1] = Wall;
  floor[0, 2] = Wall;
  floor[0, 3] = Empty;
  auto info = DungeonMaker::labelRegions<Wall, Empty>(floor);
  EXPECT_EQ(info.numRegions(), 2);
  auto candidates = DungeonMaker::findCandidates<Wall>(floor, info);
  EXPECT_TRUE(candidates.empty());
}

TEST(FindCandidates, SingleWallBetweenTwoRegions) {
  // E W E
  Static2DArr<int> floor(1, 3);
  floor[0, 0] = Empty;
  floor[0, 1] = Wall;
  floor[0, 2] = Empty;
  auto info = DungeonMaker::labelRegions<Wall, Empty>(floor);
  EXPECT_EQ(info.numRegions(), 2);
  auto candidates = DungeonMaker::findCandidates<Wall>(floor, info);
  EXPECT_EQ(candidates.size(), 1u);
  // Should connect the two regions
  bool connectsRegions = (candidates[0].regionA == 0 && candidates[0].regionB == 1) ||
                         (candidates[0].regionA == 1 && candidates[0].regionB == 0);
  EXPECT_TRUE(connectsRegions);
}

TEST(FindCandidates, SortedByCost) {
  // Two wall tiles each adjacent to 2 regions, with different costs
  // Region 0 top-left, Region 1 top-right, Region 2 bottom
  // E E W E E
  // W W W W W
  // E E E E E
  Static2DArr<int> floor(3, 5);
  floor.fill(Empty);
  floor[0, 2] = Wall;
  for (std::size_t c = 0; c < 5; c++)
    floor[1, c] = Wall;
  auto info = DungeonMaker::labelRegions<Wall, Empty>(floor);
  auto candidates = DungeonMaker::findCandidates<Wall>(floor, info);
  // Verify sorted by cost
  for (std::size_t i = 1; i < candidates.size(); i++)
    EXPECT_LE(candidates[i - 1].cost, candidates[i].cost);
}

TEST(FindCandidates, WallAdjacentToThreeRegions) {
  // Center wall [1,1] touches all 3 regions -> 3 pairs from that tile.
  // Other wall tiles may also produce candidates.
  //   E
  // E W E
  //   W
  Static2DArr<int> floor(3, 3);
  floor.fill(Wall);
  floor[0, 1] = Empty; // region 0
  floor[1, 0] = Empty; // region 1
  floor[1, 2] = Empty; // region 2
  auto info = DungeonMaker::labelRegions<Wall, Empty>(floor);
  EXPECT_EQ(info.numRegions(), 3);
  auto candidates = DungeonMaker::findCandidates<Wall>(floor, info);
  // At minimum, all 3 region pairs must appear
  bool has01 = false;
  bool has02 = false;
  bool has12 = false;
  for (const auto &c : candidates) {
    int lo = std::min(c.regionA, c.regionB);
    int hi = std::max(c.regionA, c.regionB);
    if (lo == 0 && hi == 1)
      has01 = true;
    if (lo == 0 && hi == 2)
      has02 = true;
    if (lo == 1 && hi == 2)
      has12 = true;
  }
  EXPECT_TRUE(has01);
  EXPECT_TRUE(has02);
  EXPECT_TRUE(has12);
}

// --- carveCorridor tests ---

TEST(CarveCorridor, HorizontalOnly) {
  Static2DArr<int> floor(1, 5);
  floor.fill(Wall);
  DungeonMaker::carveCorridor<Empty>(floor, Position{0, 0}, Position{4, 0});
  for (std::size_t c = 0; c < 5; c++)
    EXPECT_EQ((floor[0, c]), Empty);
}

TEST(CarveCorridor, VerticalOnly) {
  Static2DArr<int> floor(5, 1);
  floor.fill(Wall);
  DungeonMaker::carveCorridor<Empty>(floor, Position{0, 0}, Position{0, 4});
  for (std::size_t r = 0; r < 5; r++)
    EXPECT_EQ((floor[r, 0]), Empty);
}

TEST(CarveCorridor, LShapedPath) {
  // From (0,0) to (3,2): horizontal along row 0 then vertical along col 3
  Static2DArr<int> floor(3, 4);
  floor.fill(Wall);
  DungeonMaker::carveCorridor<Empty>(floor, Position{0, 0}, Position{3, 2});
  // Horizontal leg: row 0, cols 0-2
  for (std::size_t c = 0; c < 3; c++)
    EXPECT_EQ((floor[0, c]), Empty);
  // Vertical leg: col 3, rows 0-2
  for (std::size_t r = 0; r < 3; r++)
    EXPECT_EQ((floor[r, 3]), Empty);
}

TEST(CarveCorridor, ReverseLShaped) {
  // From (3,2) to (0,0): walks left then up
  Static2DArr<int> floor(3, 4);
  floor.fill(Wall);
  DungeonMaker::carveCorridor<Empty>(floor, Position{3, 2}, Position{0, 0});
  // Horizontal leg: row 2, cols 1-3
  for (std::size_t c = 1; c <= 3; c++)
    EXPECT_EQ((floor[2, c]), Empty);
  // Vertical leg: col 0, rows 0-2
  for (std::size_t r = 0; r <= 2; r++)
    EXPECT_EQ((floor[r, 0]), Empty);
}

TEST(CarveCorridor, SamePoint) {
  Static2DArr<int> floor(3, 3);
  floor.fill(Wall);
  DungeonMaker::carveCorridor<Empty>(floor, Position{1, 1}, Position{1, 1});
  EXPECT_EQ((floor[1, 1]), Empty);
  // Only the target tile should be carved
  EXPECT_EQ((floor[0, 0]), Wall);
  EXPECT_EQ((floor[2, 2]), Wall);
}

TEST(CarveCorridor, DoesNotOverwriteExistingEmpty) {
  Static2DArr<int> floor(1, 3);
  floor.fill(Empty);
  DungeonMaker::carveCorridor<Empty>(floor, Position{0, 0}, Position{2, 0});
  for (std::size_t c = 0; c < 3; c++)
    EXPECT_EQ((floor[0, c]), Empty);
}

// --- connectRegions integration tests ---

TEST(DungeonMakerTests, EmptyGrid) {
  Static2DArr<int> floor(5, 5);
  floor.fill(Empty);
  DungeonMaker::connectRegions<Wall, Empty>(floor);
  EXPECT_EQ(countRegions(floor), 1);
}

TEST(DungeonMakerTests, AllWalls) {
  Static2DArr<int> floor(5, 5);
  floor.fill(Wall);
  Static2DArr<int> original(5, 5);
  original.fill(Wall);
  DungeonMaker::connectRegions<Wall, Empty>(floor);
  for (std::size_t r = 0; r < 5; r++)
    for (std::size_t c = 0; c < 5; c++)
      EXPECT_EQ((floor[r, c]), (original[r, c]));
}

TEST(DungeonMakerTests, SingleCell) {
  Static2DArr<int> floor(1, 1);
  floor.fill(Empty);
  DungeonMaker::connectRegions<Wall, Empty>(floor);
  EXPECT_EQ((floor[0, 0]), Empty);
}

TEST(DungeonMakerTests, ZeroSizeRows) {
  Static2DArr<int> floor(0, 5);
  DungeonMaker::connectRegions<Wall, Empty>(floor);
  EXPECT_EQ(floor.size(), 0u);
}

TEST(DungeonMakerTests, ZeroSizeCols) {
  Static2DArr<int> floor(5, 0);
  DungeonMaker::connectRegions<Wall, Empty>(floor);
  EXPECT_EQ(floor.size(), 0u);
}

TEST(DungeonMakerTests, TwoRegionsSeparatedByWall) {
  // 5x5 grid with a wall column in the middle
  // EEW EE
  // EEW EE
  // EEW EE
  // EEW EE
  // EEW EE
  Static2DArr<int> floor(5, 5);
  floor.fill(Empty);
  for (std::size_t r = 0; r < 5; r++)
    floor[r, 2] = Wall;
  EXPECT_EQ(countRegions(floor), 2);
  DungeonMaker::connectRegions<Wall, Empty>(floor);
  EXPECT_EQ(countRegions(floor), 1);
}

TEST(DungeonMakerTests, ThreeDisconnectedRegions) {
  // 7x7 grid with wall cross creating 3 separate regions
  // Top-left, top-right, and bottom
  Static2DArr<int> floor(7, 7);
  floor.fill(Wall);
  // Region 1: top-left
  for (std::size_t r = 0; r < 3; r++)
    for (std::size_t c = 0; c < 3; c++)
      floor[r, c] = Empty;
  // Region 2: top-right
  for (std::size_t r = 0; r < 3; r++)
    for (std::size_t c = 4; c < 7; c++)
      floor[r, c] = Empty;
  // Region 3: bottom
  for (std::size_t r = 4; r < 7; r++)
    for (std::size_t c = 0; c < 7; c++)
      floor[r, c] = Empty;

  EXPECT_EQ(countRegions(floor), 3);
  DungeonMaker::connectRegions<Wall, Empty>(floor);
  EXPECT_EQ(countRegions(floor), 1);
}

TEST(DungeonMakerTests, AlreadyConnected) {
  Static2DArr<int> floor(5, 5);
  floor.fill(Empty);
  // Add some walls but keep everything connected
  floor[0, 0] = Wall;
  floor[0, 1] = Wall;

  int emptyCount = 0;
  for (std::size_t r = 0; r < 5; r++)
    for (std::size_t c = 0; c < 5; c++)
      if (floor[r, c] == Empty)
        emptyCount++;

  EXPECT_EQ(countRegions(floor), 1);
  DungeonMaker::connectRegions<Wall, Empty>(floor);
  EXPECT_EQ(countRegions(floor), 1);

  // No walls should have been carved
  int emptyCountAfter = 0;
  for (std::size_t r = 0; r < 5; r++)
    for (std::size_t c = 0; c < 5; c++)
      if (floor[r, c] == Empty)
        emptyCountAfter++;
  EXPECT_EQ(emptyCount, emptyCountAfter);
}

TEST(DungeonMakerTests, RegionsSeparatedBySingleWall) {
  // Two regions separated by a single wall tile
  // EWE
  Static2DArr<int> floor(1, 3);
  floor[0, 0] = Empty;
  floor[0, 1] = Wall;
  floor[0, 2] = Empty;

  EXPECT_EQ(countRegions(floor), 2);
  DungeonMaker::connectRegions<Wall, Empty>(floor);
  EXPECT_EQ(countRegions(floor), 1);
}

TEST(DungeonMakerTests, RegionsSeparatedByDoubleWall) {
  // Two regions separated by a 2-thick wall column.
  // No single wall tile is adjacent to both regions,
  // so connectRegions fails to generate any candidates
  // and leaves them disconnected.
  //
  // E W W E
  // E W W E
  // E W W E
  Static2DArr<int> floor(3, 4);
  for (std::size_t r = 0; r < 3; r++) {
    floor[r, 0] = Empty;
    floor[r, 1] = Wall;
    floor[r, 2] = Wall;
    floor[r, 3] = Empty;
  }

  EXPECT_EQ(countRegions(floor), 2);
  DungeonMaker::connectRegions<Wall, Empty>(floor);
  EXPECT_EQ(countRegions(floor), 1);
}
