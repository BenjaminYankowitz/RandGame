export module Common:Location;
import :Random;
import std;
export class Dir;

export class Dir {
public:
  [[nodiscard]] constexpr Dir() noexcept : dx(0), dy(0) {}
  [[nodiscard]] constexpr Dir(int dxI, int dyI) noexcept : dx(dxI), dy(dyI) {}
  [[nodiscard]] constexpr bool noMove() const noexcept { return dx == 0 && dy == 0; }
  [[nodiscard]] constexpr static Dir up() noexcept {
    return {0, -1};
  };
  [[nodiscard]] constexpr static Dir down() noexcept {
    return {0, 1};
  };
  [[nodiscard]] constexpr static Dir left() noexcept {
    return {-1, 0};
  };
  [[nodiscard]] constexpr static Dir right() noexcept {
    return {1, 0};
  };
  [[nodiscard]] constexpr bool operator==(const Dir &pos) const = default;
  [[nodiscard]] constexpr Dir operator-() const noexcept { return {-dx, -dy}; }
  [[nodiscard]] constexpr static auto boxDirs() {
    constexpr static auto BoxDirs = std::to_array<Dir>({{-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}});
    return BoxDirs;
  }
  [[nodiscard]] constexpr static Dir getBoxDir(int n) {
    return boxDirs()[n];
  }
  [[nodiscard]] constexpr static auto directDirs() {
    constexpr static auto DirectDirs = std::to_array<Dir>({{0,-1},{-1,0},{1,0},{0,1}});
    return DirectDirs;
  }
  [[nodiscard]] constexpr static Dir getDirectDir(int n) {
    return directDirs()[n];
  }
  int dx;
  int dy;
};



export constexpr Dir capDir(Dir d) noexcept {
  constexpr auto Cap = [](int &x) {
    if (x > 1) {
      x = 1;
    } else if (x < -1) {
      x = -1;
    }
  };
  Cap(d.dx);
  Cap(d.dy);
  return d;
}

export std::ostream &operator<<(std::ostream &out, Dir dir) {
  out << '(' << dir.dx << ',' << dir.dy << ")";
  return out;
}

[[nodiscard]] constexpr auto abs(auto n) noexcept { 
  return n < 0 ? -n : n;
}

export struct PathIterable {
  struct PathIter {
    using difference_type = std::ptrdiff_t;
    Dir c;
    Dir e;
    constexpr PathIter operator++(int) noexcept {
      PathIter ret = *this;
      ++(*this);
      return ret;
    }
    constexpr PathIter &operator++() noexcept {
      auto v1 = abs(static_cast<std::int64_t>(c.dx) * e.dy);
      auto v2 = abs(static_cast<std::int64_t>(c.dy) * e.dx);
      if (v1 < v2) {
        c.dx += capDir(e).dx;
      } else if (v1 == v2) {
        c.dx += capDir(e).dx;
        c.dy += capDir(e).dy;
      } else {
        c.dy += capDir(e).dy;
      }
      return *this;
    }
    [[nodiscard]] constexpr Dir operator*() const noexcept {
      return c;
    }
    [[nodiscard]] constexpr bool operator==(const PathIter &) const = default;
  };
  [[nodiscard]] constexpr PathIter begin() const noexcept {
    return ++PathIter{{0, 0}, e};
  }
  [[nodiscard]] constexpr PathIter end() const noexcept {
    return ++PathIter{
        e,
        e,
    };
  }
  Dir e;
};

template <>
const bool std::ranges::enable_borrowed_range<PathIterable> = true; // NOLINT(readability-identifier-naming)

export class Position {
public:
  [[nodiscard]] constexpr Position(int xI, int yI) noexcept : x(xI), y(yI) {}
  [[nodiscard]] static constexpr int chessboard(Position l1, Position l2) noexcept {
    return std::max(abs(l1.x - l2.x), abs(l1.y - l2.y));
  }
  constexpr bool operator==(const Position &pos) const = default;
  [[nodiscard]] constexpr Position up(int d = 1) const noexcept { return {x, y - d}; }
  [[nodiscard]] constexpr Position down(int d = 1) const noexcept { return {x, y + d}; }
  [[nodiscard]] constexpr Position left(int d = 1) const noexcept { return {x - d, y}; }
  [[nodiscard]] constexpr Position right(int d = 1) const noexcept { return {x + d, y}; }
  [[nodiscard]] constexpr bool within(Position p1, Position p2 = {0, 0}) const noexcept {
    const int minx = std::min(p1.x, p2.x);
    const int maxx = std::max(p1.x, p2.x);
    const int miny = std::min(p1.y, p2.y);
    const int maxy = std::max(p1.y, p2.y);
    return x >= minx && x <= maxx && y >= miny && y <= maxy;
  }
  int x;
  int y;
};

static_assert(Position{2, 1}.within({2, 2}));

export constexpr Position &operator+=(Position &pos, Dir dir) noexcept {
  pos.x += dir.dx;
  pos.y += dir.dy;
  return pos;
}
export [[nodiscard]] constexpr Position operator+(Position pos, Dir dir) noexcept { return pos += dir; }
export constexpr Position &operator-=(Position &pos, Dir dir) noexcept {
  pos.x -= dir.dx;
  pos.y -= dir.dy;
  return pos;
}
export [[nodiscard]] constexpr Position operator-(Position pos, Dir dir) noexcept { return pos -= dir; }
export [[nodiscard]] constexpr Dir operator-(Position pos1, Position pos2) noexcept {
  return {pos1.x - pos2.x, pos1.y - pos2.y};
}

export class FloorSpecifier {
public:
  [[nodiscard]] constexpr explicit FloorSpecifier(int floorI) noexcept : floor(floorI) {}
  [[nodiscard]] constexpr bool operator==(FloorSpecifier o) const noexcept { return floor == o.floor; }
  int floor;
};

export class Location {
public:
  [[nodiscard]] constexpr Location(int x, int y, int floor) noexcept : pos(x, y), mapPos(floor) {}
  [[nodiscard]] constexpr Location(Position p, FloorSpecifier mp) noexcept : pos(p), mapPos(mp) {}
  Position pos;
  FloorSpecifier mapPos;
};

export std::ostream &operator<<(std::ostream &out, Position pos) {
  out << '(' << pos.x << ',' << pos.y << ")";
  return out;
}