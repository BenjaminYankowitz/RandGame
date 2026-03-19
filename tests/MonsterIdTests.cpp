#include "TestHeader.h"
import Game;
import std;

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
// Monster::ID::Generator - constexpr tests
// ============================================================

// Generator produces non-null IDs
static_assert([] {
  Monster::ID::Generator gen;
  Monster::ID id = gen.next();
  return !id.isNull();
}());

// Sequential next produces distinct IDs
static_assert([] {
  Monster::ID::Generator gen;
  Monster::ID a = gen.next();
  Monster::ID b = gen.next();
  Monster::ID c = gen.next();
  return a != b && b != c && a != c;
}());

// Generated ID differs from null
static_assert([] {
  Monster::ID::Generator gen;
  Monster::ID id = gen.next();
  return id != Monster::ID::null();
}());

// ============================================================
// Monster::ID::Generator - runtime tests
// ============================================================

// StdHashWorks
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

// Generate 100 IDs, verify all unique
TEST(MonsterIdGenerator, ManyUniqueIds) {
  Monster::ID::Generator gen;
  std::unordered_set<Monster::ID> ids;
  for (int i = 0; i < 100; ++i)
    ids.insert(gen.next());
  EXPECT_EQ(ids.size(), 100u);
}
