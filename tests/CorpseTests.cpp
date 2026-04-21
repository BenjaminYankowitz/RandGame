#include "TestHeader.h"
import GameTypes;
import std;

// A corpse blueprint produces an Object of type Corpse
static_assert(Object(mkCorpseBluePrint(MonsterClass::Human)).type() == ObjectType::Corpse);

// corpseOf() round-trips the MonsterClass for each value
static_assert(Object(mkCorpseBluePrint(MonsterClass::Human)).corpseOf() == MonsterClass::Human);
static_assert(Object(mkCorpseBluePrint(MonsterClass::Cat)).corpseOf() == MonsterClass::Cat);
static_assert(Object(mkCorpseBluePrint(MonsterClass::SeaSlug)).corpseOf() == MonsterClass::SeaSlug);
static_assert(Object(mkCorpseBluePrint(MonsterClass::GreedyWeasel)).corpseOf() == MonsterClass::GreedyWeasel);
static_assert(Object(mkCorpseBluePrint(MonsterClass::Bryozoan)).corpseOf() == MonsterClass::Bryozoan);
static_assert(Object(mkCorpseBluePrint(MonsterClass::Imp)).corpseOf() == MonsterClass::Imp);

// Non-corpse object types are not Corpse
static_assert(ObjectType::KingsCoin != ObjectType::Corpse);

// Corpses of the same monster compare equal on both type and corpseOf
static_assert([] consteval {
  Object a(mkCorpseBluePrint(MonsterClass::Human));
  Object b(mkCorpseBluePrint(MonsterClass::Human));
  return a.type() == b.type() && a.corpseOf() == b.corpseOf();
}());

// Corpses of different monsters differ on corpseOf
static_assert([] consteval {
  Object a(mkCorpseBluePrint(MonsterClass::Human));
  Object b(mkCorpseBluePrint(MonsterClass::Cat));
  return a.type() == b.type() && a.corpseOf() != b.corpseOf();
}());

// defaultMat(Corpse) is Flesh
static_assert(defaultMat(ObjectType::Corpse) == Material::Flesh);
