#include "TestHeader.h"
import GameState;
import GameTypes;
import std;

static_assert([] consteval {
  ObjectContainer container;
  if (container.size() != 0u)
    return false;
  if (!container.empty())
    return false;
  return true;
}());

static_assert([] consteval {
  ObjectContainer container;
  container.addObject({.type = ObjectType::KingsCoin, .mat = Material::Gold});
  if (container.size() != 1u)
    return false;
  if (container.empty())
    return false;
  if (container[0].type() != ObjectType::KingsCoin)
    return false;
  if (container[0].mat() != Material::Gold)
    return false;
  if (container[0].count() != 1)
    return false;

  auto removed = container.remove(0);
  if (removed == nullptr)
    return false;
  if (removed->type() != ObjectType::KingsCoin)
    return false;
  if (container.size() != 0u)
    return false;
  if (!container.empty())
    return false;
  return true;
}());

static_assert([] consteval {
  ObjectContainer container;
  container.addObject({.type = ObjectType::KingsCoin, .mat = Material::Gold, .count = 5});
  container.addObject({.type = ObjectType::Knife, .mat = Material::Iron, .count = 5});
  if (container.size() != 2u)
    return false;

  // Remove index 0 (KingsCoin) -- because (Knife) is only object left it should be in position 0
  auto removed = container.remove(0);
  if (removed->type() != ObjectType::KingsCoin)
    return false;
  if (container.size() != 1u)
    return false;
  if (container[0].type() != ObjectType::Knife)
    return false;
  if (container[0].mat() != Material::Iron)
    return false;
  if (container[0].count() != 5)
    return false;
  return true;
}());

// Corpses of different creatures do not stack
static_assert([] consteval {
  ObjectContainer container;
  container.addObject(mkCorpseBluePrint(MonsterClass::Human));
  container.addObject(mkCorpseBluePrint(MonsterClass::Cat));
  if (container.size() != 2u)
    return false;
  if (container[0].type() != ObjectType::Corpse)
    return false;
  if (container[1].type() != ObjectType::Corpse)
    return false;
  return true;
}());

TEST(ObjectContainerTests, CombinableObjectsMerge) {
  ObjectContainer container;
  container.addObject({.type = ObjectType::KingsCoin, .count = 3});
  container.addObject({.type = ObjectType::KingsCoin});
  // Same type + material + both Normal artifact status => should merge
  EXPECT_EQ(container.size(), 1u);
  EXPECT_EQ(container[0].count(), 4); // 1 + 3
}