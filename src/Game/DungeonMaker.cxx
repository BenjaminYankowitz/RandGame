export module DungeonMaker;
import Common;
import PerlinNoise;

template <class T, class size_type>
class RoomSplitHelper {
public:
  RoomSplitHelper(Static2DArr<T, size_type> &floor, size_type rowB, size_type colB, size_type rowE, size_type colE, std::normal_distribution<> areaDist, bool dir) noexcept : floor_(floor), rowB_(rowB), colB_(colB), rowE_(rowE), colE_(colE), areaDist_(areaDist), dir_(dir) {}
  [[nodiscard]] bool doSmallStop() const noexcept {
    const size_type width = colE_ - colB_;
    const size_type height = rowE_ - rowB_;
    const size_type area = width * height;
    return Rnd::get(areaDist_) >= area;
  }
  [[nodiscard]] constexpr bool doThinSkip() const noexcept {
    return xE() - xB() <= 2;
  }
  [[nodiscard]] constexpr size_type &xB() noexcept {
    return dir_ ? rowB_ : colB_;
  }
  [[nodiscard]] constexpr size_type &xE() noexcept {
    return dir_ ? rowE_ : colE_;
  }
  [[nodiscard]] constexpr size_type &yB() noexcept {
    return dir_ ? colB_ : rowB_;
  }
  [[nodiscard]] constexpr size_type &yE() noexcept {
    return dir_ ? colE_ : rowE_;
  }
  [[nodiscard]] constexpr size_type xB() const noexcept {
    return dir_ ? rowB_ : colB_;
  }
  [[nodiscard]] constexpr size_type xE() const noexcept {
    return dir_ ? rowE_ : colE_;
  }
  [[nodiscard]] constexpr size_type yB() const noexcept {
    return dir_ ? colB_ : rowB_;
  }
  [[nodiscard]] constexpr size_type yE() const noexcept {
    return dir_ ? colE_ : rowE_;
  }
  [[nodiscard]] constexpr T &operator[](size_type x, size_type y) noexcept {
    if (dir_) {
      return floor_[x, y];
    }
    return floor_[y, x];
  }
  [[nodiscard]] constexpr RoomSplitHelper child(size_type wallX, bool right) noexcept {
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
  Static2DArr<T, size_type> &floor_;
  size_type rowB_;
  size_type colB_;
  size_type rowE_;
  size_type colE_;
  mutable std::normal_distribution<> areaDist_;
  bool dir_;
};

template <auto Wall, decltype(Wall) Empty, class size_type>
void fillArea(RoomSplitHelper<decltype(Wall), size_type> rsh) noexcept {
  if (rsh.doSmallStop()) {
    return;
  }
  if (rsh.doThinSkip()) {
    fillArea<Wall, Empty>(rsh.bigChild());
    return;
  }
  std::uniform_int_distribution<size_type> dist(rsh.xB() + 1, rsh.xE() - 2);
  const size_type wallX = Rnd::get(dist);
  for (size_type y = rsh.yB(); y < rsh.yE(); y++) {
    rsh[wallX, y] = Wall;
  }
  fillArea<Wall, Empty>(rsh.child(wallX, false));
  fillArea<Wall, Empty>(rsh.child(wallX, true));
}

constexpr double DefaultRoomArea = 100;
constexpr double DefaultRoomStandardDeviation = 0.1;

export namespace DungeonMaker {
template <auto Wall, decltype(Wall) Empty, class size_type>
void rooms(Static2DArr<decltype(Wall), size_type> &floor, double roomArea = DefaultRoomArea, double areaStddev = DefaultRoomStandardDeviation) noexcept {
  floor.fill(Empty);
  std::bernoulli_distribution dist(0.5);
  fillArea<Wall, Empty>(RoomSplitHelper(floor, 0, 0, floor.rows(), floor.cols(), std::normal_distribution<>(roomArea, areaStddev), Rnd::get(dist)));
}

template <auto Wall, decltype(Wall) Empty, class size_type>
void perlin(Static2DArr<decltype(Wall), size_type> &floor, double xscale, double yscale, double threshold = 0.0) noexcept {
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
  for (size_type xI = 0; xI < floor.cols(); xI++) {
    for (size_type yI = 0; yI < floor.rows(); yI++) {
      const double x = xI / xscale;
      const double y = yI / yscale;
      const double eX = (x * cos) - (y * sin) - minX + xoffset;
      const double eY = (y * cos) + (x * sin) - minY + yoffset;
      if (gen.getHeight(eX, eY) >= threshold) {
        floor[yI, xI] = Wall;
      } else {
        floor[yI, xI] = Empty;
      }
    }
  }
}

template <auto Wall, decltype(Wall) Empty, class size_type>
void maze(Static2DArr<decltype(Wall), size_type> &floor, size_type extraConnections = 0) {
  if (floor.size() == 0) {
    return;
  }
  floor.fill(Wall);
  for (size_type row = 0; row < floor.rows(); row += 2) {
    for (size_type col = 0; col < floor.cols(); col += 2) {
      floor[row, col] = Empty;
    }
  }
  const size_type hcols = (floor.cols() - 1) / 2;
  const size_type hrows = (floor.rows() + 1) / 2;
  const size_type hsize = hcols * hrows;
  const size_type vcols = (floor.cols() + 1) / 2;
  const size_type vrows = (floor.rows() - 1) / 2;
  const size_type vsize = vcols * vrows;
  const size_type orows = hrows;
  const size_type ocols = vcols;
  const size_type osize = orows * ocols;
  std::vector<size_type> ordering(hsize + vsize);
  std::iota(ordering.begin(), ordering.end(), 0);
  Rnd::shuffle(ordering);
  DisjointSet<std::int64_t> connected(osize);
  std::int64_t extraLeft = extraConnections;
  while (!ordering.empty()) {
    const size_type toOpen = ordering.back();
    ordering.pop_back();
    auto [arow, acol, horizontal] = [&]() {
      if (toOpen >= hsize) {
        const size_type row = (toOpen - hsize) % vrows;
        const size_type col = (toOpen - hsize) / vrows;
        return std::tuple{row, col, false};
      }
      const size_type row = toOpen % hrows;
      const size_type col = toOpen / hrows;
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
template <class size_type>
struct RegionInfo {
  Static2DArr<int, size_type> regionOf;
  std::vector<Position> representatives;
  [[nodiscard]] constexpr int numRegions() const noexcept {
    return representatives.size();
  }
};

template <auto Wall, decltype(Wall) Empty, class size_type>
constexpr RegionInfo<size_type> labelRegions(const Static2DArr<decltype(Wall), size_type> &floor) noexcept {
  const auto rows = floor.rows();
  const auto cols = floor.cols();
  Static2DArr<int, size_type> regionOf(rows, cols);
  regionOf.fill(-1);
  std::vector<Position> representatives;

  for (size_type r = 0; r < rows; r++) {
    for (size_type c = 0; c < cols; c++) {
      if (floor[r, c] != Empty || regionOf[r, c] != -1)
        continue;
      const int regionId = representatives.size();
      representatives.emplace_back(static_cast<int>(c), static_cast<int>(r));
      std::vector<std::pair<size_type, size_type>> bfs;
      bfs.emplace_back(r, c);
      regionOf[r, c] = regionId;
      for (std::size_t bfsI = 0; bfsI < bfs.size(); bfsI++) {
        auto [cr, cc] = bfs[bfsI];
        constexpr std::array<std::pair<int, int>, 4> Deltas{{{0, 1}, {0, -1}, {1, 0}, {-1, 0}}};
        for (auto [dr, dc] : Deltas) {
          const auto nr = cr + dr;
          const auto nc = cc + dc;
          if (floor.inBounds(nr, nc) && floor[nr, nc] == Empty && regionOf[nr, nc] == -1) {
            regionOf[nr, nc] = regionId;
            bfs.emplace_back(nr, nc);
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
  int cost;
};

template <auto Wall, class size_type>
constexpr std::vector<Candidate> findCandidates(const Static2DArr<decltype(Wall), size_type> &floor, const RegionInfo<size_type> &info) noexcept {
  std::vector<Candidate> candidates;
  for (auto [r, c] : floor.indexIter()) {
    if (floor[r, c] != Wall)
      continue;
    std::vector<int> adjacentRegions;
    for (auto d : Dir::directDirs()) {
      const auto nr = r + d.dy;
      const auto nc = c + d.dx;
      if (floor.inBounds(nr, nc) && info.regionOf[nr, nc] != -1) {
        int reg = info.regionOf[nr, nc];
        if (std::ranges::find(adjacentRegions, reg) == adjacentRegions.end()) {
          adjacentRegions.push_back(reg);
        }
      }
    }
    for (auto i : std::views::iota(static_cast<std::size_t>(0), adjacentRegions.size())) {
      for (auto j : std::views::iota(i + 1, adjacentRegions.size())) {
        int a = adjacentRegions[i];
        int b = adjacentRegions[j];
        candidates.push_back({a, b, Position::chessboard(info.representatives[a], info.representatives[b])});
      }
    }
  }

  std::ranges::sort(candidates, {}, &Candidate::cost);
  return candidates;
}

template <auto Empty, class size_type>
constexpr void carveCorridor(Static2DArr<decltype(Empty), size_type> &floor, Position from, Position to) noexcept {
  int x = from.x;
  const int xStep = (to.x > from.x) ? 1 : -1;
  while (x != to.x) {
    const auto row = static_cast<size_type>(from.y);
    const auto col = static_cast<size_type>(x);
    if (floor.inBounds(row, col))
      floor[row, col] = Empty;
    x += xStep;
  }
  int y = from.y;
  const int yStep = (to.y > from.y) ? 1 : -1;
  while (y != to.y) {
    const auto row = static_cast<size_type>(y);
    const auto col = static_cast<size_type>(to.x);
    if (floor.inBounds(row, col))
      floor[row, col] = Empty;
    y += yStep;
  }
  {
    const auto row = static_cast<size_type>(to.y);
    const auto col = static_cast<size_type>(to.x);
    if (floor.inBounds(row, col))
      floor[row, col] = Empty;
  }
}

template <auto Wall, decltype(Wall) Empty, class size_type>
constexpr void connectRegions(Static2DArr<decltype(Wall), size_type> &floor) noexcept {
  if (floor.rows() == 0 || floor.cols() == 0)
    return;

  const auto info = labelRegions<Wall, Empty>(floor);
  if (info.numRegions() <= 1)
    return;

  const auto candidates = findCandidates<Wall>(floor, info);
  DisjointSet<size_type> ds(info.numRegions());

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