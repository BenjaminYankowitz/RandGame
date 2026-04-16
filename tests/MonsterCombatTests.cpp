#include "TestHeader.h"
import GameState;
import Common;
import MonsterClassConfig;
using namespace Dice::Literals;

namespace {
Monster makeMonster() {
  return Monster(Monster::MonsterBodyInit::make(MonsterClassInfoArr[MonsterClass::Human]),
                 Location({0, 0}, FloorSpecifier(0)), MonsterID::null(), MonsterBrainInit{});
}
} // namespace

// removeHealth(5) on 10HP monster: returns false, health=5
TEST(MonsterCombat, RemoveHealthPartial) {
  auto m = makeMonster();
  EXPECT_FALSE(m.removeHealth(5));
  EXPECT_EQ(m.getHealth(), 5);
}

// removeHealth(10) on 10HP monster: returns true (killed)
TEST(MonsterCombat, RemoveHealthKill) {
  auto m = makeMonster();
  EXPECT_TRUE(m.removeHealth(10));
}

// removeHealth(15) on 10HP immortal: returns false, health restored to max
TEST(MonsterCombat, RemoveHealthImmortalLethal) {
  auto m = makeMonster();
  m.setImmortal();
  EXPECT_FALSE(m.removeHealth(15));
  EXPECT_EQ(m.getHealth(), 10);
}

// removeHealth(5) on 10HP immortal: returns false, health=5 (non-lethal, no restore)
TEST(MonsterCombat, RemoveHealthImmortalNonLethal) {
  auto m = makeMonster();
  m.setImmortal();
  EXPECT_FALSE(m.removeHealth(5));
  EXPECT_EQ(m.getHealth(), 5);
}
