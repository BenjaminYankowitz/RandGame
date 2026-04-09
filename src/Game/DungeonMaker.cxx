export module DungeonMaker;
import Common;
import PerlinNoise;
import OpenSimplex;
import GameTypes;
export namespace DungeonMaker {
void perlin(StaticPositionArr<TerrainType> &floor, double xscale, double yscale, double threshold = 0.0) noexcept {
  const double xoffset = Rnd::uniform_01();
  const double yoffset = Rnd::uniform_01();
  const double rotation = Rnd::uniform_real(0.0, 0.25 * std::numbers::pi_v<double>);
  const double cos = std::cos(rotation);
  const double sin = std::sin(rotation);
  const double maxBaseX = (floor.cols() - 1) / xscale;
  const double maxBaseY = (floor.rows() - 1) / yscale;
  const double minX = -sin * maxBaseY;
  const double minY = 0;
  const double maxX = cos * maxBaseX;
  const double maxY = (cos * maxBaseY) + (sin * maxBaseX);
  const double xRange = maxX - minX;
  const double yRange = maxY - minY;
  PerlinNoise::Generator gen(std::ceil(xRange + xoffset) + 1, std::ceil(yRange + yoffset) + 1);
  for (auto p : floor.indexIter()) {
    auto [xI, yI] = p;
    const double x = xI / xscale;
    const double y = yI / yscale;
    const double eX = (x * cos) - (y * sin) - minX + xoffset;
    const double eY = (y * cos) + (x * sin) - minY + yoffset;
    if (gen.getHeight(eX, eY) >= threshold) {
      floor[p] = TerrainType::Wall;
    } else {
      floor[p] = TerrainType::Empty;
    }
  }
}

void openSimplexRaw(StaticPositionArr<TerrainType> &floor, double xscale, double yscale, double threshold = 0.0) noexcept {
  OpenSimplex2S gen(Rnd::uniform_int<std::uint64_t>(0, std::numeric_limits<std::uint64_t>::max()));
  for (auto p : floor.indexIter()) {
    auto [xI, yI] = p;
    const double x = xI / xscale;
    const double y = yI / yscale;
    if (gen.noise2(x, y) >= threshold) {
      floor[p] = TerrainType::Wall;
    } else {
      floor[p] = TerrainType::Empty;
    }
  }
}
void connectRegions(StaticPositionArr<TerrainType> &floor) noexcept;

void openSimplex(StaticPositionArr<TerrainType> &floor, Position upStair, Position downStair, double xscale, double yscale, double threshold = 0.0) { // 32, 8, -0.2 seems like good values
  openSimplexRaw(floor, xscale, yscale, threshold);
  if (floor.inBounds(upStair)) {
    floor[upStair] = TerrainType::Empty;
  }
  if (floor.inBounds(downStair)) {
    floor[downStair] = TerrainType::Empty;
  }
  connectRegions(floor);
  if (floor.inBounds(upStair)) {
    floor[upStair] = TerrainType::UpStair;
  }
  if (floor.inBounds(downStair)) {
    floor[downStair] = TerrainType::DownStair;
  }
}

void maze(StaticPositionArr<TerrainType> &floor, int extraConnections = 0) {
  if (floor.size() == 0) {
    return;
  }
  floor.fill(TerrainType::Wall);
  for (int row = 0; row < floor.rows(); row += 2) {
    for (int col = 0; col < floor.cols(); col += 2) {
      floor[Position{col, row}] = TerrainType::Empty;
    }
  }
  const int hcols = (floor.cols() - 1) / 2;
  const int hrows = (floor.rows() + 1) / 2;
  const int hsize = hcols * hrows;
  const int vcols = (floor.cols() + 1) / 2;
  const int vrows = (floor.rows() - 1) / 2;
  const int vsize = vcols * vrows;
  const int orows = hrows;
  const int ocols = vcols;
  const int osize = orows * ocols;
  std::vector<int> ordering(hsize + vsize);
  std::ranges::iota(ordering, 0);
  Rnd::shuffle(ordering);
  DisjointSet<std::int64_t> connected(osize);
  std::int64_t extraLeft = extraConnections;
  for (auto toOpen : ordering) {
    auto [arow, acol, horizontal] = [&]() {
      if (toOpen >= hsize) {
        const int row = (toOpen - hsize) % vrows;
        const int col = (toOpen - hsize) / vrows;
        return std::tuple{row, col, false};
      }
      const int row = toOpen % hrows;
      const int col = toOpen / hrows;
      return std::tuple{row, col, true};
    }();
    auto &tile = floor[Position{acol * 2, arow * 2} + (horizontal ? Dir{1, 0} : Dir{0, 1})];
    if (connected.union_set((acol * orows) + arow, (acol * orows) + arow + (horizontal ? orows : 1))) {
      tile = TerrainType::Empty;
    } else if (extraLeft > 0) {
      extraLeft--;
      tile = TerrainType::Empty;
    }
  }
}
struct RegionInfo {
  StaticPositionArr<int> regionOf;
  int numRegionsV;
  [[nodiscard]] constexpr int numRegions() const noexcept {
    return numRegionsV;
  }
};

constexpr RegionInfo labelRegions(const StaticPositionArr<TerrainType> &floor) noexcept {
  StaticPositionArr<int> regionOf(floor.width(), floor.height());
  regionOf.fill(-1);
  int numRegions = 0;
  for (auto p : floor.indexIter()) {
    if (floor[p] != TerrainType::Empty || regionOf[p] != -1)
      continue;
    const int regionId = numRegions++;
    std::vector<Position> dfs;
    dfs.push_back(p);
    regionOf[p] = regionId;
    while (!dfs.empty()) {
      auto cPos = dfs.back();
      dfs.pop_back();
      for (auto d : Dir::boxDirs()) {
        auto nPos = cPos + d;
        if (floor.inBounds(nPos) && floor[nPos] == TerrainType::Empty && regionOf[nPos] == -1) {
          regionOf[nPos] = regionId;
          dfs.push_back(nPos);
        }
      }
    }
  }
  return {std::move(regionOf), numRegions};
}

[[nodiscard]] constexpr std::vector<Position> findEdges(const StaticPositionArr<TerrainType> &floor) noexcept {
  std::vector<Position> ret;
  for (auto pos : floor.indexIter()) {
    if (std::ranges::any_of(Dir::boxDirs(), [pos, &floor](Dir d) { return floor[pos] == TerrainType::Empty && floor.inBounds(pos + d) && floor[pos + d] == TerrainType::Wall; })) {
      ret.push_back(pos);
    }
  }
  return ret;
}

void connectRegions(StaticPositionArr<TerrainType> &floor) noexcept {
  auto info = labelRegions(floor);
  if (info.numRegions() <= 1)
    return;
  std::vector<Position> toCheck = findEdges(floor);
  StaticPositionArr<Position> parent(floor.width(), floor.height());
  parent.fill({-1, -1});
  std::ranges::for_each(toCheck, [&parent](Position pos) { parent[pos] = pos; });
  std::vector<Position> checking;
  DisjointSet<int> ds(info.numRegions());
  int regions = info.numRegions();
  auto handleCheck = [&parent, &floor, &toCheck, &info, &ds, &regions](Position spot, Dir d) {
    auto nSpot = spot + d;
    if (!parent.inBounds(nSpot) || floor[nSpot] != TerrainType::Wall)
      return false;
    if (parent[nSpot] == Position{-1, -1}) {
      parent[nSpot] = parent[spot];
      toCheck.push_back(nSpot);
      return false;
    }
    int cRegion = info.regionOf[parent[spot]];
    int oRegion = info.regionOf[parent[nSpot]];
    if (!ds.union_set(cRegion, oRegion))
      return false;
    for (auto p : PosPathIterableShort(parent[spot], parent[nSpot])) {
      floor[p] = TerrainType::Empty;
      parent[p] = p;
      info.regionOf[p] = cRegion;
    }
    return --regions == 1;
  };
  while (true) {
    if constexpr (InDebug) {
      if (toCheck.empty()) {
        Logging::log << "connectRegions: BFS exhausted without connecting all regions\n";
        return;
      }
    }
    std::swap(checking, toCheck);
    Rnd::shuffle(checking);
    for (auto spot : checking)
      for (auto d : Dir::boxDirs())
        if (handleCheck(spot, d))
          return;
    checking.clear();
  }
}
void carveHVCorridor(StaticPositionArr<TerrainType> &floor, Position from, Position to) noexcept {
  const int xStep = (to.x >= from.x) ? 1 : -1;
  for (int x = from.x; x != to.x + xStep; x += xStep) {
    floor[Position{x, from.y}] = TerrainType::Empty;
  }
  const int yStep = (to.y >= from.y) ? 1 : -1;
  for (int y = from.y; y != to.y + yStep; y += yStep) {
    floor[Position{to.x, y}] = TerrainType::Empty;
  }
}

void randomRooms(StaticPositionArr<TerrainType> &floor, Position upStair, Position downStair) noexcept {
  floor.fill(TerrainType::Wall);

  struct Room {
    int x, y, w, h;
    [[nodiscard]] Position center() const noexcept { return {x + (w / 2), y + (h / 2)}; }
    [[nodiscard]] bool contains(Position p) const noexcept { return p.x >= x && p.x < x + w && p.y >= y && p.y < y + h; }
  };

  constexpr int MinRoomSize = 4;
  constexpr int MaxRoomSize = 10;

  auto makeRoomAt = [&](Position p) -> Room {
    const int w = Rnd::uniform_int(MinRoomSize, MaxRoomSize);
    const int h = Rnd::uniform_int(MinRoomSize, MaxRoomSize);
    const int x = std::clamp(p.x - (w / 2), 0, std::max(0, floor.width() - w));
    const int y = std::clamp(p.y - (h / 2), 0, std::max(0, floor.height() - h));
    return {x, y, std::min(w, floor.width() - x), std::min(h, floor.height() - y)};
  };

  auto makeRandomRoom = [&]() -> Room {
    const int w = Rnd::uniform_int(MinRoomSize, MaxRoomSize);
    const int h = Rnd::uniform_int(MinRoomSize, MaxRoomSize);
    const int x = Rnd::uniform_int(0, std::max(0, floor.width() - w));
    const int y = Rnd::uniform_int(0, std::max(0, floor.height() - h));
    return {x, y, std::min(w, floor.width() - x), std::min(h, floor.height() - y)};
  };

  std::vector<Room> rooms;
  if (floor.inBounds(upStair))
    rooms.push_back(makeRoomAt(upStair));
  if (floor.inBounds(downStair))
    rooms.push_back(makeRoomAt(downStair));

  const int numExtra = Rnd::uniform_int(5, 12);
  for (int i = 0; i < numExtra; i++) {
    rooms.push_back(makeRandomRoom());
  }

  for (const auto &room : rooms) {
    for (int rx = room.x; rx < room.x + room.w; rx++) {
      for (int ry = room.y; ry < room.y + room.h; ry++) {
        floor[Position{rx, ry}] = TerrainType::Empty;
      }
    }
  }

  Rnd::shuffle(rooms);
  for (std::size_t i = 1; i < rooms.size(); i++) {
    carveHVCorridor(floor, rooms[i - 1].center(), rooms[i].center());
  }

  if (floor.inBounds(upStair)) {
    floor[upStair] = TerrainType::UpStair;
  }
  if (floor.inBounds(downStair)) {
    floor[downStair] = TerrainType::DownStair;
  }
}
} // namespace DungeonMaker
