#include <gtest/gtest.h>
import Game;
import GameTypes;


TEST(ObjectContainerTests, EmptyContainerState) {
  ObjectContainer container;
  EXPECT_EQ(container.size(), 0u);
  EXPECT_TRUE(container.empty());
}

TEST(ObjectContainerTests, AddAndRemoveObject) {
  ObjectContainer container;
  container.addObject(std::make_unique<Object>(1, ObjectType::KingsCoin, Material::Gold));
  EXPECT_EQ(container.size(), 1u);
  EXPECT_FALSE(container.empty());
  EXPECT_EQ(container[0].type(), ObjectType::KingsCoin);
  EXPECT_EQ(container[0].mat(), Material::Gold);
  EXPECT_EQ(container[0].count(), 1);

  auto removed = container.remove(0);
  EXPECT_NE(removed, nullptr);
  EXPECT_EQ(removed->type(), ObjectType::KingsCoin);
  EXPECT_EQ(container.size(), 0u);
  EXPECT_TRUE(container.empty());
}

TEST(ObjectContainerTests, AddMultipleAndRemoveSwapsLast) {
  ObjectContainer container;
  container.addObject(std::make_unique<Object>(1, ObjectType::KingsCoin, Material::Gold));
  container.addObject(std::make_unique<Object>(1, ObjectType::Knife, Material::Iron));
  EXPECT_EQ(container.size(), 2u);

  // Remove index 0 (KingsCoin) -- should swap back (Knife) into position 0
  auto removed = container.remove(0);
  EXPECT_EQ(removed->type(), ObjectType::KingsCoin);
  EXPECT_EQ(container.size(), 1u);
  EXPECT_EQ(container[0].type(), ObjectType::Knife);
  EXPECT_EQ(container[0].mat(), Material::Iron);
}

TEST(ObjectContainerTests, CombinableObjectsMerge) {
  ObjectContainer container;
  container.addObject(std::make_unique<Object>(1, ObjectType::KingsCoin, Material::Gold));
  container.addObject(std::make_unique<Object>(3, ObjectType::KingsCoin, Material::Gold));
  // Same type + material + both Normal artifact status => should merge
  EXPECT_EQ(container.size(), 1u);
  EXPECT_EQ(container[0].count(), 4);  // 1 + 3
}