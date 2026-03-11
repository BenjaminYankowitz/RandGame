#include <gtest/gtest.h>
import GameTypes;

// fight() predicates
static_assert(MoveMode::fight().isFight());
static_assert(!MoveMode::fight().isMove());

// move() predicates
static_assert(MoveMode::move().isMove());
static_assert(!MoveMode::move().isFight());

// operator| : both flags set
static_assert((MoveMode::fight() | MoveMode::move()).isFight());
static_assert((MoveMode::fight() | MoveMode::move()).isMove());

// operator& : neither flag when fight & move (different bits)
static_assert(!(MoveMode::fight() & MoveMode::move()).isFight());
static_assert(!(MoveMode::fight() & MoveMode::move()).isMove());

// operator^ : fight ^ fight clears fight
static_assert(!(MoveMode::fight() ^ MoveMode::fight()).isFight());

// operator~ : ~fight clears fight
static_assert(!(~MoveMode::fight()).isFight());

// Compound assignment |=
static_assert([] {
  auto m = MoveMode::fight();
  m |= MoveMode::move();
  return m.isFight() && m.isMove();
}());

// Compound assignment &=
static_assert([] {
  auto m = MoveMode::fight();
  m &= MoveMode::move();
  return !m.isFight() && !m.isMove();
}());

// Compound assignment ^=
static_assert([] {
  auto m = MoveMode::fight() | MoveMode::move();
  m ^= MoveMode::fight();
  return !m.isFight() && m.isMove();
}());
