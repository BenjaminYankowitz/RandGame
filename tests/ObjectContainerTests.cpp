#include "TestHeader.h"
import Game;
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
  container.addObject(std::make_unique<Object>(1, ObjectType::KingsCoin, Material::Gold));
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
  container.addObject(std::make_unique<Object>(3, ObjectType::KingsCoin, Material::Gold));
  container.addObject(std::make_unique<Object>(5, ObjectType::Knife, Material::Iron));
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

TEST(ObjectContainerTests, CombinableObjectsMerge) {
  ObjectContainer container;
  container.addObject(std::make_unique<Object>(1, ObjectType::KingsCoin, Material::Gold));
  container.addObject(std::make_unique<Object>(3, ObjectType::KingsCoin, Material::Gold));
  // Same type + material + both Normal artifact status => should merge
  EXPECT_EQ(container.size(), 1u);
  EXPECT_EQ(container[0].count(), 4); // 1 + 3
}