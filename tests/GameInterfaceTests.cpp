#include "TestHeader.h"
import Game;
import GameInterface;
import GameTypes;
import Common;
import std;

namespace {
class NullEventViewer final : public EventViewerInterface {
  void itemPickup(MonsterInterface /*grabber*/, ObjectInterface /*grabbed*/) override {}
  void debug(std::string_view /*message*/) override {}
  void monsterHitMonster(HitInfo /*hitinfo*/, MonsterInterface /*attacker*/, MonsterInterface /*attacked*/) override {}
  void monsterHitWall(MonsterInterface /*attacker*/, TerrainType /*attacked*/) override {}
  void exception(const std::exception & /*e*/) noexcept override {}
};

std::unique_ptr<EventViewerInterface> makeNullViewer() {
  return std::make_unique<NullEventViewer>();
}

GameInterface makeGI(GameState &game) {
  GameInterface gi(reinterpret_cast<IGameState &>(game), game.getPlayer().getId());
  gi.setEventViewer(makeNullViewer());
  return gi;
}
} // namespace
TEST(GameInterfaceTests, ConstructionSucceeds) {
  GameState game;
  GameInterface gi = makeGI(game);
  (void)gi;
}

TEST(GameInterfaceTests, PlayerStartsAlive) {
  GameState game;
  GameInterface gi = makeGI(game);
  EXPECT_TRUE(gi.getHealth() > 0);
}

TEST(GameInterfaceTests, InitialTimeIsZero) {
  GameState game;
  GameInterface gi = makeGI(game);
  EXPECT_EQ(gi.getTime().impl, 0);
}

TEST(GameInterfaceTests, SpeedIsPositive) {
  GameState game;
  GameInterface gi = makeGI(game);
  EXPECT_TRUE(gi.getSpeed().impl > 0);
}

TEST(GameInterfaceTests, FloorHasDimensions) {
  GameState game;
  GameInterface gi = makeGI(game);
  auto floor = gi.getFloor(FloorSpecifier(0));
  EXPECT_TRUE(floor.rows() > 0);
  EXPECT_TRUE(floor.cols() > 0);
}

TEST(GameInterfaceTests, PlayerLocationInBounds) {
  GameState game;
  GameInterface gi = makeGI(game);
  auto loc = gi.getLocation();
  auto floor = gi.getFloor(loc.mapPos);
  EXPECT_TRUE(floor.inBounds(loc.pos));
}

TEST(GameInterfaceTests, InventoryAccessible) {
  GameState game;
  GameInterface gi = makeGI(game);
  auto inv = gi.lookAtInventory();
  // Player may start with items; just verify access works
  (void)inv.size();
  (void)inv.empty();
}

TEST(GameInterfaceTests, PassTimeAdvancesClock) {
  GameState game;
  GameInterface gi = makeGI(game);
  auto t0 = gi.getTime();
  gi.passTime(TimePeriod(1));
  auto t1 = gi.getTime();
  EXPECT_TRUE(t1.impl > t0.impl);
}

TEST(GameInterfaceTests, MoveChangesTimeOrPosition) {
  GameState game;
  GameInterface gi = makeGI(game);
  auto loc0 = gi.getLocation();
  auto t0 = gi.getTime();
  gi.generalMove(Dir::right(), MoveMode::move());
  auto loc1 = gi.getLocation();
  auto t1 = gi.getTime();
  EXPECT_TRUE(loc1.pos.x != loc0.pos.x || loc1.pos.y != loc0.pos.y || t1.impl > t0.impl);
}

TEST(GameInterfaceTests, PickUpItemOutOfRangeIsNoOp) {
  GameState game;
  GameInterface gi = makeGI(game);
  auto t0 = gi.getTime();
  gi.pickUpItem(999);
  auto t1 = gi.getTime();
  EXPECT_EQ(t0.impl, t1.impl);
}

TEST(GameInterfaceTests, DropItemOutOfRangeIsNoOp) {
  GameState game;
  GameInterface gi = makeGI(game);
  auto t0 = gi.getTime();
  gi.dropItem(999);
  auto t1 = gi.getTime();
  EXPECT_EQ(t0.impl, t1.impl);
}

TEST(GameInterfaceTests, ThrowItemOutOfRangeIsNoOp) {
  GameState game;
  GameInterface gi = makeGI(game);
  auto t0 = gi.getTime();
  gi.throwItem(999, Dir::up());
  auto t1 = gi.getTime();
  EXPECT_EQ(t0.impl, t1.impl);
}

TEST(GameInterfaceTests, PlayerTileIsNotWall) {
  GameState game;
  GameInterface gi = makeGI(game);
  auto loc = gi.getLocation();
  auto floor = gi.getFloor(loc.mapPos);
  auto tile = floor.getTile(loc.pos);
  ASSERT_TRUE(tile.has_value());
  EXPECT_FALSE(isWall(tile->terrainType));
}

TEST(GameInterfaceTests, PlayerTileHasPlayer) {
  GameState game;
  GameInterface gi = makeGI(game);
  auto loc = gi.getLocation();
  auto floor = gi.getFloor(loc.mapPos);
  auto tile = floor.getTile(loc.pos);
  ASSERT_TRUE(tile.has_value());
  EXPECT_FALSE(tile->monster.isNull());
  EXPECT_TRUE(tile->monster.isPlayer());
  EXPECT_TRUE(tile->monster.isAlive());
}

TEST(GameInterfaceTests, SetEventViewerWorks) {
  GameState game;
  GameInterface gi = makeGI(game);
  gi.setEventViewer(makeNullViewer());
  EXPECT_TRUE(gi.getHealth() > 0);
}

TEST(GameInterfaceTests, FloorObjectsAccessible) {
  GameState game;
  GameInterface gi = makeGI(game);
  auto floorItems = gi.lookAtFloor();
  (void)floorItems.size();
  (void)floorItems.empty();
}

TEST(GameInterfaceTests, LenOne) {
  ObjectContainer container;
  container.addObject({.type = ObjectType::KingsCoin, .mat = Material::Gold, .count = 23});
  ObjectContainerInterface iface(container);
  auto it = iface.begin();
  auto endIt = iface.end();
  EXPECT_TRUE(it != endIt);
  EXPECT_EQ(1, container.size());
  std::size_t count = 0;
  for (auto obj : container) {
    count++;
    EXPECT_EQ(obj.count(), 23);
    EXPECT_EQ(obj.type(), ObjectType::KingsCoin);
    EXPECT_EQ(obj.mat(), Material::Gold);
  }
  EXPECT_EQ(1, count);
}

TEST(GameInterfaceTests, MultiplePassTimeAdvancesCorrectly) {
  GameState game;
  GameInterface gi = makeGI(game);
  auto t0 = gi.getTime();
  gi.passTime(TimePeriod(1));
  auto t1 = gi.getTime();
  gi.passTime(TimePeriod(1));
  auto t2 = gi.getTime();
  EXPECT_TRUE(t2.impl > t1.impl);
  EXPECT_TRUE(t1.impl > t0.impl);
}
