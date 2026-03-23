export module Common:Location;
import :Random;
import :Static2DArr;
import std;

[[nodiscard]] constexpr auto abs(auto n) noexcept { 
  return n < 0 ? -n : n;
}
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
    return std::ranges::ref_view(BoxDirs);
  }
  [[nodiscard]] constexpr static Dir getBoxDir(int n) {
    return boxDirs()[n];
  }
  [[nodiscard]] constexpr static auto directDirs() {
    constexpr static auto DirectDirs = std::to_array<Dir>({{0,-1},{-1,0},{1,0},{0,1}});
    return std::ranges::ref_view(DirectDirs);
  }
  [[nodiscard]] constexpr static Dir getDirectDir(int n) {
    return directDirs()[n];
  }
  [[nodiscard]] static constexpr int chessboard(Dir d) noexcept {
    return std::max(abs(d.dx),abs(d.dy));
  }
  int dx;
  int dy;
};

constexpr auto V = std::same_as<decltype(Dir::boxDirs()),decltype(Dir::directDirs())>;



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

struct PathIterSentinal {};

export struct PathIterable {
  struct PathIter {
    using difference_type = std::ptrdiff_t;
    using value_type = Dir;
    Dir c;
    Dir e;
    constexpr PathIter operator++(int) noexcept {
      PathIter ret = *this;
      ++(*this);
      return ret;
    }
    constexpr PathIter &operator++() noexcept {
      if(e.noMove()){
        c = {1,0};
      }
      auto v1 = abs<std::int64_t>(e.dy) * ((2 * abs<std::int64_t>(c.dx)) + 1);
      auto v2 = abs<std::int64_t>(e.dx) * ((2 * abs<std::int64_t>(c.dy)) + 1);
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
    [[nodiscard]] constexpr bool operator==(PathIterSentinal /*unused*/) const {
      return Dir::chessboard(c)>Dir::chessboard(e);
    };
  };
  [[nodiscard]] constexpr PathIter begin() const noexcept {
    return PathIter{{0, 0}, e};
  }
  [[nodiscard]] static constexpr PathIterSentinal end() noexcept {
    return {};
  }
  Dir e;
  // bool favor
};
template <>
const bool std::ranges::enable_borrowed_range<PathIterable> = true; // NOLINT(readability-identifier-naming)
export struct PathIterableShort {
  struct PathIter {
    using difference_type = std::ptrdiff_t;
    using value_type = Dir;
    int dist;
    Dir e;
    constexpr PathIter operator++(int) noexcept {
      PathIter ret = *this;
      ++(*this);
      return ret;
    }
    constexpr PathIter &operator++() noexcept {
      ++dist;
      return *this;
    }
    [[nodiscard]] constexpr Dir operator*() const noexcept {
      if(dist==0){
        return {0,0};
      }
      int bDG = Dir::chessboard(e);
      int sDG = std::min(abs(e.dx),abs(e.dy));
      int sDC = ((dist*sDG)+(bDG/2))/bDG;
      auto capEnd = capDir(e);
      if(abs(e.dx) < abs(e.dy)){
        return {sDC*capEnd.dx,dist*capEnd.dy};
      }
      return {dist*capEnd.dx,sDC*capEnd.dy};
    }
    [[nodiscard]] constexpr bool operator==(const PathIter &) const = default;
    [[nodiscard]] constexpr bool operator==(PathIterSentinal /*unused*/) const noexcept{
      return dist>Dir::chessboard(e);
    }
  };
  [[nodiscard]] constexpr PathIter begin() const noexcept {
    return PathIter{0, e};
  }
  [[nodiscard]] static constexpr PathIterSentinal end() noexcept {
    return {};
  }
  Dir e;
};
template <>
const bool std::ranges::enable_borrowed_range<PathIterableShort> = true; // NOLINT(readability-identifier-naming)

export class Position {
public:
  Position() = default;
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

export template<class T>
class StaticPositionArr : public Static2DArr<T,int> {
  public:
  constexpr StaticPositionArr(int width, int height) noexcept: Static2DArr<T,int>(height,width){}
  template<std::size_t size>
  constexpr StaticPositionArr(std::initializer_list<T[size]> list) noexcept: Static2DArr<T,int>(list){} //NOLINT(modernize-avoid-c-arrays)
  [[nodiscard]] constexpr auto& operator[](this auto&& self, Position p) noexcept {
    return self.Static2DArr<T,int>::operator[](p.y,p.x);
  }
  [[nodiscard]] constexpr int width() const noexcept{
    return Static2DArr<T,int>::cols();
  }
  [[nodiscard]] constexpr int height() const noexcept{
    return Static2DArr<T,int>::rows();
  }
  [[nodiscard]] constexpr bool inBounds(Position p) const noexcept {
    return Static2DArr<T,int>::inBounds(p.y,p.x);
  }
  [[nodiscard]] constexpr auto indexIter() const noexcept {
    return std::views::transform(Static2DArr<T,int>::indexIter(),[](std::pair<int,int> p){return Position{p.second,p.first};});
  }
};

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
  [[nodiscard]] constexpr FloorSpecifier up(int n = 1) const noexcept {return FloorSpecifier(floor-n);}
  [[nodiscard]] constexpr FloorSpecifier down(int n = 1) const noexcept {return FloorSpecifier(floor+n);}
  int floor;
};

export class Location {
public:
  [[nodiscard]] constexpr Location(int x, int y, int floor) noexcept : pos(x, y), mapPos(floor) {}
  [[nodiscard]] constexpr Location(Position p, FloorSpecifier mp) noexcept : pos(p), mapPos(mp) {}
  [[nodiscard]] constexpr Location up(int n = 1) const noexcept {return {pos,mapPos.up(n)};}
  [[nodiscard]] constexpr Location down(int n = 1) const noexcept {return {pos,mapPos.down(n)};}
  Position pos;
  FloorSpecifier mapPos;
};

export std::ostream &operator<<(std::ostream &out, Position pos) {
  out << '(' << pos.x << ',' << pos.y << ")";
  return out;
}