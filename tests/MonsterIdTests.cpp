#include <gtest/gtest.h>
import Game;

// ============================================================
// Monster::ID - constexpr tests
// ============================================================

// Default-constructed isNull
static_assert(Monster::ID().isNull());

// ID::null() isNull
static_assert(Monster::ID::null().isNull());

// clear() makes ID null
static_assert([] {
  Monster::ID id;
  id.clear();
  return id.isNull();
}());

// null IDs are equal
static_assert(Monster::ID() == Monster::ID::null());

// ============================================================
// Monster::ID::Generator - runtime tests
// ============================================================

TEST(MonsterIdGenerator, NextProducesNonNull) {
  Monster::ID::Generator gen;
  Monster::ID id = gen.next();
  EXPECT_FALSE(id.isNull());
}

TEST(MonsterIdGenerator, SequentialNextProducesDistinctIds) {
  Monster::ID::Generator gen;
  Monster::ID a = gen.next();
  Monster::ID b = gen.next();
  Monster::ID c = gen.next();
  EXPECT_NE(a, b);
  EXPECT_NE(b, c);
  EXPECT_NE(a, c);
}

TEST(MonsterIdGenerator, StdHashWorks) {
  Monster::ID::Generator gen;
  std::unordered_map<Monster::ID, int> map;
  Monster::ID a = gen.next();
  Monster::ID b = gen.next();
  map[a] = 1;
  map[b] = 2;
  EXPECT_EQ(map[a], 1);
  EXPECT_EQ(map[b], 2);
  EXPECT_EQ(map.size(), 2u);
}

TEST(MonsterIdGenerator, NextDiffersFromNull) {
  Monster::ID::Generator gen;
  Monster::ID id = gen.next();
  EXPECT_NE(id, Monster::ID::null());
}
