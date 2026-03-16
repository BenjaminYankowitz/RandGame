#include "TestHeader.h"
import Common;

template <class T>
concept acceptsUnevenArray = requires { T{{42}, {32, 53}}; };
static_assert(!acceptsUnevenArray<Static2DArr<int>>);

static_assert([] consteval {
  Static2DArr<int> arr = {{1, 2, 3}, {4, 5, 6}};
  if ((arr[0, 0]) != 1)
    return false;
  if ((arr[0, 1]) != 2)
    return false;
  if ((arr[0, 2]) != 3)
    return false;
  if ((arr[1, 0]) != 4)
    return false;
  if ((arr[1, 1]) != 5)
    return false;
  if ((arr[1, 2]) != 6)
    return false;
  return true;
}());

static_assert([] consteval {
  Static2DArr<int> arr = {{10, 20, 30}};
  if (arr.rows() != 1u)
    return false;
  if (arr.cols() != 3u)
    return false;
  if ((arr[0, 0]) != 10)
    return false;
  if ((arr[0, 1]) != 20)
    return false;
  if ((arr[0, 2]) != 30)
    return false;
  return true;
}());

static_assert([] consteval {
  Static2DArr<int> arr = {{42}};
  if (arr.rows() != 1u)
    return false;
  if (arr.cols() != 1u)
    return false;
  if ((arr[0, 0]) != 42)
    return false;
  return true;
}());

static_assert([] consteval {
  Static2DArr<int> arr(3, 4);
  if (arr.rows() != 3u)
    return false;
  if (arr.cols() != 4u)
    return false;
  if (arr.size() != 12u)
    return false;
  return true;
}());

// fill()
static_assert([] consteval {
  Static2DArr<int> arr(2, 3);
  arr.fill(42);
  return arr[0, 0] == 42 && arr[1, 2] == 42;
}());

// inBounds()
static_assert([] consteval {
  Static2DArr<int> arr(3, 4);
  return arr.inBounds(0, 0) && arr.inBounds(2, 3) && !arr.inBounds(3, 0) && !arr.inBounds(0, 4);
}());