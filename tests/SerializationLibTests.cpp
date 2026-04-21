#include <math.h>

#include <numbers>

#include "TestHeader.h"
import SerializationLib;
import Common;
using SerializationLib::Tag;

TEST(SerializationLibTests, SerializeAndDeserializeSingleInt) {
  std::stringstream ss;
  int original = 42;
  SerializationLib::toStream(ss, original);

  ss.seekg(0);
  int restored = SerializationLib::fromStream(ss, Tag<int>{});
  EXPECT_EQ(restored, original);
}

TEST(SerializationLibTests, SerializeAndDeserializeMultiple) {
  std::stringstream ss;
  int a = 123;
  double b = 3.14;
  SerializationLib::serialize(ss, a, b);

  ss.seekg(0);
  int ra = SerializationLib::fromStream(ss, Tag<int>{});
  auto rb = SerializationLib::fromStream(ss, Tag<double>{});
  EXPECT_EQ(ra, a);
  EXPECT_DOUBLE_EQ(rb, b);
}

TEST(SerializationLibTests, SerializeableConceptCheck) {
  static_assert(SerializationLib::Serializeable<int>);
  static_assert(SerializationLib::Serializeable<double>);
  static_assert(SerializationLib::Serializeable<char>);
}

TEST(SerializationLibTests, DeserializeRoundTrip) {
  std::stringstream ss;
  int a = 10;
  double b = 2.5;
  SerializationLib::serialize(ss, a, b);
  ss.seekg(0);
  int ra;
  double rb;
  SerializationLib::deserialize(ss, ra, rb);
  EXPECT_EQ(ra, a);
  EXPECT_DOUBLE_EQ(rb, b);
}

TEST(SerializationLibTests, ZeroValues) {
  std::stringstream ss;
  int zero = 0;
  double dzero = 0.0;
  SerializationLib::serialize(ss, zero, dzero);
  ss.seekg(0);
  int rz;
  double rdz;
  SerializationLib::deserialize(ss, rz, rdz);
  EXPECT_EQ(rz, 0);
  EXPECT_DOUBLE_EQ(rdz, 0.0);
}

TEST(SerializationLibTests, NegativeValues) {
  std::stringstream ss;
  int neg = -99999;
  double dneg = -std::numbers::pi;
  SerializationLib::serialize(ss, neg, dneg);
  ss.seekg(0);
  int rn;
  double rdn;
  SerializationLib::deserialize(ss, rn, rdn);
  EXPECT_EQ(rn, neg);
  EXPECT_DOUBLE_EQ(rdn, dneg);
}

TEST(SerializationLibTests, MaxIntValues) {
  std::stringstream ss;
  int maxInt = std::numeric_limits<int>::max();
  int minInt = std::numeric_limits<int>::min();
  SerializationLib::serialize(ss, maxInt, minInt);
  ss.seekg(0);
  int rmax;
  int rmin;
  SerializationLib::deserialize(ss, rmax, rmin);
  EXPECT_EQ(rmax, maxInt);
  EXPECT_EQ(rmin, minInt);
}

TEST(SerializationLibTests, CharRoundTrip) {
  std::stringstream ss;
  char c = 'Z';
  SerializationLib::toStream(ss, c);
  ss.seekg(0);
  char rc = SerializationLib::fromStream(ss, Tag<char>{});
  EXPECT_EQ(rc, 'Z');
}

TEST(SerializationLibTests, SequentialWriteRead) {
  std::stringstream ss;
  for (int i = 0; i < 10; ++i)
    SerializationLib::toStream(ss, i);
  ss.seekg(0);
  for (int i = 0; i < 10; ++i) {
    int val = SerializationLib::fromStream(ss, Tag<int>{});
    EXPECT_EQ(val, i);
  }
}

// --- std::unique_ptr ---

TEST(SerializationLibTests, UniquePtrWithValue) {
  std::stringstream ss;
  auto original = std::make_unique<int>(42);
  SerializationLib::toStream(ss, original);
  ss.seekg(0);
  auto restored = SerializationLib::fromStream(ss, Tag<std::unique_ptr<int>>{});
  ASSERT_NE(restored, nullptr);
  EXPECT_EQ(*restored, 42);
}

TEST(SerializationLibTests, UniquePtrNull) {
  std::stringstream ss;
  std::unique_ptr<int> original = nullptr;
  SerializationLib::toStream(ss, original);
  ss.seekg(0);
  auto restored = SerializationLib::fromStream(ss, Tag<std::unique_ptr<int>>{});
  EXPECT_EQ(restored, nullptr);
}

// --- std::variant ---

TEST(SerializationLibTests, VariantFirstAlternative) {
  std::stringstream ss;
  std::variant<int, double, char> original(42);
  SerializationLib::toStream(ss, original);
  ss.seekg(0);
  auto restored = SerializationLib::fromStream(ss, Tag<std::variant<int, double, char>>{});
  ASSERT_TRUE(std::holds_alternative<int>(restored));
  EXPECT_EQ(std::get<int>(restored), 42);
}

TEST(SerializationLibTests, VariantLaterAlternative) {
  std::stringstream ss;
  std::variant<int, double, char> original('Z');
  SerializationLib::toStream(ss, original);
  ss.seekg(0);
  auto restored = SerializationLib::fromStream(ss, Tag<std::variant<int, double, char>>{});
  ASSERT_TRUE(std::holds_alternative<char>(restored));
  EXPECT_EQ(std::get<char>(restored), 'Z');
}

// --- std::vector ---

TEST(SerializationLibTests, VectorOfInts) {
  std::stringstream ss;
  std::vector<int> original = {1, 2, 3, 4, 5};
  SerializationLib::toStream(ss, original);
  ss.seekg(0);
  auto restored = SerializationLib::fromStream(ss, Tag<std::vector<int>>{});
  EXPECT_EQ(restored, original);
}

TEST(SerializationLibTests, VectorEmpty) {
  std::stringstream ss;
  std::vector<int> original;
  SerializationLib::toStream(ss, original);
  ss.seekg(0);
  auto restored = SerializationLib::fromStream(ss, Tag<std::vector<int>>{});
  EXPECT_TRUE(restored.empty());
}

TEST(SerializationLibTests, VectorNested) {
  std::stringstream ss;
  std::vector<std::vector<int>> original = {{1, 2}, {3}, {4, 5, 6}};
  SerializationLib::toStream(ss, original);
  ss.seekg(0);
  auto restored = SerializationLib::fromStream(ss, Tag<std::vector<std::vector<int>>>{});
  EXPECT_EQ(restored, original);
}

// --- std::unordered_map ---

TEST(SerializationLibTests, UnorderedMapRoundTrip) {
  std::stringstream ss;
  std::unordered_map<int, int> original = {{1, 10}, {2, 20}, {3, 30}};
  SerializationLib::toStream(ss, original);
  ss.seekg(0);
  auto restored = SerializationLib::fromStream(ss, Tag<std::unordered_map<int, int>>{});
  EXPECT_EQ(restored, original);
}

TEST(SerializationLibTests, UnorderedMapEmpty) {
  std::stringstream ss;
  std::unordered_map<int, int> original;
  SerializationLib::toStream(ss, original);
  ss.seekg(0);
  auto restored = SerializationLib::fromStream(ss, Tag<std::unordered_map<int, int>>{});
  EXPECT_TRUE(restored.empty());
}

// --- std::priority_queue ---

TEST(SerializationLibTests, PriorityQueueRoundTrip) {
  std::stringstream ss;
  std::priority_queue<int> original;
  original.push(3);
  original.push(1);
  original.push(4);
  original.push(1);
  original.push(5);
  SerializationLib::toStream(ss, original);
  ss.seekg(0);
  auto restored = SerializationLib::fromStream(ss, Tag<std::priority_queue<int>>{});
  EXPECT_EQ(restored.size(), original.size());
  while (!original.empty()) {
    EXPECT_EQ(restored.top(), original.top());
    restored.pop();
    original.pop();
  }
}

// --- mixed nesting ---

TEST(SerializationLibTests, VectorOfUniquePtrs) {
  std::stringstream ss;
  std::vector<std::unique_ptr<int>> original;
  original.push_back(std::make_unique<int>(10));
  original.push_back(nullptr);
  original.push_back(std::make_unique<int>(30));
  SerializationLib::toStream(ss, original);
  ss.seekg(0);
  auto restored = SerializationLib::fromStream(ss, Tag<std::vector<std::unique_ptr<int>>>{});
  ASSERT_EQ(restored.size(), 3u);
  ASSERT_NE(restored[0], nullptr);
  EXPECT_EQ(*restored[0], 10);
  EXPECT_EQ(restored[1], nullptr);
  ASSERT_NE(restored[2], nullptr);
  EXPECT_EQ(*restored[2], 30);
}

// --- Static2DArr ---

TEST(SerializationLibTests, Static2DArrRoundTrip) {
  std::stringstream ss;
  Static2DArr<int> original(3, 4);
  for (auto [r, c] : original.indexIter()) {
    original[r, c] = static_cast<int>((r * 10) + c);
  }
  toStream(ss, original);
  ss.seekg(0);
  auto restored = fromStream(ss, Tag<Static2DArr<int>>{});
  ASSERT_EQ(restored.rows(), 3u);
  ASSERT_EQ(restored.cols(), 4u);
  for (auto [r, c] : original.indexIter()) {
    auto rVal = restored[r, c];
    auto oVal = original[r, c];
    EXPECT_EQ(rVal, oVal);
  }
}

TEST(SerializationLibTests, Static2DArrEmpty) {
  std::stringstream ss;
  Static2DArr<int> original(0, 0);
  toStream(ss, original);
  ss.seekg(0);
  auto restored = fromStream(ss, Tag<Static2DArr<int>>{});
  EXPECT_EQ(restored.rows(), 0u);
  EXPECT_EQ(restored.cols(), 0u);
  EXPECT_EQ(restored.size(), 0u);
}

TEST(SerializationLibTests, Static2DArrSingleElement) {
  std::stringstream ss;
  Static2DArr<int> original(1, 1);
  original[0, 0] = 99;
  toStream(ss, original);
  ss.seekg(0);
  auto restored = fromStream(ss, Tag<Static2DArr<int>>{});
  ASSERT_EQ(restored.rows(), 1u);
  ASSERT_EQ(restored.cols(), 1u);
  auto val = restored[0, 0];
  EXPECT_EQ(val, 99);
}

// --- StaticPositionArr ---

TEST(SerializationLibTests, StaticPositionArrRoundTrip) {
  std::stringstream ss;
  StaticPositionArr<int> original(5, 3);
  for (auto pos : original.indexIter()) {
    original[pos] = (pos.x * 100) + pos.y;
  }
  toStream(ss, original);
  ss.seekg(0);
  auto restored = fromStream(ss, Tag<StaticPositionArr<int>>{});
  ASSERT_EQ(restored.width(), 5);
  ASSERT_EQ(restored.height(), 3);
  for (auto pos : original.indexIter()) {
    EXPECT_EQ(restored[pos], original[pos]);
  }
}

TEST(SerializationLibTests, StaticPositionArrEmpty) {
  std::stringstream ss;
  StaticPositionArr<int> original(0, 0);
  toStream(ss, original);
  ss.seekg(0);
  auto restored = fromStream(ss, Tag<StaticPositionArr<int>>{});
  EXPECT_EQ(restored.width(), 0);
  EXPECT_EQ(restored.height(), 0);
}

// --- exception safety ---

TEST(SerializationLibTests, TrivialReadOnEmptyStreamThrows) {
  std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
  EXPECT_THROW(SerializationLib::fromStream(ss, Tag<int>{}), SerializationLib::DeserializationError);
}

TEST(SerializationLibTests, TrivialReadOnTruncatedStreamThrows) {
  std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
  char oneByte = 0x7F;
  ss.write(&oneByte, 1);
  EXPECT_THROW(SerializationLib::fromStream(ss, Tag<int>{}), SerializationLib::DeserializationError);
}

TEST(SerializationLibTests, VectorWithTruncatedBodyThrows) {
  std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
  std::size_t fakeSize = 5;
  SerializationLib::toStream(ss, fakeSize);
  EXPECT_THROW(SerializationLib::fromStream(ss, Tag<std::vector<int>>{}), SerializationLib::DeserializationError);
}

TEST(SerializationLibTests, VariantOOBIndexThrows) {
  using V = std::variant<int, double>;
  std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
  std::size_t badIndex = 99;
  SerializationLib::toStream(ss, badIndex);
  EXPECT_THROW(SerializationLib::fromStream(ss, Tag<V>{}), SerializationLib::DeserializationError);
}

TEST(SerializationLibTests, VariantTruncatedBeforeIndexThrows) {
  using V = std::variant<int, double>;
  std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
  EXPECT_THROW(SerializationLib::fromStream(ss, Tag<V>{}), SerializationLib::DeserializationError);
}

TEST(SerializationLibTests, StaticPositionArrNonSquare) {
  std::stringstream ss;
  StaticPositionArr<double> original(2, 7);
  for (auto pos : original.indexIter()) {
    original[pos] = (pos.x * 0.1) + pos.y;
  }
  toStream(ss, original);
  ss.seekg(0);
  auto restored = fromStream(ss, Tag<StaticPositionArr<double>>{});
  ASSERT_EQ(restored.width(), 2);
  ASSERT_EQ(restored.height(), 7);
  for (auto pos : original.indexIter()) {
    EXPECT_DOUBLE_EQ(restored[pos], original[pos]);
  }
}
