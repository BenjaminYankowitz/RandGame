#include "TestHeader.h"
import GameTypes;

// ============================================================
// TimePeriod
// ============================================================

// Construction
static_assert(TimePeriod(10).impl == 10);
static_assert(TimePeriod(0).impl == 0);

// operator+
static_assert((TimePeriod(3) + TimePeriod(7)).impl == 10);

// operator+=
static_assert([] {
  TimePeriod t(5);
  t += TimePeriod(3);
  return t.impl == 8;
}());

// operator*
static_assert((TimePeriod(4) * 3).impl == 12);

// operator*=
static_assert([] {
  TimePeriod t(5);
  t *= 4;
  return t.impl == 20;
}());

// operator/
static_assert((TimePeriod(20) / 4).impl == 5);

// operator/=
static_assert([] {
  TimePeriod t(30);
  t /= 6;
  return t.impl == 5;
}());

// operator<=>
static_assert(TimePeriod(5) < TimePeriod(10));
static_assert(TimePeriod(10) > TimePeriod(5));
static_assert(!(TimePeriod(5) > TimePeriod(5)));

// operator--
static_assert([] {
  TimePeriod t(3);
  --t;
  return t.impl == 2;
}());

// future()
static_assert(TimePeriod(1).future());
static_assert(!TimePeriod(0).future());
static_assert([] {
  TimePeriod t(1);
  --t;
  return !t.future();
}());

// Chained expression
static_assert(((TimePeriod(10) + TimePeriod(5)) * 2).impl == 30);

// ============================================================
// GameTime
// ============================================================

// Default
static_assert(GameTime().impl == 0);

// + with TimePeriod
static_assert([] {
  GameTime g;
  GameTime g2 = g + TimePeriod(5);
  return g2.impl == 5;
}());

// += with TimePeriod
static_assert([] {
  GameTime g;
  g += TimePeriod(10);
  return g.impl == 10;
}());

// ++
static_assert([] {
  GameTime g;
  ++g;
  ++g;
  return g.impl == 2;
}());

// <=>
static_assert([] {
  GameTime a;
  a += TimePeriod(5);
  GameTime b;
  b += TimePeriod(10);
  return a < b;
}());

// ==
static_assert(GameTime() == GameTime());
static_assert([] {
  GameTime a;
  a += TimePeriod(3);
  GameTime b;
  b += TimePeriod(3);
  return a == b;
}());
