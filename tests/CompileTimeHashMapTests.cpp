#include "TestHeader.h"
import Common;


static constexpr auto TestPairing = CompileTimeHashMap::to_Pairing<int, int>({
    {10, 100},
    {20, 200},
    {30, 300},
});
static constexpr auto TestMap = CompileTimeHashMap::to_Map<TestPairing, 0, 73>();

static_assert(TestMap.get(10) == 100);
static_assert(TestMap.get(20) == 200);
static_assert(TestMap.get(30) == 300);
static_assert(TestMap.get(99) == 73);  // Missing key returns null value (73)

// Trivial test so CTest registers the binary
TEST(CompileTimeHashMapTests, PositiveLookups) {
  EXPECT_EQ(TestMap.get(10), 100);
  EXPECT_EQ(TestMap.get(20), 200);
  EXPECT_EQ(TestMap.get(30), 300);
  EXPECT_EQ(TestMap.get(99), 73);
}
