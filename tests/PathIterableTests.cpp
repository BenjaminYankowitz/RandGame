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
  for (auto _ : PathIterable{e}) {
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

// -- Empty path --
static_assert(pathSize(Dir{0, 0}) == 0);

// -- Horizontal paths --
static_assert(pathSize(Dir{1, 0}) == 1);
static_assert(collectPath<1>(Dir{1, 0}) == std::array<Dir, 1>{{{1, 0}}});

static_assert(pathSize(Dir{3, 0}) == 3);
static_assert(collectPath<3>(Dir{3, 0}) ==
              std::array<Dir, 3>{{{1, 0}, {2, 0}, {3, 0}}});

// -- Vertical paths --
static_assert(pathSize(Dir{0, 1}) == 1);
static_assert(collectPath<1>(Dir{0, 1}) == std::array<Dir, 1>{{{0, 1}}});

static_assert(pathSize(Dir{0, 3}) == 3);
static_assert(collectPath<3>(Dir{0, 3}) ==
              std::array<Dir, 3>{{{0, 1}, {0, 2}, {0, 3}}});

// -- Diagonal paths --
static_assert(pathSize(Dir{2, 2}) == 2);
static_assert(collectPath<2>(Dir{2, 2}) ==
              std::array<Dir, 2>{{{1, 1}, {2, 2}}});

static_assert(pathSize(Dir{3, 3}) == 3);
static_assert(collectPath<3>(Dir{3, 3}) ==
              std::array<Dir, 3>{{{1, 1}, {2, 2}, {3, 3}}});

// -- Negative axis-aligned --
static_assert(pathSize(Dir{-3, 0}) == 3);
static_assert(collectPath<3>(Dir{-3, 0}) ==
              std::array<Dir, 3>{{{-1, 0}, {-2, 0}, {-3, 0}}});

static_assert(pathSize(Dir{0, -2}) == 2);
static_assert(collectPath<2>(Dir{0, -2}) ==
              std::array<Dir, 2>{{{0, -1}, {0, -2}}});

// -- Negative diagonal --
static_assert(pathSize(Dir{-2, -2}) == 2);
static_assert(collectPath<2>(Dir{-2, -2}) ==
              std::array<Dir, 2>{{{-1, -1}, {-2, -2}}});

// -- Size properties --
static_assert(pathSize(Dir{5, 0}) == 5);
static_assert(pathSize(Dir{-4, 0}) == 4);
static_assert(pathSize(Dir{0, 5}) == 5);
static_assert(pathSize(Dir{0, -4}) == 4);
static_assert(pathSize(Dir{4, 4}) == 4);
static_assert(pathSize(Dir{-3, -3}) == 3);

// -- First element is capDir(e) --
static_assert(*PathIterable{Dir{1, 0}}.begin() == capDir(Dir{1, 0}));
static_assert(*PathIterable{Dir{0, 1}}.begin() == capDir(Dir{0, 1}));
static_assert(*PathIterable{Dir{3, 3}}.begin() == capDir(Dir{3, 3}));
static_assert(*PathIterable{Dir{-2, -2}}.begin() == capDir(Dir{-2, -2}));
static_assert(*PathIterable{Dir{5, 3}}.begin() == capDir(Dir{5, 3}));

// -- Endpoint is included in range --
static_assert(collectPath<1>(Dir{1, 0})[0] == Dir{1, 0});
static_assert(collectPath<3>(Dir{3, 0})[2] == Dir{3, 0});
static_assert(collectPath<3>(Dir{0, 3})[2] == Dir{0, 3});
static_assert(collectPath<3>(Dir{3, 3})[2] == Dir{3, 3});
static_assert(collectPath<2>(Dir{-2, -2})[1] == Dir{-2, -2});

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

// -- Non-uniform paths now terminate (bug was fixed) --
static_assert(pathTerminates(Dir{3, 2}, 100));
static_assert(pathTerminates(Dir{3, 1}, 100));
static_assert(pathTerminates(Dir{5, 2}, 100));
static_assert(pathTerminates(Dir{1, 3}, 100));
static_assert(pathTerminates(Dir{-3, 2}, 100));

// -- Non-uniform path sequences --
static_assert(collectPath<4>(Dir{3, 2}) ==
              std::array<Dir, 4>{{{1, 1}, {2, 1}, {2, 2}, {3, 2}}});
static_assert(collectPath<3>(Dir{3, 1}) ==
              std::array<Dir, 3>{{{1, 1}, {2, 1}, {3, 1}}});

// ============================================================
// Runtime tests: invariant checks on non-uniform paths
// ============================================================

TEST(PathIterableInvariants, MonotonicityAndValidSteps3x2) {
  constexpr Dir E{3, 2};
  constexpr Dir Capped = capDir(E);

  auto path = PathIterable{E};
  auto it = path.begin();
  auto end = path.end();

  Dir prev = *it;
  EXPECT_EQ(prev, Capped);
  ++it;

  while (it != end) {
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
  }
}
