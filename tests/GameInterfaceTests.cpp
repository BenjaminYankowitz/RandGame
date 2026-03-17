#include "TestHeader.h"
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

static std::unique_ptr<EventViewerInterface> makeNullViewer() {
  return std::make_unique<NullEventViewer>();
}
} // namespace
TEST(GameInterfaceTests, ConstructionSucceeds) {
  GameInterface gi(makeNullViewer());
  (void)gi;
}

TEST(GameInterfaceTests, PlayerStartsAlive) {
  GameInterface gi(makeNullViewer());
  EXPECT_TRUE(gi.getHealth() > 0);
}

TEST(GameInterfaceTests, InitialTimeIsZero) {
  GameInterface gi(makeNullViewer());
  EXPECT_EQ(gi.getTime().impl, 0);
}

TEST(GameInterfaceTests, SpeedIsPositive) {
  GameInterface gi(makeNullViewer());
  EXPECT_TRUE(gi.getSpeed().impl > 0);
}

TEST(GameInterfaceTests, FloorHasDimensions) {
  GameInterface gi(makeNullViewer());
  auto floor = gi.getFloor(FloorSpecifier(0));
  EXPECT_TRUE(floor.rows() > 0);
  EXPECT_TRUE(floor.cols() > 0);
}

TEST(GameInterfaceTests, PlayerLocationInBounds) {
  GameInterface gi(makeNullViewer());
  auto loc = gi.getLocation();
  auto floor = gi.getFloor(loc.mapPos);
  EXPECT_TRUE(floor.inBounds(loc.pos));
}

TEST(GameInterfaceTests, InventoryAccessible) {
  GameInterface gi(makeNullViewer());
  auto inv = gi.lookAtInventory();
  // Player may start with items; just verify access works
  (void)inv.size();
  (void)inv.empty();
}

TEST(GameInterfaceTests, PassTimeAdvancesClock) {
  GameInterface gi(makeNullViewer());
  auto t0 = gi.getTime();
  gi.passTime(TimePeriod(1));
  auto t1 = gi.getTime();
  EXPECT_TRUE(t1.impl > t0.impl);
}

TEST(GameInterfaceTests, MoveChangesTimeOrPosition) {
  GameInterface gi(makeNullViewer());
  auto loc0 = gi.getLocation();
  auto t0 = gi.getTime();
  gi.generalMove(Dir::right(), MoveMode::move());
  auto loc1 = gi.getLocation();
  auto t1 = gi.getTime();
  EXPECT_TRUE(loc1.pos.x != loc0.pos.x || loc1.pos.y != loc0.pos.y || t1.impl > t0.impl);
}

TEST(GameInterfaceTests, PickUpItemOutOfRangeIsNoOp) {
  GameInterface gi(makeNullViewer());
  auto t0 = gi.getTime();
  gi.pickUpItem(999);
  auto t1 = gi.getTime();
  EXPECT_EQ(t0.impl, t1.impl);
}

TEST(GameInterfaceTests, DropItemOutOfRangeIsNoOp) {
  GameInterface gi(makeNullViewer());
  auto t0 = gi.getTime();
  gi.dropItem(999);
  auto t1 = gi.getTime();
  EXPECT_EQ(t0.impl, t1.impl);
}

TEST(GameInterfaceTests, ThrowItemOutOfRangeIsNoOp) {
  GameInterface gi(makeNullViewer());
  auto t0 = gi.getTime();
  gi.throwItem(999, Dir::up());
  auto t1 = gi.getTime();
  EXPECT_EQ(t0.impl, t1.impl);
}

TEST(GameInterfaceTests, PlayerTileIsNotWall) {
  GameInterface gi(makeNullViewer());
  auto loc = gi.getLocation();
  auto floor = gi.getFloor(loc.mapPos);
  auto tile = floor.getTile(loc.pos);
  EXPECT_NE(tile.terrainType, TerrainType::Wall);
}

TEST(GameInterfaceTests, StairsAreNotWalls) {
  // Verify that stair terrain types are distinct from Wall and Empty
  EXPECT_NE(TerrainType::UpStair, TerrainType::Wall);
  EXPECT_NE(TerrainType::DownStair, TerrainType::Wall);
  EXPECT_NE(TerrainType::UpStair, TerrainType::Empty);
  EXPECT_NE(TerrainType::DownStair, TerrainType::Empty);
  EXPECT_NE(TerrainType::UpStair, TerrainType::DownStair);
}

TEST(GameInterfaceTests, PlayerTileHasPlayer) {
  GameInterface gi(makeNullViewer());
  auto loc = gi.getLocation();
  auto floor = gi.getFloor(loc.mapPos);
  auto tile = floor.getTile(loc.pos);
  EXPECT_FALSE(tile.monster.isNull());
  EXPECT_TRUE(tile.monster.isPlayer());
  EXPECT_TRUE(tile.monster.isAlive());
}

TEST(GameInterfaceTests, SetEventViewerWorks) {
  GameInterface gi(makeNullViewer());
  gi.setEventViewer(makeNullViewer());
  EXPECT_TRUE(gi.getHealth() > 0);
}

TEST(GameInterfaceTests, FloorObjectsAccessible) {
  GameInterface gi(makeNullViewer());
  auto floorItems = gi.lookAtFloor();
  (void)floorItems.size();
  (void)floorItems.empty();
}

TEST(GameInterfaceTests, LenOne) {
  ObjectContainer container;
  container.addObject(std::make_unique<Object>(23, ObjectType::KingsCoin, Material::Gold));
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
