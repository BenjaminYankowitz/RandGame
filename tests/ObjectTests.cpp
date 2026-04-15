#include "TestHeader.h"
import Game;
import GameTypes;
import std;

// isCombinable() returns true for Normal artifacts
static_assert([] consteval {
  Object obj(ObjectBluePrint{.type = ObjectType::KingsCoin});
  return obj.isCombinable();
}());

// canCombine() true for same type + material + both Normal
static_assert([] consteval {
  Object a(ObjectBluePrint{.type = ObjectType::KingsCoin, .mat = Material::Gold});
  Object b(ObjectBluePrint{.type = ObjectType::KingsCoin, .mat = Material::Gold});
  return a.canCombine(b);
}());

// canCombine() false for different type
static_assert([] consteval {
  Object a(ObjectBluePrint{.type = ObjectType::KingsCoin});
  Object b(ObjectBluePrint{.type = ObjectType::Knife});
  return !a.canCombine(b);
}());

// canCombine() false for different material
static_assert([] consteval {
  Object a(ObjectBluePrint{.type = ObjectType::KingsCoin, .mat = Material::Gold});
  Object b(ObjectBluePrint{.type = ObjectType::KingsCoin, .mat = Material::Iron});
  return !a.canCombine(b);
}());

// combine() merges counts correctly
static_assert([] consteval {
  auto a = std::make_unique<Object>(ObjectBluePrint{.type = ObjectType::KingsCoin, .count = 3});
  auto b = std::make_unique<Object>(ObjectBluePrint{.type = ObjectType::KingsCoin, .count = 2});
  a->combine(std::move(b));
  return a->count() == 5;
}());

// split(n) produces correct counts on both halves
static_assert([] consteval {
  auto obj = std::make_unique<Object>(ObjectBluePrint{.type = ObjectType::Knife, .mat = Material::Iron, .count = 7});
  auto split = obj->split(3);
  if (obj->count() != 4)
    return false;
  if (split->count() != 3)
    return false;
  // Preserves type, material, artifact
  if (split->type() != ObjectType::Knife)
    return false;
  if (split->mat() != Material::Iron)
    return false;
  if (split->artifactStatus() != ArtifactId::Normal)
    return false;
  return true;
}());
