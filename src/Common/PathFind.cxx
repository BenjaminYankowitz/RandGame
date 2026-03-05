export module Common:PathFind;
import :Location;
import :Static2DArr;
import std;

using Dy2D = std::extents<int, std::dynamic_extent, std::dynamic_extent>;
export namespace FindPath {
template <class LayoutPolicy, class AccessorPolicy>
[[nodiscard]] constexpr Dir findPath(std::mdspan<bool, Dy2D, LayoutPolicy, AccessorPolicy> map, Position start, Position goal, int maxDist = std::numeric_limits<int>::max()) {
  const int height = map.extent(0);
  const int width = map.extent(1);
  auto mapP = [&map, width, height](Position p) -> bool {
    return p.within({width-1,height-1}) && map[p.y, p.x];
  };
  if (Position::chessboard(goal, start) <= 1) {
    return mapP(goal) ? goal - start : Dir{0, 0};
  }
  Static2DArr<std::int8_t> origin(height, width);
  auto originP = [&origin](Position p) -> std::int8_t & {
    return origin[p.y, p.x];
  };
  constexpr int DirSent = 8;
  origin.fill(DirSent);
  originP(start) = -1;
  std::vector<Position> checking;
  std::vector<Position> toCheck;
  int bestDist = Position::chessboard(start, goal);
  std::int8_t bestDir = -1;
  auto checkSpot = [&mapP, &originP, &toCheck, &bestDist, &bestDir, goal](Position pos, std::int8_t dir) {
    if (!mapP(pos) || originP(pos) != DirSent) {
      return;
    }
    const int dist = Position::chessboard(pos, goal);
    originP(pos) = dir;
    toCheck.push_back(pos);
    if (dist < bestDist) {
      bestDist = dist;
      bestDir = dir;
    }
  };
  std::ranges::for_each(std::views::iota(0, DirSent), [&checkSpot, start](std::int8_t dirN) {
    Dir dir = Dir::getBoxDir(dirN);
    Position cPos = start + dir;
    checkSpot(cPos, dirN);
  });
  for (int dist = 0; dist < maxDist; dist++) {
    swap(toCheck, checking);
    toCheck.clear();
    for (auto i : checking) {
      const std::int8_t oDir = originP(i);
      if (Position::chessboard(i, goal) == 1) {
        return Dir::getBoxDir(oDir);
      }
      std::ranges::for_each(Dir::boxDirs(), [&checkSpot, i, oDir](Dir dir) {
        Position cPos = i + dir;
        checkSpot(cPos, oDir);
      });
    }
    if (toCheck.empty()) {
      break;
    }
  }
  return bestDir == -1 ? Dir{0,0} : Dir::getBoxDir(bestDir);
}
} // namespace FindPath