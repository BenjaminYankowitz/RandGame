module;
#include <cassert>
export module Common:LineOfSight;
import :Location;
import std;

template <class T>
concept SeeThrough2dArr = requires(const T &map, int n, Position p) {
  { map.extent(n) } -> std::integral;
  { map[p] } -> std::same_as<bool>;
};

template <SeeThrough2dArr MapType>
struct MapWrap {
  [[nodiscard]] bool operator[](Dir p) const noexcept {
    auto [x, y] = p;
    if (swapXY)
      std::swap(x, y);
    if (flipX)
      x = -x;
    if (flipY)
      y = -y;
    return impl[offset + Dir{x, y}];
  }
  const MapType &impl;
  Position offset;
  bool flipX;
  bool flipY;
  bool swapXY;
};

struct Corner {
  int x;
  int y;
};

[[nodiscard]] constexpr int getYat(Corner s, int x) noexcept {
  std::int64_t xV = x;
  std::int64_t sy = s.y;
  std::int64_t sx = s.x;
  return (((2 * xV * ((2 * sy) - 1)) / ((2 * sx) - 1)) + 1) / 2;
}

[[nodiscard]] constexpr auto slopeCMP(Corner s1, Corner s2) {
  return ((2 * s2.y) - 1) * ((2 * s1.x) - 1) <=> ((2 * s1.y) - 1) * ((2 * s2.x) - 1);
}

[[nodiscard]] constexpr auto slopeEQ(Corner s1, Corner s2) {
  return slopeCMP(s1, s2) == 0;
}

[[nodiscard]] constexpr auto slopeLT(Corner s1, Corner s2) {
  return slopeCMP(s1, s2) < 0;
}

[[nodiscard]] constexpr auto slopeGT(Corner s1, Corner s2) {
  return slopeCMP(s1, s2) > 0;
}

[[nodiscard]] constexpr auto slopeLE(Corner s1, Corner s2) {
  return slopeCMP(s1, s2) <= 0;
}

[[nodiscard]] constexpr auto slopeGE(Corner s1, Corner s2) {
  return slopeCMP(s1, s2) >= 0;
}

[[nodiscard]] constexpr Dir nextSpot(Dir d, Corner s) noexcept {
  std::int64_t dx = d.dx;
  std::int64_t dy = d.dy;
  std::int64_t sx = s.x;
  std::int64_t sy = s.y;
  auto cmp = ((2 * dx) + 1) * ((2 * sy) - 1) <=> ((dy * 2) + 1) * ((2 * sx) - 1);
  Dir ret = d;
  if (cmp >= 0)
    ret += Dir{0, 1};
  if (cmp <= 0)
    ret += Dir{1, 0};
  return ret;
}

static_assert(slopeCMP({2, 2}, {2, 2}) == 0);
static_assert(slopeCMP({2, 2}, {2, 1}) < 0);
static_assert(slopeLE({2, 2}, {2, 1}));

template <bool min, SeeThrough2dArr MapType>
[[nodiscard]] constexpr bool bnAngle(MapWrap<MapType> map, Dir end) noexcept {
  int xM = min ? 0 : 1;
  int yM = min ? 1 : 0;
  auto pastEnd = [end](Dir nSpot) {
    if constexpr (min) {
      return nSpot.dx > end.dx;
    }
    return nSpot.dy > end.dy;
  };
  const Corner slopeBound = {end.dx + xM, end.dy + yM};
  auto slopeCmp = [](Corner s1, Corner s2) {
    if constexpr (min)
      std::swap(s1, s2);
    return slopeGT(s1, s2);
  };
  Corner slope = {end.dx + 1 - xM, end.dy + 1 - yM};
  Dir cSpot = {0, 0};
  while (true) {
    auto nSpot = nextSpot(cSpot, slope);
    if (nSpot == end || pastEnd(nSpot)) {
      return true;
    }
    if (map[nSpot]) {
      cSpot = nSpot;
      continue;
    }
    auto [x, y] = nSpot;
    Corner nSlope = {x + xM, y + yM};
    assert(slopeCmp(nSlope, slope));
    slope = nSlope;
    if (slopeCmp(slope, slopeBound)) {
      return false;
    }
    cSpot = {0, 0};
  }
}

export namespace LineOfSight {
template <SeeThrough2dArr MapType>
[[nodiscard]] constexpr bool inLineOfSight(const MapType &map, Position start, Position end) noexcept {
  auto pathD = end - start;
  if (pathD.dx == 0 || pathD.dy == 0 || std::abs(pathD.dx) == std::abs(pathD.dy)) {
    auto path = PosPathIterableShort(start, end) | std::views::drop(1) | std::views::take_while([end](Position p) { return p != end; });
    return std::ranges::all_of(path, [&map](Position p) {
      return map[p];
    });
  }
  bool flipX = pathD.dx < 0;
  bool flipY = pathD.dy < 0;
  auto absX = std::abs(pathD.dx);
  auto absY = std::abs(pathD.dy);
  bool swapXY = absY > absX;
  if (swapXY) {
    std::swap(absX, absY);
  }
  MapWrap<MapType> aMap{map, start, flipX, flipY, swapXY};
  return bnAngle<false>(aMap, Dir{absX, absY});
}

template <SeeThrough2dArr MapType>
[[nodiscard]] auto allInLineOfSight(const MapType &map, Position start) noexcept {
  return std::views::iota(0,map.extent(0)*map.extent(1)) | 
  std::views::transform([mX = map.extent(1)](int i){return Position(i%mX,i/mX);}) |
  std::views::filter([&map, start](Position end){return inLineOfSight(map,start,end);});
}
} // namespace LineOfSight