#include "TestHeader.h"
import GameTypes;
import Common;

// Fight predicates
static_assert(hasOverlap(MoveMode::Fight, MoveMode::Fight));
static_assert(!hasOverlap(MoveMode::Fight, MoveMode::Move));

// Move predicates
static_assert(hasOverlap(MoveMode::Move, MoveMode::Move));
static_assert(!hasOverlap(MoveMode::Move, MoveMode::Fight));

// operator| : both flags set
static_assert(hasOverlap(MoveMode::Fight | MoveMode::Move, MoveMode::Fight));
static_assert(hasOverlap(MoveMode::Fight | MoveMode::Move, MoveMode::Move));

// operator& : neither flag when fight & move (different bits)
static_assert(!hasOverlap(MoveMode::Fight & MoveMode::Move, MoveMode::Fight));
static_assert(!hasOverlap(MoveMode::Fight & MoveMode::Move, MoveMode::Move));

// operator^ : fight ^ fight clears fight
static_assert(!hasOverlap(MoveMode::Fight ^ MoveMode::Fight, MoveMode::Fight)); //NOLINT(misc-redundant-expression)

// operator~ : ~fight clears fight
static_assert(!hasOverlap(~MoveMode::Fight, MoveMode::Fight));

// Compound assignment |=
static_assert([] {
  auto m = MoveMode::Fight;
  m |= MoveMode::Move;
  return hasOverlap(m, MoveMode::Fight) && hasOverlap(m, MoveMode::Move);
}());

// Compound assignment &=
static_assert([] {
  auto m = MoveMode::Fight;
  m &= MoveMode::Move;
  return !hasOverlap(m, MoveMode::Fight) && !hasOverlap(m, MoveMode::Move);
}());

// Compound assignment ^=
static_assert([] {
  auto m = MoveMode::Fight | MoveMode::Move;
  m ^= MoveMode::Fight;
  return !hasOverlap(m, MoveMode::Fight) && hasOverlap(m, MoveMode::Move);
}());
