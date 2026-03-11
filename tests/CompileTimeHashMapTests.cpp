#include <gtest/gtest.h>
import Common;

// Bug 2: CompileTimeHashMap stores `true` instead of the actual key in getHashMapLen().
// Line 52: `used[index] = true;` should be `used[index] = i.key;`
// This breaks duplicate key detection for keys != 1.
// See: src/Common/CompileTimeHashMap.cxx line 52

// Positive test: unique keys should work correctly.
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
