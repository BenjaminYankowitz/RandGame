module GameState;

WorldFloor createFloor(int xDim, int yDim, Position upStair, Position downStair) {
  WorldFloor ret(xDim, yDim);
  DungeonMaker::openSimplex(ret.getTerrainTypeArr(), upStair, downStair, 32, 8, -0.2);
  return ret;
}

void addMonsters(GameState &state, FloorSpecifier floor, int count) noexcept {
  const WorldFloor &floorRef = state.getFloor(floor);
  for (int i = 0; i < count; ++i) {
    Position pos;
    int attempts = 0;
    do {
      pos = {Rnd::rnd(floorRef.cols()), Rnd::rnd(floorRef.rows())};
    } while (!floorRef.isOpenTile(pos) && (++attempts < 100));
    if (!floorRef.isOpenTile(pos))
      continue;
    Monster::createMonster(state, {pos, floor}, MonsterClass::Imp);
  }
}

void GameState::generateGame() noexcept {
  constexpr int DungeonWidth = 90;
  constexpr int DungeonHeight = 30;
  Position up = {-1, -1};
  for (int floor = 0; floor < 10; floor++) {
    Position down = up;
    if (floor == 9) {
      down = {-1, -1};
    }
    while (down == up) {
      down = {Rnd::rnd(DungeonWidth), Rnd::rnd(DungeonHeight)};
    }
    floorData_.push_back(createFloor(DungeonWidth, DungeonHeight, up, down));
    addMonsters(*this, FloorSpecifier(floor), floor);
    up = down;
  }
  auto tryPlaceMonster = [this](Position pos, MonsterClass mClass, bool isPlayer = false) {
    const auto cFloor = FloorSpecifier(0);
    while (pos != Position(0, DungeonHeight + 1)) {
      Location cLoc(pos, cFloor);
      if (!isOpenTile(cLoc)) {
        pos.x++;
        if (pos.x == DungeonWidth) {
          pos.x = 0;
          pos.y++;
        }
      } else {
        break;
      }
    }
    if (pos == Position(0, DungeonHeight + 1))
      return Monster::ID::null();
    return Monster::createMonster(*this, {pos, cFloor}, mClass, isPlayer);
  };
  player_ = tryPlaceMonster({0, 0}, MonsterClass::Human, true);
  tryPlaceMonster({0, 2}, MonsterClass::SeaSlug);
  tryPlaceMonster({4, 2}, MonsterClass::SeaSlug);
  tryPlaceMonster({2, 4}, MonsterClass::GreedyWeasel);
  tryPlaceMonster({4, 4}, MonsterClass::Bryozoan);
  WorldFloor &startingFloor = floorData_[0];
  startingFloor.getObjects({1, 0}).addObject({.type = ObjectType::KingsCoin, .mat = Material::Gold});
  startingFloor.getObjects({4, 2}).addObject({.type = ObjectType::KingsCoin, .mat = Material::Gold});
  startingFloor.getObjects({1, 0}).addObject({.type = ObjectType::Knife, .mat = Material::Iron});
  startingFloor.getObjects({1, 0}).addObject({.type = ObjectType::Knife, .mat = Material::Gold});
}

void GameState::broadcastEvent(Location eventLoc, auto &&func) noexcept {
  auto [pos, floor] = eventLoc;
  auto listeners = getFloor(floor).getEventListeners() |
                   std::views::transform([this](Monster::ID id) -> Monster & { return getMonster(id); }) |
                   std::views::filter([this, pos](const Monster &viewer) { return viewer.inLineOfSight(*this, pos); });
  std::ranges::for_each(listeners, func);
}

void GameState::broadcastItemPickup(const Monster &monster, const Object &object) noexcept {
  broadcastEvent(monster.getLoc(), [this, &monster, &object](Monster &viewer) {
    viewer.informItemPickup(*this, monster, object);
  });
}

void GameState::broadcastMonsterHitMonster(const Monster::HitReturn &hitInfo, const Monster &attacker, Monster &attacked) noexcept {
  auto inform = [this, &hitInfo, &attacker, &attacked](Monster &viewer) {
    viewer.informMonsterHitMonster(*this, hitInfo, attacker, attacked);
  };
  if (attacked.isAlive() && !attacked.caresEvent())
    inform(attacked);
  broadcastEvent(attacker.getLoc(), inform);
}

void GameState::broadcastMonsterHitWall(const Monster &attacker, Location loc) noexcept {
  auto inform = [this, &attacker, &loc](Monster &viewer) {
    viewer.informMonsterHitWall(*this, attacker, loc);
  };
  broadcastEvent(attacker.getLoc(), inform);
}

void GameState::broadcastMonsterAte(const Monster &eater, const Object &eaten) noexcept {
  broadcastEvent(eater.getLoc(), [this, &eater, &eaten](Monster &viewer) {
    viewer.informMonsterAte(*this, eater, eaten);
  });
}
