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
  [[nodiscard]] constexpr bool operator[](Dir p) const noexcept {
    auto [x, y] = p;
    if (swapXY)
      std::swap(x, y);
    if (flipX)
      x = -x;
    if (flipY)
      y = -y;
    return impl[unDo(p)];
  }
  [[nodiscard]] constexpr Position unDo(Dir p) const noexcept {
    auto [x, y] = p;
    if (swapXY)
      std::swap(x, y);
    if (flipX)
      x = -x;
    if (flipY)
      y = -y;
    return offset + Dir{x, y};
  }
  [[nodiscard]] constexpr std::pair<int,int> extentsXY() const noexcept {
    int xC = impl.extent(1);
    int yC = impl.extent(0);
    auto [xO,yO] = offset;
    int retX = flipX ? xO+1 : xC-xO;
    int retY = flipY ? yO+1 : yC-yO;
    if(swapXY){
      std::swap(retX,retY);
    }
    return {retX,retY};
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

[[nodiscard]] constexpr int getYat(Corner s, int x, bool roundDown) noexcept {
  std::int64_t xV = x;
  std::int64_t sy = s.y;
  std::int64_t sx = s.x;
  auto num = ((2 * xV) - 1) * ((2 * sy) - 1);
  auto denom = (2 * sx) - 1;
  return roundDown ? (num+denom-1)/denom/2 : ((num/denom)+1)/2;
}

static_assert(getYat({1,1}, 2, false)==2);
static_assert(getYat({1,1}, 2, true)==1);


[[nodiscard]] constexpr auto operator<=>(Corner s1, Corner s2) {
  return ((2 * s1.y) - 1) * ((2 * s2.x) - 1) <=> ((2 * s2.y) - 1) * ((2 * s1.x) - 1);
}


[[nodiscard]] constexpr auto operator==(Corner s1, Corner s2) {
  return s1<=>s2==0;
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

static_assert(Corner{3, 3} == Corner{2, 2});
static_assert(Corner{2, 2} > Corner{2, 1});
static_assert(Corner{2, 2} > Corner{2, 1});

template <SeeThrough2dArr MapType>
[[nodiscard]] constexpr bool bnAngleV(MapWrap<MapType> map, Dir end) noexcept {
  const Corner slopeBoundMax = {end.dx, end.dy + 1};
  const Corner slopeBoundMin = {end.dx + 1, end.dy};
  auto slopeCmp = [](Corner s1, Corner s2, bool max) {
    if (!max)
      std::swap(s1, s2);
    return s1 > s2;
  };
  Corner slopeMax = slopeBoundMin;
  Corner slopeMin = slopeBoundMax;
  Dir cSpotMax = {0, 0};
  Dir cSpotMin = {0, 0};
  while (true) {
    const bool nUpdate = cSpotMax.dx <= cSpotMin.dx;
    Dir &cSpot = nUpdate ? cSpotMax : cSpotMin;
    Corner &slope = nUpdate ? slopeMax : slopeMin;
    const Dir nSpot = nextSpot(cSpot, slope);
    if (nSpot.dx >= end.dx && nSpot.dy >= end.dy)
      return true;
    if (map[nSpot]) {
      cSpot = nSpot;
      continue;
    }
    const auto [x, y] = nSpot;
    slope = {x + !nUpdate, y + nUpdate};
    if (slope == (nUpdate ? slopeBoundMax : slopeBoundMin) || slopeCmp(slope, nUpdate ? slopeMin : slopeMax, nUpdate)) {
      return false;
    }
    cSpot = nSpot + Dir{0, -1 + (2 * nUpdate)};
    if (cSpot == end)
      return true;
    if (!map[cSpot])
      return false;
  }
}
template <SeeThrough2dArr MapType>
void allInLineOfSightQuadImpl(MapWrap<MapType> map, Corner min, Corner max, int dist, std::vector<Position>& out) noexcept {
  const auto [extentX, extentY] = map.extentsXY();
  if(dist>=extentX){
    return;
  }
  const int minY = getYat(min,dist,false);
  const int maxY = getYat(max,dist+1,true);
  Corner pSlope = min;
  assert(min<=max);
  if(minY >= extentY){
    return;
  }
  for(int cY : std::views::iota(minY,std::min(maxY+1,extentY))){
    const Dir cSpot{dist,cY};
    out.push_back(map.unDo(cSpot));
    if(map[cSpot])
      continue;
    const Corner bCorner = {dist+1,cY};
    const Corner tCorner = {dist,cY+1};
    if(pSlope<=bCorner){
      allInLineOfSightQuadImpl(map,pSlope,bCorner,dist+1,out);
    }
    if(max<tCorner){
      return;
    }
    pSlope = tCorner;
  }
  if (pSlope<=max) {
    allInLineOfSightQuadImpl(map, pSlope, max, dist + 1, out);
  }
}

template <SeeThrough2dArr MapType>
void allInLineOfSightQuad(MapWrap<MapType> map, std::vector<Position>& out) noexcept {
  auto [extentX, extentY] = map.extentsXY();
  int mY = extentY;
  for(auto cY : std::views::iota(1,extentY)){
    Dir cSpot{0,cY};
    if(!map[cSpot]){
      mY = cY;
      break;
    }
  }
  allInLineOfSightQuadImpl(map,{extentX,1},{1,mY},1,out);
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
  return bnAngleV(aMap, Dir{absX, absY});
}

template <SeeThrough2dArr MapType>
[[nodiscard]] auto allInLineOfSight(const MapType &map, Position start) noexcept {
  std::vector<Position> ret;
  ret.push_back(start);
  allInLineOfSightQuad(MapWrap(map,start,false,false,false),ret);
  allInLineOfSightQuad(MapWrap(map,start,false,true,true),ret);
  allInLineOfSightQuad(MapWrap(map,start,true,false,true),ret);
  allInLineOfSightQuad(MapWrap(map,start,true,true,false),ret);
  return ret;
  // return std::views::iota(0, map.extent(0) * map.extent(1)) |
  //        std::views::transform([mX = map.extent(1)](int i) { return Position(i % mX, i / mX); }) |
  //        std::views::filter([&map, start](Position end) { return inLineOfSight(map, start, end); });
}
} // namespace LineOfSight

struct Q {
  static constexpr int extent(int n)  {
    if(n==0){
      return 20;
    }
    return 30;
  }
  bool operator[](Position /*unused*/) const {return false;}
};

constexpr Q Q;

constexpr MapWrap V = MapWrap{Q,{5,6},false,false,false};
constexpr MapWrap V1 = MapWrap{Q,{5,6},false,true,true};
constexpr MapWrap V2 = MapWrap{Q,{5,6},true,false,true};
constexpr MapWrap V3 = MapWrap{Q,{5,6},true,true,false};

constexpr auto Z2 = V1.extentsXY();
constexpr auto Z3 = Z2.first;

static_assert(V.extentsXY()==std::pair<int,int>(25,14));
static_assert(V1.extentsXY()==std::pair<int,int>(7,25));
static_assert(V2.extentsXY()==std::pair<int,int>(14,6));
static_assert(V3.extentsXY()==std::pair<int,int>(6,7));