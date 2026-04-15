#include "TestHeader.h"
import Game;
import Common;
import std;

TEST(WorldFloor, SeeThroughDefault) {
  WorldFloor floor(5, 5);
  EXPECT_TRUE(floor.seeThrough({2, 2}));
}

TEST(WorldFloor, SeeThroughWall) {
  WorldFloor floor(5, 5);
  floor.getTerrainType({2, 2}) = TerrainType::Wall;
  EXPECT_FALSE(floor.seeThrough({2, 2}));
}

TEST(WorldFloor, SeeThroughOutOfBounds) {
  WorldFloor floor(5, 5);
  EXPECT_FALSE(floor.seeThrough({10, 10}));
}

TEST(WorldFloor, IsOpenTerrainDefault) {
  WorldFloor floor(5, 5);
  EXPECT_TRUE(floor.isOpenTerrain({1, 1}));
}

TEST(WorldFloor, IsOpenTerrainWall) {
  WorldFloor floor(5, 5);
  floor.getTerrainType({1, 1}) = TerrainType::Wall;
  EXPECT_FALSE(floor.isOpenTerrain({1, 1}));
}

TEST(WorldFloor, IsOpenTileEmpty) {
  WorldFloor floor(5, 5);
  EXPECT_TRUE(floor.isOpenTile({1, 1}));
}

TEST(WorldFloor, IsOpenTileWall) {
  WorldFloor floor(5, 5);
  floor.getTerrainType({1, 1}) = TerrainType::Wall;
  EXPECT_FALSE(floor.isOpenTile({1, 1}));
}

TEST(WorldFloor, IsOpenTileMonsterPresent) {
  WorldFloor floor(5, 5);
  Monster::ID::Generator gen;
  floor.getMonster({1, 1}) = gen.next();
  EXPECT_FALSE(floor.isOpenTile({1, 1}));
}

TEST(WorldFloor, FindTerrainFound) {
  WorldFloor floor(5, 5);
  floor.getTerrainType({3, 2}) = TerrainType::UpStair;
  Position found = floor.findTerrain(TerrainType::UpStair);
  EXPECT_EQ(found, (Position{3, 2}));
}

TEST(WorldFloor, FindTerrainNotFound) {
  WorldFloor floor(5, 5);
  Position found = floor.findTerrain(TerrainType::UpStair);
  EXPECT_EQ(found, (Position{-1, -1}));
}

TEST(WorldFloor, EventListeners) {
  WorldFloor floor(5, 5);
  Monster::ID::Generator gen;
  Monster::ID id1 = gen.next();
  Monster::ID id2 = gen.next();

  floor.addEventListener(id1);
  floor.addEventListener(id2);
  EXPECT_EQ(floor.getEventListenersArr().size(), 2u);

  floor.removeEventListener(id1);
  EXPECT_EQ(floor.getEventListenersArr().size(), 1u);
  EXPECT_EQ(floor.getEventListenersArr()[0], id2);
}

TEST(WorldFloor, InBoundsValid) {
  WorldFloor floor(5, 5);
  EXPECT_TRUE(floor.inBounds({0, 0}));
  EXPECT_TRUE(floor.inBounds({4, 4}));
}

TEST(WorldFloor, InBoundsInvalid) {
  WorldFloor floor(5, 5);
  EXPECT_FALSE(floor.inBounds({5, 5}));
  EXPECT_FALSE(floor.inBounds({-1, 0}));
}
