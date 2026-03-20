#include "TestHeader.h"
import DungeonMaker;
import Common;
import std;

constexpr int Wall = 1;
constexpr int Empty = 0;

namespace {
constexpr int countRegions(Static2DArr<int> &floor) {
  return DungeonMaker::labelRegions<Wall, Empty>(floor).numRegions();
}
} // namespace

// --- labelRegions tests ---

static_assert([] {
  Static2DArr<int> floor(3, 3);
  floor.fill(Empty);
  auto info = DungeonMaker::labelRegions<Wall, Empty>(floor);
  if (info.numRegions() != 1)
    return false;
  if (info.representatives.size() != 1u)
    return false;
  for (std::size_t r = 0; r < 3; r++)
    for (std::size_t c = 0; c < 3; c++)
      if (info.regionOf[r, c] != 0)
        return false;
  return true;
}(),
              "LabelRegions AllEmpty");

static_assert([] {
  Static2DArr<int> floor(3, 3);
  floor.fill(Wall);
  auto info = DungeonMaker::labelRegions<Wall, Empty>(floor);
  if (info.numRegions() != 0)
    return false;
  if (!info.representatives.empty())
    return false;
  for (std::size_t r = 0; r < 3; r++)
    for (std::size_t c = 0; c < 3; c++)
      if (info.regionOf[r, c] != -1)
        return false;
  return true;
}(),
              "LabelRegions AllWalls");

static_assert([] {
  // E W E
  // E W E
  Static2DArr<int> floor(2, 3);
  floor.fill(Empty);
  floor[0, 1] = Wall;
  floor[1, 1] = Wall;
  auto info = DungeonMaker::labelRegions<Wall, Empty>(floor);
  if (info.numRegions() != 2)
    return false;
  if (info.representatives.size() != 2u)
    return false;
  if (info.regionOf[0, 0] != info.regionOf[1, 0])
    return false;
  if (info.regionOf[0, 2] != info.regionOf[1, 2])
    return false;
  if (info.regionOf[0, 0] == info.regionOf[0, 2])
    return false;
  if (info.regionOf[0, 1] != -1)
    return false;
  return true;
}(),
              "LabelRegions TwoRegions");

static_assert([] {
  // E W
  // W E
  Static2DArr<int> floor(2, 2);
  floor[0, 0] = Empty;
  floor[0, 1] = Wall;
  floor[1, 0] = Wall;
  floor[1, 1] = Empty;
  auto info = DungeonMaker::labelRegions<Wall, Empty>(floor);
  return info.numRegions() == 2 && info.regionOf[0, 0] != info.regionOf[1, 1];
}(),
              "LabelRegions DiagonalNotConnected");

static_assert([] {
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
  if (info.numRegions() != 2)
    return false;
  if (info.representatives[0].x != 1 || info.representatives[0].y != 0)
    return false;
  if (info.representatives[1].x != 0 || info.representatives[1].y != 2)
    return false;
  return true;
}(),
              "LabelRegions RepresentativeIsFirstTileInRegion");

static_assert([] {
  Static2DArr<int> floor(0, 0);
  auto info = DungeonMaker::labelRegions<Wall, Empty>(floor);
  return info.numRegions() == 0 && info.representatives.empty();
}(),
              "LabelRegions ZeroSize");

// --- findCandidates tests ---

static_assert([] {
  Static2DArr<int> floor(3, 3);
  floor.fill(Empty);
  auto info = DungeonMaker::labelRegions<Wall, Empty>(floor);
  auto candidates = DungeonMaker::findCandidates<Wall>(floor, info);
  return candidates.empty();
}(),
              "FindCandidates NoCandidatesAllEmpty");

static_assert([] {
  // E W W E
  Static2DArr<int> floor(1, 4);
  floor[0, 0] = Empty;
  floor[0, 1] = Wall;
  floor[0, 2] = Wall;
  floor[0, 3] = Empty;
  auto info = DungeonMaker::labelRegions<Wall, Empty>(floor);
  if (info.numRegions() != 2)
    return false;
  auto candidates = DungeonMaker::findCandidates<Wall>(floor, info);
  return candidates.empty();
}(),
              "FindCandidates NoCandidatesThickWall");

static_assert([] {
  // E W E
  Static2DArr<int> floor(1, 3);
  floor[0, 0] = Empty;
  floor[0, 1] = Wall;
  floor[0, 2] = Empty;
  auto info = DungeonMaker::labelRegions<Wall, Empty>(floor);
  if (info.numRegions() != 2)
    return false;
  auto candidates = DungeonMaker::findCandidates<Wall>(floor, info);
  if (candidates.size() != 1u)
    return false;
  bool connectsRegions = (candidates[0].regionA == 0 && candidates[0].regionB == 1) ||
                         (candidates[0].regionA == 1 && candidates[0].regionB == 0);
  return connectsRegions;
}(),
              "FindCandidates SingleWallBetweenTwoRegions");

static_assert([] {
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
  for (std::size_t i = 1; i < candidates.size(); i++)
    if (candidates[i - 1].cost > candidates[i].cost)
      return false;
  return true;
}(),
              "FindCandidates SortedByCost");

static_assert([] {
  //   E
  // E W E
  //   W
  Static2DArr<int> floor(3, 3);
  floor.fill(Wall);
  floor[0, 1] = Empty; // region 0
  floor[1, 0] = Empty; // region 1
  floor[1, 2] = Empty; // region 2
  auto info = DungeonMaker::labelRegions<Wall, Empty>(floor);
  if (info.numRegions() != 3)
    return false;
  auto candidates = DungeonMaker::findCandidates<Wall>(floor, info);
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
  return has01 && has02 && has12;
}(),
              "FindCandidates WallAdjacentToThreeRegions");

// --- carveCorridor tests ---

static_assert([] {
  Static2DArr<int> floor(1, 5);
  floor.fill(Wall);
  DungeonMaker::carveCorridor<Empty>(floor, Position{0, 0}, Position{4, 0});
  for (std::size_t c = 0; c < 5; c++)
    if (floor[0, c] != Empty)
      return false;
  return true;
}(),
              "CarveCorridor HorizontalOnly");

static_assert([] {
  Static2DArr<int> floor(5, 1);
  floor.fill(Wall);
  DungeonMaker::carveCorridor<Empty>(floor, Position{0, 0}, Position{0, 4});
  for (std::size_t r = 0; r < 5; r++)
    if (floor[r, 0] != Empty)
      return false;
  return true;
}(),
              "CarveCorridor VerticalOnly");

static_assert([] {
  // From (0,0) to (3,2): horizontal along row 0 then vertical along col 3
  Static2DArr<int> floor(3, 4);
  floor.fill(Wall);
  DungeonMaker::carveCorridor<Empty>(floor, Position{0, 0}, Position{3, 2});
  for (std::size_t c = 0; c < 3; c++)
    if (floor[0, c] != Empty)
      return false;
  for (std::size_t r = 0; r < 3; r++)
    if (floor[r, 3] != Empty)
      return false;
  return true;
}(),
              "CarveCorridor LShapedPath");

static_assert([] {
  // From (3,2) to (0,0): walks left then up
  Static2DArr<int> floor(3, 4);
  floor.fill(Wall);
  DungeonMaker::carveCorridor<Empty>(floor, Position{3, 2}, Position{0, 0});
  for (std::size_t c = 1; c <= 3; c++)
    if (floor[2, c] != Empty)
      return false;
  for (std::size_t r = 0; r <= 2; r++)
    if (floor[r, 0] != Empty)
      return false;
  return true;
}(),
              "CarveCorridor ReverseLShaped");

static_assert([] {
  Static2DArr<int> floor(3, 3);
  floor.fill(Wall);
  DungeonMaker::carveCorridor<Empty>(floor, Position{1, 1}, Position{1, 1});
  return floor[1, 1] == Empty && floor[0, 0] == Wall && floor[2, 2] == Wall;
}(),
              "CarveCorridor SamePoint");

static_assert([] {
  Static2DArr<int> floor(1, 3);
  floor.fill(Empty);
  DungeonMaker::carveCorridor<Empty>(floor, Position{0, 0}, Position{2, 0});
  for (std::size_t c = 0; c < 3; c++)
    if (floor[0, c] != Empty)
      return false;
  return true;
}(),
              "CarveCorridor DoesNotOverwriteExistingEmpty");

// --- connectRegions integration tests ---

static_assert([] {
  Static2DArr<int> floor(5, 5);
  floor.fill(Empty);
  DungeonMaker::connectRegions<Wall, Empty>(floor);
  return countRegions(floor) == 1;
}(),
              "ConnectRegions EmptyGrid");

static_assert([] {
  Static2DArr<int> floor(5, 5);
  floor.fill(Wall);
  Static2DArr<int> original(5, 5);
  original.fill(Wall);
  DungeonMaker::connectRegions<Wall, Empty>(floor);
  for (std::size_t r = 0; r < 5; r++)
    for (std::size_t c = 0; c < 5; c++)
      if (floor[r, c] != original[r, c])
        return false;
  return true;
}(),
              "ConnectRegions AllWalls");

static_assert([] {
  Static2DArr<int> floor(1, 1);
  floor.fill(Empty);
  DungeonMaker::connectRegions<Wall, Empty>(floor);
  return floor[0, 0] == Empty;
}(),
              "ConnectRegions SingleCell");

static_assert([] {
  Static2DArr<int> floor(0, 5);
  DungeonMaker::connectRegions<Wall, Empty>(floor);
  return floor.size() == 0u;
}(),
              "ConnectRegions ZeroSizeRows");

static_assert([] {
  Static2DArr<int> floor(5, 0);
  DungeonMaker::connectRegions<Wall, Empty>(floor);
  return floor.size() == 0u;
}(),
              "ConnectRegions ZeroSizeCols");

static_assert([] {
  // 5x5 grid with a wall column in the middle
  Static2DArr<int> floor(5, 5);
  floor.fill(Empty);
  for (std::size_t r = 0; r < 5; r++)
    floor[r, 2] = Wall;
  if (countRegions(floor) != 2)
    return false;
  DungeonMaker::connectRegions<Wall, Empty>(floor);
  return countRegions(floor) == 1;
}(),
              "ConnectRegions TwoRegionsSeparatedByWall");

static_assert([] {
  // 7x7 grid with wall cross creating 3 separate regions
  Static2DArr<int> floor(7, 7);
  floor.fill(Wall);
  for (std::size_t r = 0; r < 3; r++)
    for (std::size_t c = 0; c < 3; c++)
      floor[r, c] = Empty;
  for (std::size_t r = 0; r < 3; r++)
    for (std::size_t c = 4; c < 7; c++)
      floor[r, c] = Empty;
  for (std::size_t r = 4; r < 7; r++)
    for (std::size_t c = 0; c < 7; c++)
      floor[r, c] = Empty;
  if (countRegions(floor) != 3)
    return false;
  DungeonMaker::connectRegions<Wall, Empty>(floor);
  return countRegions(floor) == 1;
}(),
              "ConnectRegions ThreeDisconnectedRegions");

static_assert([] {
  Static2DArr<int> floor(5, 5);
  floor.fill(Empty);
  floor[0, 0] = Wall;
  floor[0, 1] = Wall;
  int emptyCount = 0;
  for (std::size_t r = 0; r < 5; r++)
    for (std::size_t c = 0; c < 5; c++)
      if (floor[r, c] == Empty)
        emptyCount++;
  if (countRegions(floor) != 1)
    return false;
  DungeonMaker::connectRegions<Wall, Empty>(floor);
  if (countRegions(floor) != 1)
    return false;
  int emptyCountAfter = 0;
  for (std::size_t r = 0; r < 5; r++)
    for (std::size_t c = 0; c < 5; c++)
      if (floor[r, c] == Empty)
        emptyCountAfter++;
  return emptyCount == emptyCountAfter;
}(),
              "ConnectRegions AlreadyConnected");

static_assert([] {
  // EWE
  Static2DArr<int> floor(1, 3);
  floor[0, 0] = Empty;
  floor[0, 1] = Wall;
  floor[0, 2] = Empty;
  if (countRegions(floor) != 2)
    return false;
  DungeonMaker::connectRegions<Wall, Empty>(floor);
  return countRegions(floor) == 1;
}(),
              "ConnectRegions RegionsSeparatedBySingleWall");

static_assert([] {
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
  if (countRegions(floor) != 2)
    return false;
  DungeonMaker::connectRegions<Wall, Empty>(floor);
  return countRegions(floor) == 1;
}(),
              "ConnectRegions RegionsSeparatedByDoubleWall");
