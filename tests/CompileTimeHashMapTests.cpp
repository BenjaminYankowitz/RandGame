#include "TestHeader.h"
import Common;

// Original 3-entry int->int map
static constexpr auto TestPairing = CompileTimeHashMap::to_Pairing<int, int>({
    {10, 100},
    {20, 200},
    {30, 300},
});
static constexpr auto TestMap = CompileTimeHashMap::to_Map<TestPairing, 0, 73>();

static_assert(TestMap.get(10) == 100);
static_assert(TestMap.get(20) == 200);
static_assert(TestMap.get(30) == 300);
static_assert(TestMap.get(99) == 73); // Missing key returns null value

// Single-entry map edge case
static constexpr auto SinglePairing = CompileTimeHashMap::to_Pairing<int, int>({
    {42, 999},
});
static constexpr auto SingleMap = CompileTimeHashMap::to_Map<SinglePairing, 0, -1>();

static_assert(SingleMap.get(42) == 999);
static_assert(SingleMap.get(1) == -1);

// Larger map (10 entries) to exercise collision resolution
static constexpr auto LargePairing = CompileTimeHashMap::to_Pairing<int, int>({
    {1, 10},
    {2, 20},
    {3, 30},
    {4, 40},
    {5, 50},
    {6, 60},
    {7, 70},
    {8, 80},
    {9, 90},
    {10, 100},
});
static constexpr auto LargeMap = CompileTimeHashMap::to_Map<LargePairing, 0, -1>();

static_assert(LargeMap.get(1) == 10);
static_assert(LargeMap.get(5) == 50);
static_assert(LargeMap.get(10) == 100);
static_assert(LargeMap.get(0) == -1);  // null key returns null value
static_assert(LargeMap.get(11) == -1); // missing key
