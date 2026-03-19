#include "TestHeader.h"
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
  auto rb = SerializationLib::fromStream<double>(ss);
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
  double dneg = -3.14159;
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
  char rc = SerializationLib::fromStream<char>(ss);
  EXPECT_EQ(rc, 'Z');
}

TEST(SerializationLibTests, SequentialWriteRead) {
  std::stringstream ss;
  for (int i = 0; i < 10; ++i)
    SerializationLib::toStream(ss, i);
  ss.seekg(0);
  for (int i = 0; i < 10; ++i) {
    int val = SerializationLib::fromStream<int>(ss);
    EXPECT_EQ(val, i);
  }
}
