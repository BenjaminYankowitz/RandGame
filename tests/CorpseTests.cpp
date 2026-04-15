#include "TestHeader.h"
import GameTypes;
import std;

// isCorpse(corpseOf(Human)) is true
static_assert(isCorpse(corpseOf(MonsterClass::Human)));

// corpseOfWhat round-trips for each MonsterClass
static_assert(corpseOfWhat(corpseOf(MonsterClass::Human)) == MonsterClass::Human);
static_assert(corpseOfWhat(corpseOf(MonsterClass::Cat)) == MonsterClass::Cat);
static_assert(corpseOfWhat(corpseOf(MonsterClass::SeaSlug)) == MonsterClass::SeaSlug);
static_assert(corpseOfWhat(corpseOf(MonsterClass::GreedyWeasel)) == MonsterClass::GreedyWeasel);
static_assert(corpseOfWhat(corpseOf(MonsterClass::Bryozoan)) == MonsterClass::Bryozoan);
static_assert(corpseOfWhat(corpseOf(MonsterClass::Imp)) == MonsterClass::Imp);

// isCorpse(KingsCoin) is false
static_assert(!isCorpse(ObjectType::KingsCoin));

// corpseOf equality/inequality
static_assert(corpseOf(MonsterClass::Human) == corpseOf(MonsterClass::Human));
static_assert(corpseOf(MonsterClass::Human) != corpseOf(MonsterClass::Cat));

// defaultMat(corpseOf(...)) returns Flesh
static_assert(defaultMat(corpseOf(MonsterClass::Human)) == Material::Flesh);
static_assert(defaultMat(corpseOf(MonsterClass::Cat)) == Material::Flesh);
