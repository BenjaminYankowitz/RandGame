export module DungeonMaker;
import Common;
import PerlinNoise;
import OpenSimplex;

template <class T>
class RoomSplitHelper {
public:
  RoomSplitHelper(StaticPositionArr<T> &floor, int rowB, int colB, int rowE, int colE, std::normal_distribution<> areaDist, bool dir) noexcept : floor_(floor), rowB_(rowB), colB_(colB), rowE_(rowE), colE_(colE), areaDist_(areaDist), dir_(dir) {}
  [[nodiscard]] bool doSmallStop() const noexcept {
    const int width = colE_ - colB_;
    const int height = rowE_ - rowB_;
    const int area = width * height;
    return Rnd::get(areaDist_) >= area;
  }
  [[nodiscard]] constexpr bool doThinSkip() const noexcept {
    return xE() - xB() <= 2;
  }
  [[nodiscard]] constexpr auto &&xB(this auto &&self) noexcept {
    return self.dir_ ? self.rowB_ : self.colB_;
  }
  [[nodiscard]] constexpr auto &&xE(this auto &&self) noexcept {
    return self.dir_ ? self.rowE_ : self.colE_;
  }
  [[nodiscard]] constexpr auto &&yB(this auto &&self) noexcept {
    return self.dir_ ? self.colB_ : self.rowB_;
  }
  [[nodiscard]] constexpr auto &&yE(this auto &&self) noexcept {
    return self.dir_ ? self.colE_ : self.rowE_;
  }
  [[nodiscard]] constexpr T &operator[](Position p) noexcept {
    if (dir_) {
      return floor_[p];
    }
    return floor_[p];
  }
  [[nodiscard]] constexpr RoomSplitHelper child(int wallX, bool right) noexcept {
    RoomSplitHelper ret = *this;
    if (right) {
      ret.xB() = wallX + 1;
    } else {
      ret.xE() = wallX;
    }
    ret.dir_ = !ret.dir_;
    return ret;
  }
  [[nodiscard]] constexpr RoomSplitHelper bigChild() noexcept {
    RoomSplitHelper ret = *this;
    ret.dir_ = !ret.dir_;
    return ret;
  }

private:
  StaticPositionArr<T> &floor_;
  int rowB_;
  int colB_;
  int rowE_;
  int colE_;
  mutable std::normal_distribution<> areaDist_;
  bool dir_;
};

template <auto Wall, decltype(Wall) Empty>
void fillArea(RoomSplitHelper<decltype(Wall)> rsh) noexcept {
  if (rsh.doSmallStop()) {
    return;
  }
  if (rsh.doThinSkip()) {
    fillArea<Wall, Empty>(rsh.bigChild());
    return;
  }
  const int wallX = Rnd::uniform_int(rsh.xB() + 1, rsh.xE() - 2);
  for (int y = rsh.yB(); y < rsh.yE(); y++) {
    rsh[wallX, y] = Wall;
  }
  fillArea<Wall, Empty>(rsh.child(wallX, false));
  fillArea<Wall, Empty>(rsh.child(wallX, true));
}

constexpr double DefaultRoomArea = 100;
constexpr double DefaultRoomStandardDeviation = 0.1;

export namespace DungeonMaker {
template <auto Wall, decltype(Wall) Empty>
void rooms(StaticPositionArr<decltype(Wall)> &floor, double roomArea = DefaultRoomArea, double areaStddev = DefaultRoomStandardDeviation) noexcept {
  floor.fill(Empty);
  std::bernoulli_distribution dist(0.5);
  fillArea<Wall, Empty>(RoomSplitHelper(floor, 0, 0, floor.rows(), floor.cols(), std::normal_distribution<>(roomArea, areaStddev), Rnd::get(dist)));
}

template <auto Wall, decltype(Wall) Empty>
void perlin(StaticPositionArr<decltype(Wall)> &floor, double xscale, double yscale, double threshold = 0.0) noexcept {
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
      floor[p] = Wall;
    } else {
      floor[p] = Empty;
    }
  }
}

template <auto Wall, decltype(Wall) Empty>
void openSimplex(StaticPositionArr<decltype(Wall)> &floor, double xscale, double yscale, double threshold = 0.0) noexcept {
  OpenSimplex2S gen(Rnd::uniform_int<std::uint64_t>(0, std::numeric_limits<std::uint64_t>::max()));
  for (auto p : floor.indexIter()) {
    auto [xI, yI] = p;
    const double x = xI / xscale;
    const double y = yI / yscale;
    if (gen.noise2(x, y) >= threshold) {
      floor[p] = Wall;
    } else {
      floor[p] = Empty;
    }
  }
}

template <auto Wall, decltype(Wall) Empty>
void maze(StaticPositionArr<decltype(Wall)> &floor, int extraConnections = 0) {
  if (floor.size() == 0) {
    return;
  }
  floor.fill(Wall);
  for (int row = 0; row < floor.rows(); row += 2) {
    for (int col = 0; col < floor.cols(); col += 2) {
      floor[row, col] = Empty;
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
  std::iota(ordering.begin(), ordering.end(), 0);
  Rnd::shuffle(ordering);
  DisjointSet<std::int64_t> connected(osize);
  std::int64_t extraLeft = extraConnections;
  while (!ordering.empty()) {
    const int toOpen = ordering.back();
    ordering.pop_back();
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
    auto &tile = floor[arow * 2 + !horizontal, acol * 2 + horizontal];
    if (connected.union_set(acol * orows + arow, acol * orows + arow + (horizontal ? orows : 1))) {
      tile = Empty;
    } else if (extraLeft > 0) {
      extraLeft--;
      tile = Empty;
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

template <auto Wall, decltype(Wall) Empty>
constexpr RegionInfo labelRegions(const StaticPositionArr<decltype(Wall)> &floor) noexcept {
  StaticPositionArr<int> regionOf(floor.width(), floor.height());
  regionOf.fill(-1);
  int numRegions = 0;
  for (auto p : floor.indexIter()) {
    if (floor[p] != Empty || regionOf[p] != -1)
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
        if (floor.inBounds(nPos) && floor[nPos] == Empty && regionOf[nPos] == -1) {
          regionOf[nPos] = regionId;
          dfs.push_back(nPos);
        }
      }
    }
  }
  return {std::move(regionOf), numRegions};
}

template <auto Empty>
constexpr void carveCorridor(StaticPositionArr<decltype(Empty)> &floor, Position from, Position to) noexcept {
  for (auto d : PathIterableShort(to - from)) {
    floor[from + d] = Empty;
  }
}

template <auto Wall, decltype(Wall) Empty>
[[nodiscard]] constexpr std::vector<Position> findEdges(const StaticPositionArr<decltype(Wall)> &floor) noexcept {
  std::vector<Position> ret;
  for (auto pos : floor.indexIter()) {
    if (std::ranges::any_of(Dir::boxDirs(), [pos, &floor](Dir d) { return floor[pos] == Empty && floor.inBounds(pos + d) && floor[pos + d] == Wall; })) {
      ret.push_back(pos);
    }
  }
  return ret;
}

template <auto Wall, decltype(Wall) Empty>
void connectRegions(StaticPositionArr<decltype(Wall)> &floor) noexcept {
  if (floor.isNull())
    return;

  const auto info = labelRegions<Wall, Empty>(floor);
  if (info.numRegions() <= 1)
    return;

  std::vector<Position> toCheck = findEdges<Wall, Empty>(floor);
  StaticPositionArr<Position> parent(floor.width(), floor.height());
  parent.fill({-1, -1});
  for (auto pos : toCheck) {
    parent[pos] = pos;
  }
  std::vector<Position> checking;
  DisjointSet<int> ds(info.numRegions());
  int regions = info.numRegions();
  auto handleCheck = [&](Position spot, Dir d) {
    auto nSpot = spot + d;
    if (!parent.inBounds(nSpot) || floor[nSpot] != Wall)
      return false;
    if (parent[nSpot] == Position{-1, -1}) {
      parent[nSpot] = parent[spot];
      toCheck.push_back(nSpot);
      return false;
    }
    int cRegion = info.regionOf[parent[spot]];
    int oRegion = info.regionOf[parent[nSpot]];
    if (ds.union_set(cRegion, oRegion)) {
      carveCorridor<Empty>(floor, parent[spot], parent[nSpot]);
      if (--regions == 1) {
        return true;
      }
    }
    return false;
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
    for (auto spot : checking) {
      for (auto d : Dir::boxDirs()) {
        if (handleCheck(spot, d)) {
          return;
        }
      }
    }
    checking.clear();
  }
}
} // namespace DungeonMaker