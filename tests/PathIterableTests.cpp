#include "TestHeader.h"
import Common;

// -- Constexpr helpers --
template <std::size_t N>
constexpr auto collectPath(Dir e) {
  std::array<Dir, N> result{};
  std::size_t i = 0;
  for (auto d : PathIterable{e}) {
    if (i >= N)
      break;
    result[i++] = d;
  }
  return result;
}

constexpr std::size_t pathSize(Dir e) {
  std::size_t count = 0;
  for ([[maybe_unused]] auto d : PathIterable{e}) {
    ++count;
  }
  return count;
}

constexpr bool pathTerminates(Dir e, int maxSteps) {
  auto path = PathIterable{e};
  auto it = path.begin();
  auto end = path.end();
  int steps = 0;
  while (it != end && steps < maxSteps) {
    ++it;
    ++steps;
  }
  return it == end;
}

// ============================================================
// Compile-time tests: working cases (axis-aligned & diagonal)
// ============================================================

// -- Empty path --
static_assert(pathSize(Dir{0, 0}) == 0);

// -- Horizontal paths --
static_assert(pathSize(Dir{1, 0}) == 2);
static_assert(collectPath<2>(Dir{1, 0}) == std::array<Dir, 2>{{{0, 0}, {1, 0}}});

static_assert(pathSize(Dir{3, 0}) == 4);
static_assert(collectPath<4>(Dir{3, 0}) ==
              std::array<Dir, 4>{{{0, 0}, {1, 0}, {2, 0}, {3, 0}}});

// -- Vertical paths --
static_assert(pathSize(Dir{0, 1}) == 2);
static_assert(collectPath<2>(Dir{0, 1}) == std::array<Dir, 2>{{{0, 0}, {0, 1}}});

static_assert(pathSize(Dir{0, 3}) == 4);
static_assert(collectPath<4>(Dir{0, 3}) ==
              std::array<Dir, 4>{{{0, 0}, {0, 1}, {0, 2}, {0, 3}}});

// -- Diagonal paths --
static_assert(pathSize(Dir{2, 2}) == 3);
static_assert(collectPath<3>(Dir{2, 2}) ==
              std::array<Dir, 3>{{{0, 0}, {1, 1}, {2, 2}}});

static_assert(pathSize(Dir{3, 3}) == 4);
static_assert(collectPath<4>(Dir{3, 3}) ==
              std::array<Dir, 4>{{{0, 0}, {1, 1}, {2, 2}, {3, 3}}});

// -- Negative axis-aligned --
static_assert(pathSize(Dir{-3, 0}) == 4);
static_assert(collectPath<4>(Dir{-3, 0}) ==
              std::array<Dir, 4>{{{0, 0}, {-1, 0}, {-2, 0}, {-3, 0}}});

static_assert(pathSize(Dir{0, -2}) == 3);
static_assert(collectPath<3>(Dir{0, -2}) ==
              std::array<Dir, 3>{{{0, 0}, {0, -1}, {0, -2}}});

// -- Negative diagonal --
static_assert(pathSize(Dir{-2, -2}) == 3);
static_assert(collectPath<3>(Dir{-2, -2}) ==
              std::array<Dir, 3>{{{0, 0}, {-1, -1}, {-2, -2}}});

// -- Size properties --
static_assert(pathSize(Dir{5, 0}) == 6);
static_assert(pathSize(Dir{-4, 0}) == 5);
static_assert(pathSize(Dir{0, 5}) == 6);
static_assert(pathSize(Dir{0, -4}) == 5);
static_assert(pathSize(Dir{4, 4}) == 5);
static_assert(pathSize(Dir{-3, -3}) == 4);

// -- First element is always {0,0} --
static_assert(*PathIterable{Dir{1, 0}}.begin() == Dir{0, 0});
static_assert(*PathIterable{Dir{0, 1}}.begin() == Dir{0, 0});
static_assert(*PathIterable{Dir{3, 3}}.begin() == Dir{0, 0});
static_assert(*PathIterable{Dir{-2, -2}}.begin() == Dir{0, 0});

// -- Endpoint is included in range --
static_assert(collectPath<2>(Dir{1, 0})[1] == Dir{1, 0});
static_assert(collectPath<4>(Dir{3, 0})[3] == Dir{3, 0});
static_assert(collectPath<4>(Dir{0, 3})[3] == Dir{0, 3});
static_assert(collectPath<4>(Dir{3, 3})[3] == Dir{3, 3});
static_assert(collectPath<3>(Dir{-2, -2})[2] == Dir{-2, -2});

// -- Each consecutive step is a valid move --
static_assert([] {
  constexpr Dir E{3, 0};
  Dir capped = capDir(E);
  Dir prev{0, 0};
  bool first = true;
  for (auto d : PathIterable{E}) {
    if (first) {
      first = false;
      prev = d;
      continue;
    }
    Dir step{d.dx - prev.dx, d.dy - prev.dy};
    bool validStep = step == Dir{capped.dx, 0} || step == Dir{0, capped.dy} ||
                     step == capped;
    if (!validStep)
      return false;
    prev = d;
  }
  return true;
}());

static_assert([] {
  constexpr Dir E{3, 3};
  Dir capped = capDir(E);
  Dir prev{0, 0};
  bool first = true;
  for (auto d : PathIterable{E}) {
    if (first) {
      first = false;
      prev = d;
      continue;
    }
    Dir step{d.dx - prev.dx, d.dy - prev.dy};
    bool validStep = step == Dir{capped.dx, 0} || step == Dir{0, capped.dy} ||
                     step == capped;
    if (!validStep)
      return false;
    prev = d;
  }
  return true;
}());

// -- Termination (axis-aligned and diagonal) --
static_assert(pathTerminates(Dir{3, 0}, 100));
static_assert(pathTerminates(Dir{0, 3}, 100));
static_assert(pathTerminates(Dir{-3, 0}, 100));
static_assert(pathTerminates(Dir{0, -3}, 100));
static_assert(pathTerminates(Dir{3, 3}, 100));
static_assert(pathTerminates(Dir{-2, -2}, 100));
static_assert(pathTerminates(Dir{4, -4}, 100));
static_assert(pathTerminates(Dir{-1, 1}, 100));

// ============================================================
// Runtime tests: non-uniform paths (expose dx/dy swap bug)
// ============================================================

TEST(PathIterableBug, NonUniform3x2) {
  EXPECT_TRUE(pathTerminates(Dir{3, 2}, 100));
}

TEST(PathIterableBug, NonUniform3x1) {
  EXPECT_TRUE(pathTerminates(Dir{3, 1}, 100));
}

TEST(PathIterableBug, NonUniform5x2) {
  EXPECT_TRUE(pathTerminates(Dir{5, 2}, 100));
}

TEST(PathIterableBug, NonUniform1x3) {
  EXPECT_TRUE(pathTerminates(Dir{1, 3}, 100));
}

TEST(PathIterableBug, NonUniformNeg3x2) {
  EXPECT_TRUE(pathTerminates(Dir{-3, 2}, 100));
}

// ============================================================
// Runtime tests: invariant checks on prefix of non-terminating paths
// ============================================================

TEST(PathIterableInvariants, MonotonicityAndValidSteps3x2) {
  constexpr Dir E{3, 2};
  constexpr Dir Capped = capDir(E);
  constexpr int MaxSteps = 20;

  auto path = PathIterable{E};
  auto it = path.begin();
  auto end = path.end();

  Dir prev = *it;
  EXPECT_EQ(prev, (Dir{0, 0}));
  ++it;

  int steps = 0;
  while (it != end && steps < MaxSteps) {
    Dir cur = *it;

    // Monotonicity: dx should only increase (positive ex)
    EXPECT_GE(cur.dx, prev.dx);
    // Monotonicity: dy should only increase (positive ey)
    EXPECT_GE(cur.dy, prev.dy);

    // Each step should be one of the three valid moves
    Dir step{cur.dx - prev.dx, cur.dy - prev.dy};
    bool validStep = step == Dir{Capped.dx, 0} || step == Dir{0, Capped.dy} ||
                     step == Capped;
    EXPECT_TRUE(validStep);

    prev = cur;
    ++it;
    ++steps;
  }
}

// ============================================================
// Runtime tests: works with views::drop(1)
// ============================================================

TEST(PathIterableDrop, DropFirstElement) {
  auto path = PathIterable{Dir{3, 0}};
  std::vector<Dir> dropped;
  for (auto d : path | std::views::drop(1)) {
    dropped.push_back(d);
  }
  EXPECT_EQ(dropped.size(), 3u);
  EXPECT_EQ(dropped[0], (Dir{1, 0}));
  EXPECT_EQ(dropped[1], (Dir{2, 0}));
  EXPECT_EQ(dropped[2], (Dir{3, 0}));
}
