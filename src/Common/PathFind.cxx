export module Common:PathFind;
import :Location;
import :Static2DArr;
import std;

template <class T>
concept Bool2dArr = requires(const T &map, int a, int b) {
  { map.extent(a) } -> std::convertible_to<int>;
  { map[a, b] } -> std::same_as<bool>;
};
using Dy2D = std::extents<int, std::dynamic_extent, std::dynamic_extent>;
export namespace FindPath {
template <Bool2dArr MapType>
[[nodiscard]] constexpr Dir findPath(MapType map, Position start, Position goal, int maxDist = std::numeric_limits<int>::max()) {
  const int height = map.extent(0);
  const int width = map.extent(1);
  auto mapP = [&map, width, height](Position p) -> bool {
    return p.within({width - 1, height - 1}) && map[p.y, p.x];
  };
  const int initDist = Position::chessboard(goal, start);
  if (initDist <= 1) {
    return mapP(goal) ? goal - start : Dir{0, 0};
  }
  Static2DArr<std::int8_t, int> origin(height, width);
  auto originP = [&origin](Position p) -> std::int8_t & {
    return origin[p.y, p.x];
  };
  constexpr static int NoDir = -2;
  origin.fill(NoDir);
  originP(start) = -1;
  std::vector<Position> checking;
  std::vector<Position> toCheck;
  int bestDist = initDist;
  std::int8_t bestDir = -1;
  auto checkSpot = [&mapP, &originP, &toCheck, &bestDist, &bestDir, goal](Position pos, std::int8_t dir) {
    if (!mapP(pos) || originP(pos) != NoDir) {
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
  for (auto [dirN, dir] : std::views::zip(std::views::iota(0), Dir::boxDirs())) {
    Position cPos = start + dir;
    checkSpot(cPos, dirN);
  }
  for (int _ : std::views::iota(0, maxDist)) {
    swap(toCheck, checking);
    toCheck.clear();
    for (auto i : checking) {
      const std::int8_t oDir = originP(i);
      if (Position::chessboard(i, goal) == 1) {
        return Dir::getBoxDir(oDir);
      }
      for (Dir dir : Dir::boxDirs()) {
        checkSpot(i + dir, oDir);
      }
    }
    if (toCheck.empty()) {
      break;
    }
  }
  return bestDir == -1 ? Dir{0, 0} : Dir::getBoxDir(bestDir);
}
} // namespace FindPath