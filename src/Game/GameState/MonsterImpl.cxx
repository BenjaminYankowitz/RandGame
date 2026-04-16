module GameState;

Dir monsterPath(const GameState &state, const Monster &start, Location end) {
  auto [cPos, floor] = start.getLoc();
  if (end.mapPos != floor) {
    TerrainType targetStair = (end.mapPos.floor < floor.floor) ? TerrainType::UpStair : TerrainType::DownStair;
    if (state.getFloor(floor).getTerrainType(cPos) == targetStair) {
      return Dir::getInvalid();
    }
    Position stairPos = state.getFloor(floor).findTerrain(targetStair);
    if (stairPos == Position{-1, -1}) {
      return Dir{0, 0};
    }
    if (Position::chessboard(cPos, stairPos) <= 1) {
      return stairPos - cPos;
    }
    return FindPath::findPath(WorldFloorWrapper<&WorldFloor::isOpenTile>(state.getFloor(floor)), cPos, stairPos, 30);
  }
  if (Position::chessboard(cPos, end.pos) <= 1) {
    return end.pos - cPos;
  }
  return FindPath::findPath(WorldFloorWrapper<&WorldFloor::isOpenTile>(state.getFloor(floor)), cPos, end.pos, 3);
}

bool Monster::isOpenMove(GameState &state, Dir d) const noexcept {
  return state.getFloor(loc_.mapPos).isOpenTile(loc_.pos + d);
}

bool Monster::inLineOfSight(const GameState &state, Position pos) const noexcept {
  if (!state.getFloor(loc_.mapPos).inBounds(pos)) {
    return false;
  }
  return LineOfSight::inLineOfSight(WorldFloorWrapper<&WorldFloor::seeThrough>(state.getFloor(loc_.mapPos)), loc_.pos, pos);
}

TimePeriod Monster::goToTarget(GameState &state, NoTarget /*unused*/) noexcept {
  if (body_.health < body_.maxHealth) {
    return rest();
  }
  std::array IntToDir = Dir::boxDirsArr();
  auto *const endIter = std::ranges::remove_if(IntToDir, [this, &state](Dir d) {
                          return !isOpenMove(state, d);
                        }).begin();
  const std::size_t validDirs = std::distance(IntToDir.begin(), endIter);
  if (validDirs == 0) {
    return reThink(ReThinkReason::CanNotMove);
  }
  TimePeriod ret = generalMove(state, IntToDir[Rnd::rnd(validDirs)], MoveMode::Move);
  if (ret.future()) {
    return ret;
  }
  state.printDebug("Failed to move to a place which should be moveable to");
  return reThink(ReThinkReason::CanNotMove);
}

TimePeriod Monster::goToTarget(GameState &state, HangTarget target) noexcept {
  return state.tryGetMonster(target.target).doIf([&](const Monster &target) { 
    if(getLoc()==target.getLoc()){
      if(brain_.snuggleDesire<=0){
        return reThink(ReThinkReason::DoneWithSnuggles);
      }
      brain_.snuggleDesire-=4;
      return body_.speed;
    }
    return pathTo(state, target.getLoc(), MoveMode::GetWith); }, [&]() { return reThink(ReThinkReason::TargetDead); });
}

TimePeriod Monster::goToTarget(GameState &state, ID target) noexcept {
  return state.tryGetMonster(target).doIf([&](const Monster &target) { return pathTo(state, target.getLoc(), MoveMode::Fight); }, [&]() { return reThink(ReThinkReason::TargetDead); });
}

TimePeriod Monster::goToTarget(GameState &state, Location target) noexcept {
  return pathTo(state, target, MoveMode::Move);
}

TimePeriod Monster::goToTarget(GameState &state, Monster::EatTarget target) noexcept {
  if (target.loc == getLoc()) {
    for (auto [i, obj] : Views::enumerate(state.getObjects(getLoc()))) {
      if (wantsToEat(obj)) {
        return eatItem(state, i, true);
      }
    }
    return reThink(ReThinkReason::FoodGone);
  }
  return goToTarget(state, target.loc);
}

TimePeriod Monster::pathTo(GameState &state, Location target, MoveMode onceReached) noexcept {
  if (target == getLoc()) {
    return reThink(ReThinkReason::ReachedDestination);
  }
  auto movePlan = monsterPath(state, *this, target);
  if (movePlan == Dir{0, 0}) {
    return reThink(ReThinkReason::CanNotPathToTarget);
  }
  Location moveTo = loc_;
  if (movePlan.invalid()) {
    moveTo = (state.getTerrainType(loc_) == TerrainType::UpStair) ? moveTo.up() : moveTo.down();
  } else {
    moveTo.pos += movePlan;
  }
  if (target != moveTo) {
    const TimePeriod tTaken = generalMove(state, moveTo, MoveMode::Move);
    if (tTaken.future()) {
      return tTaken;
    }
    return reThink(ReThinkReason::CanNotPathToTarget);
  }
  if (onceReached == MoveMode::None) {
    return reThink(ReThinkReason::ReachedDestination);
  }
  const TimePeriod tTaken = generalMove(state, moveTo, onceReached);
  if (tTaken.future()) {
    return tTaken;
  }
  if (hasOverlap(onceReached, MoveMode::Move)) {
    return reThink(ReThinkReason::CanNotPathToTarget);
  }
  if (hasOverlap(onceReached, MoveMode::GetWith)) {
    return reThink(ReThinkReason::FailedGetWith);
  }
  state.printDebug("Attack attempted but no time taken. This should not be possible.");
  state.printDebug("If it is possible logic should probably be reworked.");
  return reThink(ReThinkReason::FailedAttack);
}

void Monster::findTask(GameState &state) noexcept {
  auto [pos, mapPos] = getLoc();
  const auto &cFloor = state.getFloor(mapPos);
  for (auto cPos : LineOfSight::allInLineOfSight(WorldFloorWrapper<&WorldFloor::seeThrough>(cFloor), pos)) {
    auto [objs, monst, tile] = cFloor.getTile(cPos);
    ID cMonst = monst;
    while (!cMonst.isNull()) {
      if (cMonst == getId()) {
        cMonst = next_;
        continue;
      }
      auto &monstRef = state.getMonster(cMonst);
      if (wantsToKill(monstRef)) {
        brain_.target = cMonst;
        return;
      }
      if (brain_.snuggleDesire > 30 && body_.mClass == MonsterClass::SeaSlug && monstRef.body_.mClass == MonsterClass::SeaSlug) {
        brain_.target = HangTarget{cMonst};
        return;
      }
      cMonst = monstRef.next_;
    }
    if (std::ranges::any_of(objs, [this](const Object &obj) { return wantsToEat(obj); })) {
      brain_.target = Monster::EatTarget{{cPos, mapPos}};
      return;
    }
  }
}

TimePeriod Monster::takeItem(GameState &state, ObjectContainer &container, std::size_t index) noexcept {
  static constexpr std::size_t PickUpItemSpeedFraction = 10;
  state.broadcastItemPickup(*this, container[index]);
  inventory_.addObject(container.remove(index));
  return getSpeed() / PickUpItemSpeedFraction;
}

TimePeriod Monster::generalMove(GameState &state, Location nLoc, MoveMode mode) noexcept {
  if (!state.isOpenTerrain(nLoc)) {
    return TimePeriod(0);
  }
  auto &destMonster = state.getMonster(nLoc);
  if (hasOverlap(mode, destMonster.isNull() ? MoveMode::Move : MoveMode::GetWith)) {
    if (!next_.isNull()) {
      state.getMonster(next_).prev_ = prev_;
    }
    ID &currentSpot = prev_.isNull() ? state.getMonster(getLoc()) : state.getMonster(prev_).next_;
    prev_.clear();
    currentSpot = next_;
    next_ = destMonster;
    destMonster = id_;
    if (!next_.isNull()) {
      state.getMonster(next_).prev_ = id_;
    }
    if (getLoc().mapPos != nLoc.mapPos && caresEvent()) {
      state.getFloor(getLoc().mapPos).removeEventListener(getId());
      state.getFloor(nLoc.mapPos).addEventListener(getId());
    }
    loc_ = nLoc;
    return getSpeed();
  }
  if (!destMonster.isNull() && hasOverlap(mode, MoveMode::Fight)) {
    return hitMonster(state, state.getMonster(destMonster));
  }
  return TimePeriod{0};
}

TimePeriod Monster::generalMove(GameState &state, Position nPos, MoveMode mode) noexcept {
  return generalMove(state, Location(nPos, loc_.mapPos), mode);
}

TimePeriod Monster::generalMove(GameState &state, Dir d, MoveMode mode) noexcept {
  return generalMove(state, Location(loc_.pos + d, loc_.mapPos), mode);
}

TimePeriod Monster::goUpStair(GameState &state, MoveMode m) noexcept {
  if (state.getTerrainType(loc_) == TerrainType::UpStair) {
    return generalMove(state, loc_.up(), m);
  }
  return TimePeriod(0);
}

TimePeriod Monster::goDownStair(GameState &state, MoveMode m) noexcept {
  if (state.getTerrainType(loc_) == TerrainType::DownStair) {
    return generalMove(state, loc_.down(), m);
  }
  return TimePeriod(0);
}

void Monster::informItemPickup(GameState &state, const Monster &grabber, const Object &grabbed) noexcept {
  if (isPlayer()) {
    state.printItemPickup(grabber, grabbed);
    return;
  }
  if (brain_.config.hatesItemPickup()) {
    brain_.target = grabber.getId();
  }
}

void Monster::informMonsterHitMonster(GameState &state, const HitReturn &hitinfo, const Monster &attacker, const Monster &attacked) noexcept {
  if (isPlayer()) {
    state.printMonsterHitMonster(hitinfo, attacker, attacked);
    return;
  }
  if (attacked == *this) {
    brain_.target = attacker.getId();
  }
}

void Monster::informMonsterHitWall(GameState &state, const Monster &attacker, Location loc) noexcept { // NOLINT(readability-make-member-function-const)
  if (isPlayer()) {
    state.printMonsterHitWall(attacker, loc);
    return;
  }
}

void Monster::informMonsterAte(GameState &state, const Monster &eater, const Object &eaten) noexcept { // NOLINT(readability-make-member-function-const)
  if (isPlayer()) {
    state.printMonsterAte(eater, eaten);
    return;
  }
}

TimePeriod Monster::runAI(GameState &state) noexcept {
  if (!isAlive()) {
    return TimePeriod(0);
  }
  if (body_.mClass == MonsterClass::SeaSlug)
    brain_.snuggleDesire++;
  if (std::holds_alternative<NoTarget>(brain_.target)) {
    findTask(state);
  }
  TimePeriod timeTaken = brain_.target.visit([&](auto target) { return goToTarget(state, target); });
  if (!isAlive()) {
    return TimePeriod(0);
  }
  if (timeTaken.future()) {
    return timeTaken;
  }
  state.printDebug("time taken to runAI is 0");
  return reThink(ReThinkReason::Unknown);
}

TimePeriod Monster::dropItem(GameState &state, std::size_t i) noexcept {
  static constexpr std::size_t DropItemSpeedFraction = 10;
  auto obj = removeFromInvent(i);
  state.getObjects(getLoc()).addObject(std::move(obj));
  return getSpeed() / DropItemSpeedFraction;
}

[[nodiscard]] constexpr std::pair<bool, bool> getNMirror(const WorldFloor &floor, Position lastPos, Position cSpot) noexcept {
  const Dir moveDir = lastPos - cSpot;
  bool mX = moveDir.dx != 0;
  bool mY = moveDir.dy != 0;
  if (!mX || !mY)
    return std::make_pair(mX, mY);
  mX = !floor.isOpenTerrain(lastPos + Dir{moveDir.dx, 0});
  mY = !floor.isOpenTerrain(lastPos + Dir{0, moveDir.dy});
  if (mX || mY)
    return std::make_pair(mX, mY);
  int choice = 2;
  if not consteval {
    choice = Rnd::rnd(3);
  }
  return std::make_pair(choice != 0, choice != 1);
}

constexpr Location runPath(GameState &state, Location start, Dir dir, int maxDist, auto &monsterHit, bool bounceOnWall) noexcept { // TODO: ben - add animation for this.
  if (dir.noMove()) {
    return start;
  }
  const auto [startPos, floorId] = start;
  const auto &floor = state.getFloor(floorId);
  Position lastPos = startPos;
  Position basePos = startPos;
  auto iter = ++PathIterable(dir).begin();
  bool mirrorX = false;
  bool mirrorY = false;
  while (true) {
    const Dir cDir = (*iter).mirror(mirrorX, mirrorY);
    const int dist = Dir::chessboard(cDir);
    if (dist > maxDist)
      break;
    const Position cSpot = basePos + cDir;
    if (!floor.isOpenTerrain(cSpot)) {
      if (!bounceOnWall)
        break;
      const auto [mX, mY] = getNMirror(floor, lastPos, cSpot);
      mirrorX ^= mX;
      mirrorY ^= mY;
      const Dir nDir = (*iter).mirror(mirrorX, mirrorY);
      basePos = lastPos - nDir;
      continue;
    }
    if (const auto targetID = floor.getMonster(cSpot)) {
      const int nRemDist = monsterHit(state.getMonster(targetID), maxDist - dist);
      maxDist = dist + nRemDist;
    }
    lastPos = cSpot;
    ++iter;
  }
  return {lastPos, floorId};
}

constexpr void sendItemFlying(GameState &state, Monster &source, std::unique_ptr<Object> obj, Location start, Dir dir) {
  const int maxDist = source.getMaxThrowingDistance();
  auto onHit = [&state, &source, &obj = *obj, maxDist](Monster &target, int distLeft) {
    Health damage = obj.type() == ObjectType::Knife ? 5 : 2;
    if (distLeft < maxDist / 2) // TODO: ben - replace this with to-hit penalty once to-hit is added
      damage /= 2;
    monsterHitMonster(state, source, target, {damage});
    return 0;
  };
  state.getObjects(runPath(state, start, dir, maxDist, onHit, false)).addObject(std::move(obj));
}

constexpr void createBeam(GameState &state, Monster &source, Location start, Dir dir, int maxDist, Dice::Group damage) {
  auto onHit = [&](Monster &target, int distLeft) {
    monsterHitMonster(state, source, target, {damage()});
    return distLeft - Rnd::rnd(5);
  };
  runPath(state, start, dir, maxDist, onHit, true);
}

TimePeriod Monster::throwItem(GameState &state, std::size_t i, Dir dir, int count) noexcept {
  sendItemFlying(state, *this, removeFromInvent(i, count), getLoc(), dir);
  return getSpeed();
}

TimePeriod Monster::eatItem(GameState &state, std::size_t i, bool fromFloor) noexcept {
  ObjectContainer &container = fromFloor ? state.getObjects(loc_) : inventory_;
  Object &toEat = container[i];
  state.broadcastMonsterAte(*this, toEat);
  if (--toEat.count() == 0) {
    (void)container.remove(i);
  }
  return getSpeed();
}

TimePeriod Monster::rest() noexcept {
  using namespace Dice::Literals;
  static constexpr auto HealDice = "1d2-1"_dice;
  body_.health = std::min<Health>(body_.health + HealDice(), body_.maxHealth);
  return getSpeed();
}

TimePeriod Monster::hitMonster(GameState &state, Monster &target) noexcept {
  monsterHitMonster(state, *this, target, {body_.damage()});
  return getSpeed();
}

TimePeriod Monster::castBeam(GameState &state, Dir dir) noexcept {
  createBeam(state, *this, loc_, dir, Rnd::uniform_int(10, 20), body_.damage);
  return getSpeed();
}

Monster::ID Monster::createMonster(GameState &game, Location loc, MonsterClass mClass, bool isPlayer) noexcept {
  auto &monsterDest = game.getMonster(loc);
  if (!monsterDest.isNull()) {
    return ID::null();
  }
  const auto &mInfo = MonsterClassInfoArr[mClass];
  ID id = game.nextMonsterId();
  MonsterBodyInit body{.speed = mInfo.speed, .maxHealth = mInfo.baseHealth, .damage = mInfo.damage, .mClass = mClass};
  MonsterBrainConfig brain = isPlayer ? PlayerBrain : mInfo.brain;
  Monster &mstr = game.insertMonster(std::make_unique<Monster>(body, loc, id, brain));
  if (mstr.caresEvent()) {
    game.getFloor(loc.mapPos).addEventListener(id);
  }
  monsterDest = id;
  if (!isPlayer && mstr.getSpeed().future()) {
    game.addMonsterEvent(TimePeriod(0), id);
  }
  return id;
}

void Monster::kill(GameState &state, ObjectContainer &itemsTo) noexcept {
  if (caresEvent()) {
    state.getFloor(getLoc().mapPos).removeEventListener(getId());
  }
  setDead();
  if (prev_.isNull()) {
    state.getMonster(loc_) = next_;
  } else {
    state.getMonster(prev_).next_ = next_;
  }
  if (!next_.isNull()) {
    state.getMonster(next_).prev_ = prev_;
  }
  next_.clear();
  prev_.clear();
  itemsTo.takeAllFrom(inventory_);
  itemsTo.addObject(ObjectBluePrint{corpseOf(body_.mClass)});
}

void monsterHitMonster(GameState &state, Monster &attacker, Monster &attacked, Monster::AttackInfo info) noexcept {
  auto hitReturn = attacked.hitBy(info);
  attacker.gainExp(hitReturn.exp);
  state.broadcastMonsterHitMonster(hitReturn, attacker, attacked);
  if (!hitReturn.killed)
    return;
  attacked.kill(state, state.getObjects(attacked.getLoc()));
  if (attacked.isPlayer())
    return;
  (void)state.removeMonster(attacked.getId());
}

Monster::HitReturn Monster::hitBy(AttackInfo info) noexcept {
  const bool killed = removeHealth(info.damage);
  return {info.damage, killed ? exp_ / 2 : 0, killed};
}