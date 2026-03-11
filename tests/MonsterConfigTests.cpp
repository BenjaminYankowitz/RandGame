#include <gtest/gtest.h>
import MonsterClassConfig;
import GameTypes;
import Common;

// ============================================================
// MonsterBrain - Player brain (exported constant)
// ============================================================

static_assert(PlayerBrain.isPlayer());
static_assert(PlayerBrain.caresItemPickup());
static_assert(PlayerBrain.caresMonsterAttack());
static_assert(PlayerBrain.caresEvent());

// MonsterBrain - Setters mutate correctly
static_assert([] {
  MonsterBrain b = MonsterClassInfoArr[MonsterClass::Human].brain;
  b.setIsPlayer(true);
  return b.isPlayer();
}());

static_assert([] {
  MonsterBrain b = MonsterClassInfoArr[MonsterClass::Human].brain;
  b.setHatesItemPickups(true);
  return b.hatesItemPickup();
}());
