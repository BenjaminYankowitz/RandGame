export module Common:Location;
import std;

export class Dir {
  class BoxIterable {
    using difference_type = std::ptrdiff_t;
    class BoxIterator {
    public:
      using value_type = Dir;
      using difference_type = BoxIterable::difference_type;
      [[nodiscard]] explicit constexpr BoxIterator(int n) : n_(n) {}
      [[nodiscard]] explicit constexpr BoxIterator() : n_(8) {}
      [[nodiscard]] constexpr Dir operator*() const {
        return getBoxDir(n_);
      }
      constexpr BoxIterator &operator++() {
        n_++;
        return *this;
      }
      constexpr BoxIterator operator++(int) {
        BoxIterator ret = *this;
        operator++();
        return ret;
      }
      [[nodiscard]] constexpr bool operator==(BoxIterator o) const {
        return n_ == o.n_;
      }
      [[nodiscard]] constexpr bool operator!=(BoxIterator o) const {
        return n_ != o.n_;
      }

    private:
      int n_;
    };

  public:
    [[nodiscard]] static constexpr BoxIterator begin() {
      return BoxIterator(0);
    }
    [[nodiscard]] static constexpr BoxIterator end() {
      return BoxIterator();
    }
    [[nodiscard]] static constexpr BoxIterator cbegin() { return begin(); }
    [[nodiscard]] static constexpr BoxIterator cend() { return end(); }
  };
  constexpr static BoxIterable BoxIteratorInst;

public:
  [[nodiscard]] constexpr static Dir getBoxDir(int n) {
  if(n<0 || n>=8){
    std::unreachable();
  }
    const int k = n + (n >= 4 ? 1 : 0);
    return {k % 3 - 1, k / 3 - 1};
  }
  [[nodiscard]] constexpr Dir() noexcept : dx(0), dy(0) {}
  [[nodiscard]] constexpr Dir(int dxI, int dyI) noexcept : dx(dxI), dy(dyI) {}
  [[nodiscard]] constexpr bool noMove() const noexcept { return dx == 0 && dy == 0; }
  [[nodiscard]] constexpr static Dir up() noexcept {
    return {0, -1};
  };
  [[nodiscard]] constexpr static Dir down() {
    return {0, 1};
  };
  [[nodiscard]] constexpr static Dir left() {
    return {-1, 0};
  };
  [[nodiscard]] constexpr static Dir right() {
    return {1, 0};
  };
  bool operator==(const Dir &pos) const = default;
  [[nodiscard]] constexpr static const BoxIterable &boxDirs() {
    return BoxIteratorInst;
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

[[nodiscard]] constexpr auto abs(auto n) noexcept{
  return n < 0 ? -n : n;
}

export class Position {
public:
  [[nodiscard]] constexpr Position(int xI, int yI) noexcept : x(xI), y(yI) {}
  [[nodiscard]] static constexpr std::size_t chessboard(Position l1, Position l2) noexcept {
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

static_assert(Position{2,1}.within({2,2}));

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