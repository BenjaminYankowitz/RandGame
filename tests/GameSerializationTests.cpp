#include "TestHeader.h"
import GameState;
import SerializationLib;
import Common;
import NullEventViewer;
import std;
using SerializationLib::fromStream;
using SerializationLib::Tag;

// --- Monster ---

TEST(GameSerializationTests, MonsterRoundTrip) {
  GameState game;
  game.generateGame();
  const Monster &player = game.getPlayer();

  std::stringstream ss;
  toStream(ss, player);
  ss.seekg(0);
  auto restored = fromStream(ss, Tag<Monster>{});

  EXPECT_EQ(restored.getClass(), player.getClass());
  EXPECT_EQ(restored.getLoc(), player.getLoc());
  EXPECT_EQ(restored.getHealth(), player.getHealth());
  EXPECT_EQ(restored.getMaxHealth(), player.getMaxHealth());
  EXPECT_EQ(restored.getId(), player.getId());
  EXPECT_EQ(restored.isAlive(), player.isAlive());
  EXPECT_EQ(restored.isPlayer(), player.isPlayer());
}

TEST(GameSerializationTests, MonsterWithInventory) {
  GameState game;
  game.generateGame();
  game.setEventViewer(std::make_unique<NullEventViewer>());
  Monster &player = game.getPlayer();
  auto &objects = game.getObjects(player.getLoc());
  objects.addObject(ObjectBluePrint{ObjectType::KingsCoin, Material::Gold});
  (void)player.takeItem(game, objects, 0);

  std::stringstream ss;
  toStream(ss, player);
  ss.seekg(0);
  auto restored = fromStream(ss, Tag<Monster>{});

  EXPECT_EQ(restored.viewInventory().size(), player.viewInventory().size());
  EXPECT_EQ(restored.viewInventory()[0].type(), player.viewInventory()[0].type());
  EXPECT_EQ(restored.viewInventory()[0].mat(), player.viewInventory()[0].mat());
}

TEST(GameSerializationTests, MonsterBytesWritten) {
  GameState game;
  game.generateGame();
  const Monster &player = game.getPlayer();

  std::stringstream ss;
  toStream(ss, player);
  EXPECT_GT(static_cast<std::streamsize>(ss.tellp()), 0);
}

// --- WorldFloor ---

TEST(GameSerializationTests, WorldFloorRoundTrip) {
  GameState game;
  game.generateGame();
  const auto &floor = game.getFloor(FloorSpecifier(0));

  std::stringstream ss;
  toStream(ss, floor);
  ss.seekg(0);
  auto restored = fromStream(ss, Tag<WorldFloor>{});

  EXPECT_EQ(restored.cols(), floor.cols());
  EXPECT_EQ(restored.rows(), floor.rows());

  for (auto pos : floor.getTerrainTypeArr().indexIter()) {
    EXPECT_EQ(restored.getTerrainType(pos), floor.getTerrainType(pos));
  }
}

TEST(GameSerializationTests, WorldFloorPreservesMonsterIds) {
  GameState game;
  game.generateGame();
  const auto &floor = game.getFloor(game.getPlayer().getLoc().mapPos);
  auto playerPos = game.getPlayer().getLoc().pos;

  std::stringstream ss;
  toStream(ss, floor);
  ss.seekg(0);
  auto restored = fromStream(ss, Tag<WorldFloor>{});

  EXPECT_EQ(restored.getMonster(playerPos), floor.getMonster(playerPos));
}

TEST(GameSerializationTests, WorldFloorPreservesObjects) {
  GameState game;
  game.generateGame();
  auto loc = game.getPlayer().getLoc();
  game.getObjects(loc).addObject(ObjectBluePrint{ObjectType::Knife, Material::Iron});
  const auto &floor = game.getFloor(loc.mapPos);

  std::stringstream ss;
  toStream(ss, floor);
  ss.seekg(0);
  auto restored = fromStream(ss, Tag<WorldFloor>{});

  EXPECT_EQ(restored.getObjects(loc.pos).size(), floor.getObjects(loc.pos).size());
  EXPECT_EQ(restored.getObjects(loc.pos)[0].type(), floor.getObjects(loc.pos)[0].type());
}

// --- GameState ---

TEST(GameSerializationTests, GameStateRoundTrip) {
  GameState game;
  game.generateGame();
  auto playerLoc = game.getPlayer().getLoc();
  auto playerId = game.getPlayer().getId();
  auto playerHealth = game.getPlayer().getHealth();
  auto time = game.getTime();

  std::stringstream ss;
  toStream(ss, game);
  ss.seekg(0);
  auto restored = fromStream(ss, Tag<GameState>{});

  EXPECT_EQ(restored.getTime(), time);
  EXPECT_TRUE(restored.containsMonster(playerId));
  auto &restoredPlayer = restored.getMonster(playerId);
  EXPECT_EQ(restoredPlayer.getLoc(), playerLoc);
  EXPECT_EQ(restoredPlayer.getHealth(), playerHealth);
  EXPECT_EQ(restoredPlayer.isPlayer(), true);
}

TEST(GameSerializationTests, GameStatePreservesFloors) {
  GameState game;
  game.generateGame();
  auto playerFloor = game.getPlayer().getLoc().mapPos;
  const auto &origFloor = game.getFloor(playerFloor);

  std::stringstream ss;
  toStream(ss, game);
  ss.seekg(0);
  auto restored = fromStream(ss, Tag<GameState>{});

  const auto &restoredFloor = restored.getFloor(playerFloor);
  EXPECT_EQ(restoredFloor.rows(), origFloor.rows());
  EXPECT_EQ(restoredFloor.cols(), origFloor.cols());
  for (auto pos : origFloor.getTerrainTypeArr().indexIter()) {
    EXPECT_EQ(restoredFloor.getTerrainType(pos), origFloor.getTerrainType(pos));
  }
}

TEST(GameSerializationTests, GameStatePreservesAllMonsters) {
  GameState game;
  game.generateGame();

  std::stringstream ss;
  toStream(ss, game);
  ss.seekg(0);
  auto restored = fromStream(ss, Tag<GameState>{});

  // Check that all monsters on floor 0 are present in the restored state
  const auto &floor = game.getFloor(FloorSpecifier(0));
  for (auto pos : floor.getMonsterArr().indexIter()) {
    auto id = floor.getMonster(pos);
    if (!id.isNull()) {
      ASSERT_TRUE(restored.containsMonster(id));
      EXPECT_EQ(restored.getMonster(id).getLoc(), game.getMonster(id).getLoc());
      EXPECT_EQ(restored.getMonster(id).getClass(), game.getMonster(id).getClass());
    }
  }
}

TEST(GameSerializationTests, GameStatePreservesObjects) {
  GameState game;
  game.generateGame();
  auto loc = game.getPlayer().getLoc();
  game.getObjects(loc).addObject(ObjectBluePrint{ObjectType::KingsCoin, Material::Gold});

  std::stringstream ss;
  toStream(ss, game);
  ss.seekg(0);
  auto restored = fromStream(ss, Tag<GameState>{});

  EXPECT_EQ(restored.getObjects(loc).size(), 1u);
  EXPECT_EQ(restored.getObjects(loc)[0].type(), ObjectType::KingsCoin);
  EXPECT_EQ(restored.getObjects(loc)[0].mat(), Material::Gold);
}
