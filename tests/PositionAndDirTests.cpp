#include "TestHeader.h"
import Common;

// -- Dir Construction & noMove --
static_assert(Dir().dx == 0);
static_assert(Dir().dy == 0);
static_assert(Dir(3, -7).dx == 3);
static_assert(Dir(3, -7).dy == -7);
static_assert(Dir(0, 0).noMove());
static_assert(Dir().noMove());
static_assert(!Dir(1, 0).noMove());
static_assert(!Dir(0, 1).noMove());
static_assert(!Dir(-1, -1).noMove());
static_assert(!Dir(100, -200).noMove());
static_assert(std::ranges::none_of(Dir::boxDirs(), [](Dir d) { return d.noMove(); }));

// -- Dir Cardinals --
static_assert(Dir::up() == Dir(0, -1));
static_assert(Dir::down() == Dir(0, 1));
static_assert(Dir::left() == Dir(-1, 0));
static_assert(Dir::right() == Dir(1, 0));

// -- Dir Unary Negation --
static_assert(-Dir(1, 2) == Dir(-1, -2));
static_assert(-Dir(-3, 0) == Dir(3, 0));
static_assert(-Dir(0, 0) == Dir(0, 0));
static_assert(-Dir(0, -5) == Dir(0, 5));
static_assert(-Dir(-4, 7) == Dir(4, -7));
static_assert(!(-Dir(1, 1)).noMove());
static_assert((-Dir(0, 0)).noMove());
static_assert(-(-Dir(3, -2)) == Dir(3, -2));
static_assert(std::ranges::all_of(Dir::boxDirs(), [](Dir d) { return -(-d) == d; }));

// -- Dir Equality --
static_assert(Dir(1, 2) == Dir(1, 2));
static_assert(Dir(-3, 0) == Dir(-3, 0));
static_assert(Dir(1, 0) != Dir(2, 0));
static_assert(Dir(0, 1) != Dir(0, 2));
static_assert(Dir(1, 2) != Dir(3, 4));

// -- boxDir answer key & boxDirs range equality --
constexpr static auto AnswerKeyBoxDir = std::to_array<Dir>(
    {{-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}});
constexpr static auto AnswerKeyDirectDir = std::to_array<Dir>(
    {{0, -1}, {-1, 0}, {1, 0}, {0, 1}});

static_assert(std::ranges::all_of(std::views::iota(0, 8), [](std::int8_t dirN) {
  return AnswerKeyBoxDir[dirN] == Dir::getBoxDir(dirN);
}));
static_assert(std::ranges::all_of(std::views::iota(0, 4), [](std::int8_t dirN) {
  return AnswerKeyDirectDir[dirN] == Dir::getDirectDir(dirN);
}));

// static_assert(Dir::getDirectDir(1).dy==-1);

static_assert(std::ranges::equal(AnswerKeyBoxDir, Dir::boxDirs()));
static_assert(std::ranges::equal(AnswerKeyDirectDir, Dir::directDirs()));

// -- Position Construction & Equality --
static_assert(Position(10, -20).x == 10);
static_assert(Position(10, -20).y == -20);
static_assert(Position(0, 0).x == 0);
static_assert(Position(0, 0).y == 0);
static_assert(Position(1, 2) == Position(1, 2));
static_assert(Position(1, 2) != Position(3, 4));
static_assert(Position(1, 2) != Position(1, 3));
static_assert(Position(1, 2) != Position(3, 2));

// -- Position Movement --
static_assert(Position(5, 5).up() == Position(5, 4));
static_assert(Position(5, 5).down() == Position(5, 6));
static_assert(Position(5, 5).left() == Position(4, 5));
static_assert(Position(5, 5).right() == Position(6, 5));
static_assert(Position(10, 10).up(3) == Position(10, 7));
static_assert(Position(10, 10).down(4) == Position(10, 14));
static_assert(Position(10, 10).left(5) == Position(5, 10));
static_assert(Position(10, 10).right(6) == Position(16, 10));
static_assert(Position(0, 0).up().right().down().left() == Position(0, 0));

// -- Chessboard --
static_assert(Position::chessboard({3, 3}, {3, 3}) == 0u);
static_assert(Position::chessboard({0, 0}, {1, 0}) == 1u);
static_assert(Position::chessboard({0, 0}, {0, 1}) == 1u);
static_assert(Position::chessboard({0, 0}, {1, 1}) == 1u);
static_assert(Position::chessboard({0, 0}, {7, 0}) == 7u);
static_assert(Position::chessboard({0, 0}, {3, 1}) == 3u);
static_assert(Position::chessboard({0, 0}, {1, 5}) == 5u);
static_assert(Position::chessboard({2, 3}, {7, 1}) == Position::chessboard({7, 1}, {2, 3}));
static_assert(Position::chessboard({-3, -4}, {2, 1}) == 5u);

// -- Within --
static_assert(Position(5, 5).within({10, 10}));
static_assert(Position(0, 0).within({10, 10}));
static_assert(Position(10, 10).within({10, 10}));
static_assert(Position(5, 0).within({10, 10}));
static_assert(Position(0, 5).within({10, 10}));
static_assert(Position(10, 5).within({10, 10}));
static_assert(Position(5, 10).within({10, 10}));
static_assert(!Position(11, 5).within({10, 10}));
static_assert(!Position(5, 11).within({10, 10}));
static_assert(!Position(-1, 5).within({10, 10}));
static_assert(!Position(5, -1).within({10, 10}));
static_assert(Position(5, 5).within({3, 3}, {7, 7}));
static_assert(!Position(2, 5).within({3, 3}, {7, 7}));
static_assert(Position(5, 5).within({7, 7}, {3, 3}));
static_assert(Position(5, 5).within({5, 5}, {5, 5}));
static_assert(!Position(4, 5).within({5, 5}, {5, 5}));

// -- Position + Dir operators --
static_assert(Position(3, 4) + Dir(1, -2) == Position(4, 2));
static_assert(Position(3, 4) - Dir(1, -2) == Position(2, 6));
static_assert(Position(3, 4) + Dir() == Position(3, 4));
static_assert(Position(3, 4) + Dir(1, -1) == Position(4, 3));
static_assert(Position(3, 4) - Dir(1, -1) == Position(2, 5));
static_assert((Position(10, 20) + Dir(3, -5)) - Dir(3, -5) == Position(10, 20));

static_assert([] {
  Position p(3, 4);
  p += Dir(2, 3);
  return p == Position(5, 7);
}());

static_assert([] {
  Position p(3, 4);
  p -= Dir(2, 3);
  return p == Position(1, 1);
}());

// -- Position - Position = Dir --
static_assert((Position(5, 3) - Position(2, 1)) == Dir(3, 2));
static_assert((Position(7, 7) - Position(7, 7)).noMove());
static_assert((Position(7, 7) - Position(7, 7)) == Dir(0, 0));
static_assert((Position(1, 2) - Position(5, 8)) == Dir(-4, -6));

// -- capDir --
static_assert(capDir(Dir(1, 1)) == Dir(1, 1));
static_assert(capDir(Dir(-1, -1)) == Dir(-1, -1));
static_assert(capDir(Dir(1, 0)) == Dir(1, 0));
static_assert(capDir(Dir(5, 10)) == Dir(1, 1));
static_assert(capDir(Dir(-5, -10)) == Dir(-1, -1));
static_assert(capDir(Dir(5, -5)) == Dir(1, -1));
static_assert(capDir(Dir(0, 0)) == Dir(0, 0));
static_assert(capDir(Dir(0, 0)).noMove());
static_assert(capDir(Dir(3, -7)) == Dir(1, -1));
static_assert(capDir(Dir(-2, 4)) == Dir(-1, 1));

// -- Integration --
static_assert(
    std::ranges::all_of(Dir::boxDirs(), [](Dir d) {
      static constexpr Position Start(50, 50);
      Position moved = Start + d;
      Position returned = moved - d;
      return moved != Start && returned == Start;
    }));

static_assert(
    std::ranges::all_of(Dir::boxDirs(), [](Dir d) {
      constexpr static Position Origin(23, 78);
      Position neighbor = Origin + d;
      return Position::chessboard(Origin, neighbor) == 1u;
    }));

static_assert([] {
  Position a(10, 20);
  Position b(15, 17);
  Dir diff = b - a;
  Dir capped = capDir(diff);
  return capped == Dir(1, -1);
}());

// ============================================================
// Runtime tests: Stream output (not constexpr)
// ============================================================

TEST(StreamOutput, DirFormat) {
  std::ostringstream oss;
  oss << Dir(3, -1);
  EXPECT_EQ(oss.str(), "(3,-1)");
}

TEST(StreamOutput, DirZero) {
  std::ostringstream oss;
  oss << Dir();
  EXPECT_EQ(oss.str(), "(0,0)");
}

TEST(StreamOutput, PositionFormat) {
  std::ostringstream oss;
  oss << Position(10, 20);
  EXPECT_EQ(oss.str(), "(10,20)");
}

TEST(StreamOutput, PositionNegative) {
  std::ostringstream oss;
  oss << Position(-5, -3);
  EXPECT_EQ(oss.str(), "(-5,-3)");
}

// -- FloorSpecifier --
static_assert(FloorSpecifier(0) == FloorSpecifier(0));
static_assert(!(FloorSpecifier(0) == FloorSpecifier(1)));
static_assert(FloorSpecifier(5).floor == 5);
static_assert(FloorSpecifier(3).up() == FloorSpecifier(2));
static_assert(FloorSpecifier(3).down() == FloorSpecifier(4));
static_assert(FloorSpecifier(3).up(2) == FloorSpecifier(1));
static_assert(FloorSpecifier(3).down(3) == FloorSpecifier(6));
static_assert(FloorSpecifier(5).up().down() == FloorSpecifier(5));
static_assert(FloorSpecifier(0).down().up() == FloorSpecifier(0));

// -- Location --
static_assert(Location(Position(3, 4), FloorSpecifier(5)).pos == Position(3, 4));
static_assert(Location(Position(3, 4), FloorSpecifier(5)).mapPos == FloorSpecifier(5));
static_assert(Location(Position(3, 4), FloorSpecifier(5)) == Location(Position(3, 4), FloorSpecifier(5)));
static_assert(!(Location(Position(3, 4), FloorSpecifier(5)) == Location(Position(3, 4), FloorSpecifier(6))));
static_assert(!(Location(Position(3, 4), FloorSpecifier(5)) == Location(Position(3, 5), FloorSpecifier(5))));
static_assert(!(Location(Position(3, 4), FloorSpecifier(5)) == Location(Position(4, 4), FloorSpecifier(5))));
static_assert(Location(Position(1, 2), FloorSpecifier(3)) == Location(Position(1, 2), FloorSpecifier(3)));
static_assert(Location(Position(5, 5), FloorSpecifier(3)).up() == Location(Position(5, 5), FloorSpecifier(2)));
static_assert(Location(Position(5, 5), FloorSpecifier(3)).down() == Location(Position(5, 5), FloorSpecifier(4)));
static_assert(Location(Position(5, 5), FloorSpecifier(3)).up().down() == Location(Position(5, 5), FloorSpecifier(3)));

// -- PosPathIterableShort --

TEST(PosPathIterableShortTests, SamePoint) {
  std::vector<Position> path;
  for (auto p : PosPathIterableShort(Position{3, 3}, Position{3, 3})) {
    path.push_back(p);
  }
  ASSERT_EQ(path.size(), 1u);
  EXPECT_EQ(path[0], Position(3, 3));
}

TEST(PosPathIterableShortTests, Horizontal) {
  std::vector<Position> path;
  for (auto p : PosPathIterableShort(Position{1, 0}, Position{4, 0})) {
    path.push_back(p);
  }
  ASSERT_EQ(path.size(), 4u);
  EXPECT_EQ(path[0], Position(1, 0));
  EXPECT_EQ(path[3], Position(4, 0));
  for (auto &pos : path)
    EXPECT_EQ(pos.y, 0);
}

TEST(PosPathIterableShortTests, Vertical) {
  std::vector<Position> path;
  for (auto p : PosPathIterableShort(Position{0, 1}, Position{0, 4})) {
    path.push_back(p);
  }
  ASSERT_EQ(path.size(), 4u);
  EXPECT_EQ(path[0], Position(0, 1));
  EXPECT_EQ(path[3], Position(0, 4));
  for (auto &pos : path)
    EXPECT_EQ(pos.x, 0);
}

TEST(PosPathIterableShortTests, Diagonal) {
  std::vector<Position> path;
  for (auto p : PosPathIterableShort(Position{0, 0}, Position{3, 3})) {
    path.push_back(p);
  }
  ASSERT_EQ(path.size(), 4u);
  EXPECT_EQ(path[0], Position(0, 0));
  EXPECT_EQ(path[3], Position(3, 3));
}
