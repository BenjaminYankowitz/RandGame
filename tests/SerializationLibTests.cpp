#include <gtest/gtest.h>
import SerializationLib;
import std;


TEST(SerializationLibTests, SerializeAndDeserializeSingleInt) {
  std::stringstream ss;
  int original = 42;
  SerializationLib::toStream(ss, original);

  ss.seekg(0);
  int restored = SerializationLib::fromStream<int>(ss);
  EXPECT_EQ(restored, original);
}

TEST(SerializationLibTests, SerializeAndDeserializeMultiple) {
  std::stringstream ss;
  int a = 123;
  double b = 3.14;
  SerializationLib::serialize(ss, a, b);

  ss.seekg(0);
  int ra = SerializationLib::fromStream<int>(ss);
  double rb = SerializationLib::fromStream<double>(ss);
  EXPECT_EQ(ra, a);
  EXPECT_DOUBLE_EQ(rb, b);
}

TEST(SerializationLibTests, SerializeableConceptCheck) {
  static_assert(SerializationLib::Serializeable<int>);
  static_assert(SerializationLib::Serializeable<double>);
  static_assert(SerializationLib::Serializeable<char>);
}

TEST(SerializationLibTests, DeserializeIsBroken) {
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
