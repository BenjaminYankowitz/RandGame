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

// Iteration via begin()/end()
static_assert([] consteval {
  Static2DArr<int> arr = {{1, 2}, {3, 4}};
  int sum = 0;
  for (auto val : arr)
    sum += val;
  return sum == 10;
}());

// indexIter() yields correct (row, col) pairs
static_assert([] consteval {
  Static2DArr<int> arr(2, 3);
  arr.fill(0);
  for (auto [r, c] : arr.indexIter())
    arr[r, c] = (r * 10) + c;
  return arr[0, 0] == 0 && arr[0, 2] == 2 && arr[1, 0] == 10 && arr[1, 2] == 12;
}());

// Move construction
static_assert([] consteval {
  Static2DArr<int> arr = {{7, 8}, {9, 10}};
  Static2DArr<int> moved(std::move(arr));
  return moved.rows() == 2 && moved.cols() == 2 && moved[0, 0] == 7 && moved[1, 1] == 10;
}());

// int size_type
static_assert([] consteval {
  Static2DArr<int, int> arr(3, 4);
  arr.fill(5);
  return arr.rows() == 3 && arr.cols() == 4 && arr[2, 3] == 5;
}());

// Empty array
static_assert([] consteval {
  Static2DArr<int> arr;
  return arr.rows() == 0 && arr.cols() == 0 && arr.size() == 0 && arr.isNull();
}());
