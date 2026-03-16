export module DungeonMaker;
import Common;
import PerlinNoise;

template <class T>
class RoomSplitHelper {
public:
  RoomSplitHelper(Static2DArr<T> &floor, std::size_t rowB, std::size_t colB, std::size_t rowE, std::size_t colE, std::normal_distribution<> areaDist, bool dir) noexcept : floor_(floor), rowB_(rowB), colB_(colB), rowE_(rowE), colE_(colE), areaDist_(areaDist), dir_(dir) {}
  [[nodiscard]] bool doSmallStop() const noexcept {
    const std::size_t width = colE_ - colB_;
    const std::size_t height = rowE_ - rowB_;
    const std::size_t area = width * height;
    return Rnd::get(areaDist_) >= area;
  }
  [[nodiscard]] constexpr bool doThinSkip() const noexcept {
    return xE() - xB() <= 2;
  }
  [[nodiscard]] constexpr std::size_t &xB() noexcept {
    return dir_ ? rowB_ : colB_;
  }
  [[nodiscard]] constexpr std::size_t &xE() noexcept {
    return dir_ ? rowE_ : colE_;
  }
  [[nodiscard]] constexpr std::size_t &yB() noexcept {
    return dir_ ? colB_ : rowB_;
  }
  [[nodiscard]] constexpr std::size_t &yE() noexcept {
    return dir_ ? colE_ : rowE_;
  }
  [[nodiscard]] constexpr std::size_t xB() const noexcept {
    return dir_ ? rowB_ : colB_;
  }
  [[nodiscard]] constexpr std::size_t xE() const noexcept {
    return dir_ ? rowE_ : colE_;
  }
  [[nodiscard]] constexpr std::size_t yB() const noexcept {
    return dir_ ? colB_ : rowB_;
  }
  [[nodiscard]] constexpr std::size_t yE() const noexcept {
    return dir_ ? colE_ : rowE_;
  }
  [[nodiscard]] constexpr T &operator[](std::size_t x, std::size_t y) noexcept {
    if (dir_) {
      return floor_[x, y];
    }
    return floor_[y, x];
  }
  [[nodiscard]] constexpr RoomSplitHelper child(std::size_t wallX, bool right) noexcept {
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
  Static2DArr<T> &floor_;
  std::size_t rowB_;
  std::size_t colB_;
  std::size_t rowE_;
  std::size_t colE_;
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
  std::uniform_int_distribution<std::size_t> dist(rsh.xB() + 1, rsh.xE() - 2);
  const std::size_t wallX = Rnd::get(dist);
  for (std::size_t y = rsh.yB(); y < rsh.yE(); y++) {
    rsh[wallX, y] = Wall;
  }
  fillArea<Wall, Empty>(rsh.child(wallX, false));
  fillArea<Wall, Empty>(rsh.child(wallX, true));
}

constexpr double DefaultRoomArea = 100;
constexpr double DefaultRoomStandardDeviation = 0.1;

export namespace DungeonMaker {
template <auto Wall, decltype(Wall) Empty>
void rooms(Static2DArr<decltype(Wall)> &floor, double roomArea = DefaultRoomArea, double areaStddev = DefaultRoomStandardDeviation) noexcept {
  floor.fill(Empty);
  std::bernoulli_distribution dist(0.5);
  fillArea<Wall, Empty>(RoomSplitHelper(floor, 0, 0, floor.rows(), floor.cols(), std::normal_distribution<>(roomArea, areaStddev), Rnd::get(dist)));
}

template <auto Wall, decltype(Wall) Empty>
void perlin(Static2DArr<decltype(Wall)> &floor, double xscale, double yscale, double threshold = 0.0) noexcept {
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
  const double maxY = cos * maxBaseY + sin * maxBaseX;
  const double xRange = maxX - minX;
  const double yRange = maxY - minY;
  PerlinNoise::Generator gen(std::ceil(xRange + xoffset) + 1, std::ceil(yRange + yoffset) + 1);
  for (std::size_t xI = 0; xI < floor.cols(); xI++) {
    for (std::size_t yI = 0; yI < floor.rows(); yI++) {
      const double x = xI / xscale;
      const double y = yI / yscale;
      const double eX = x * cos - y * sin - minX + xoffset;
      const double eY = y * cos + x * sin - minY + yoffset;
      if (gen.getHeight(eX, eY) >= threshold) {
        floor[yI, xI] = Wall;
      } else {
        floor[yI, xI] = Empty;
      }
    }
  }
}

template <auto Wall, decltype(Wall) Empty>
void maze(Static2DArr<decltype(Wall)> &floor, std::size_t extraConnections = 0) {
  if (floor.size() == 0) {
    return;
  }
  floor.fill(Wall);
  for (std::size_t row = 0; row < floor.rows(); row += 2) {
    for (std::size_t col = 0; col < floor.cols(); col += 2) {
      floor[row, col] = Empty;
    }
  }
  const std::size_t hcols = (floor.cols() - 1) / 2;
  const std::size_t hrows = (floor.rows() + 1) / 2;
  const std::size_t hsize = hcols * hrows;
  const std::size_t vcols = (floor.cols() + 1) / 2;
  const std::size_t vrows = (floor.rows() - 1) / 2;
  const std::size_t vsize = vcols * vrows;
  const std::size_t orows = hrows;
  const std::size_t ocols = vcols;
  const std::size_t osize = orows * ocols;
  std::vector<std::size_t> ordering(hsize + vsize);
  std::iota(ordering.begin(), ordering.end(), 0);
  Rnd::shuffle(ordering);
  DisjointSet<std::size_t> connected(osize);
  std::size_t extraLeft = extraConnections;
  while (!ordering.empty()) {
    const std::size_t toOpen = ordering.back();
    ordering.pop_back();
    auto [arow, acol, horizontal] = [&]() {
      if (toOpen >= hsize) {
        const std::size_t row = (toOpen - hsize) % vrows;
        const std::size_t col = (toOpen - hsize) / vrows;
        return std::tuple{row, col, false};
      }
      const std::size_t row = toOpen % hrows;
      const std::size_t col = toOpen / hrows;
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
  Static2DArr<int> regionOf;
  std::vector<Position> representatives;
  [[nodiscard]] constexpr int numRegions() const noexcept {
    return representatives.size();
  }
};

template <auto Wall, decltype(Wall) Empty>
RegionInfo labelRegions(const Static2DArr<decltype(Wall)> &floor) noexcept {
  const auto rows = floor.rows();
  const auto cols = floor.cols();
  Static2DArr<int> regionOf(rows, cols);
  regionOf.fill(-1);
  std::vector<Position> representatives;

  for (std::size_t r = 0; r < rows; r++) {
    for (std::size_t c = 0; c < cols; c++) {
      if (floor[r, c] != Empty || regionOf[r, c] != -1)
        continue;
      const int regionId = representatives.size();
      representatives.emplace_back(static_cast<int>(c), static_cast<int>(r));
      std::queue<std::pair<std::size_t, std::size_t>> bfs;
      bfs.emplace(r, c);
      regionOf[r, c] = regionId;
      while (!bfs.empty()) {
        auto [cr, cc] = bfs.front();
        bfs.pop();
        constexpr std::array<std::pair<int, int>, 4> Deltas{{{0, 1}, {0, -1}, {1, 0}, {-1, 0}}};
        for (auto [dr, dc] : Deltas) {
          const auto nr = cr + dr;
          const auto nc = cc + dc;
          if (floor.inBounds(nr, nc) && floor[nr, nc] == Empty && regionOf[nr, nc] == -1) {
            regionOf[nr, nc] = regionId;
            bfs.emplace(nr, nc);
          }
        }
      }
    }
  }

  return {std::move(regionOf), std::move(representatives)};
}

struct Candidate {
  int regionA;
  int regionB;
  std::size_t cost;
};

template <auto Wall>
std::vector<Candidate> findCandidates(const Static2DArr<decltype(Wall)> &floor, const RegionInfo &info) noexcept {
  std::vector<Candidate> candidates;

  for (std::size_t r = 0; r < floor.rows(); r++) {
    for (std::size_t c = 0; c < floor.cols(); c++) {
      if (floor[r, c] != Wall)
        continue;
      constexpr std::array<std::pair<int, int>, 4> Deltas{{{0, 1}, {0, -1}, {1, 0}, {-1, 0}}};
      std::vector<int> adjacentRegions;
      for (auto [dr, dc] : Deltas) {
        const auto nr = r + dr;
        const auto nc = c + dc;
        if (floor.inBounds(nr, nc) && info.regionOf[nr, nc] != -1) {
          int reg = info.regionOf[nr, nc];
          if (std::ranges::find(adjacentRegions, reg) == adjacentRegions.end()) {
            adjacentRegions.push_back(reg);
          }
        }
      }
      for (std::size_t i = 0; i < adjacentRegions.size(); i++) {
        for (std::size_t j = i + 1; j < adjacentRegions.size(); j++) {
          int a = adjacentRegions[i];
          int b = adjacentRegions[j];
          candidates.push_back({a, b, Position::chessboard(info.representatives[a], info.representatives[b])});
        }
      }
    }
  }

  std::ranges::sort(candidates, {}, &Candidate::cost);
  return candidates;
}

template <auto Empty>
void carveCorridor(Static2DArr<decltype(Empty)> &floor, Position from, Position to) noexcept {
  int x = from.x;
  const int xStep = (to.x > from.x) ? 1 : -1;
  while (x != to.x) {
    const auto row = static_cast<std::size_t>(from.y);
    const auto col = static_cast<std::size_t>(x);
    if (floor.inBounds(row, col))
      floor[row, col] = Empty;
    x += xStep;
  }
  int y = from.y;
  const int yStep = (to.y > from.y) ? 1 : -1;
  while (y != to.y) {
    const auto row = static_cast<std::size_t>(y);
    const auto col = static_cast<std::size_t>(to.x);
    if (floor.inBounds(row, col))
      floor[row, col] = Empty;
    y += yStep;
  }
  {
    const auto row = static_cast<std::size_t>(to.y);
    const auto col = static_cast<std::size_t>(to.x);
    if (floor.inBounds(row, col))
      floor[row, col] = Empty;
  }
}

template <auto Wall, decltype(Wall) Empty>
void connectRegions(Static2DArr<decltype(Wall)> &floor) noexcept {
  if (floor.rows() == 0 || floor.cols() == 0)
    return;

  const auto info = labelRegions<Wall, Empty>(floor);
  if (info.numRegions() <= 1)
    return;

  const auto candidates = findCandidates<Wall>(floor, info);
  DisjointSet<std::size_t> ds(info.numRegions());

  for (const auto &cand : candidates) {
    if (ds.union_set(cand.regionA, cand.regionB))
      carveCorridor<Empty>(floor, info.representatives[cand.regionA], info.representatives[cand.regionB]);
  }

  for (int i = 1; i < info.numRegions(); i++) {
    if (ds.union_set(0, i))
      carveCorridor<Empty>(floor, info.representatives[0], info.representatives[i]);
  }
}
} // namespace DungeonMaker