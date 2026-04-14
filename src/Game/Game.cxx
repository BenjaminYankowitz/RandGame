export module Game;
import Common;
import DungeonMaker;
export import GameTypes;
import MonsterClassConfig;
import SerializationLib;

export class GameState;
export class Monster {
public:
  struct MonsterBodyInit {
    TimePeriod speed;
    MustInit<Health> maxHealth;
    Health currentHealth = maxHealth;
    Dice::Group damage;
    MustInit<MonsterClass> mClass;
    bool alive = true;
    int maxThrowingDistance = 10;
    bool immortal = false;
  };
  struct MonsterBody {
    explicit MonsterBody(MonsterBodyInit body) noexcept : speed(body.speed), maxHealth(body.maxHealth), health(body.currentHealth), maxThrowingDistance(body.maxThrowingDistance), damage(body.damage), mClass(body.mClass), alive(body.alive), immortal(body.immortal) {}
    TimePeriod speed;
    Health maxHealth;
    Health health;
    int maxThrowingDistance;
    Dice::Group damage;
    MonsterClass mClass;
    bool alive : 1;
    bool immortal : 1; 
  };
  using ID = MonsterID;
  struct HitReturn {
    MustInit<Health> damageDone;
    int exp = 0;
    bool killed = false;
  };

  struct NoTarget {};
  struct EatTarget {
    Location loc;
  };
  struct HangTarget {
    ID target;
  };
  struct MonsterBrain {
    std::variant<NoTarget, ID, Location, EatTarget, HangTarget> target = NoTarget{};
    int snuggleDesire = 0;
    MonsterBrainConfig config;
  };

  struct AttackInfo {
    MustInit<Health> damage;
  };

  static ID createMonster(GameState &game, Location loc, MonsterClass mClass, bool isPlayer = false) noexcept;
  [[nodiscard]] constexpr TimePeriod generalMove(GameState &state, Location nLoc, MoveMode m) noexcept;
  [[nodiscard]] constexpr TimePeriod generalMove(GameState &state, Position nPos, MoveMode m) noexcept;
  [[nodiscard]] constexpr TimePeriod generalMove(GameState &state, Dir d, MoveMode m) noexcept;
  [[nodiscard]] constexpr TimePeriod goUpStair(GameState &state, MoveMode m) noexcept;
  [[nodiscard]] constexpr TimePeriod goDownStair(GameState &state, MoveMode m) noexcept;
  [[nodiscard]] TimePeriod runAI(GameState &state) noexcept;
  [[nodiscard]] constexpr MonsterClass getClass() const noexcept { return body_.mClass; };
  [[nodiscard]] constexpr TimePeriod getSpeed() const noexcept { return body_.speed; };
  [[nodiscard]] constexpr Health getHealth() const noexcept { return body_.health; }
  [[nodiscard]] constexpr Health getMaxHealth() const noexcept { return body_.maxHealth; }
  [[nodiscard]] constexpr int getMaxThrowingDistance() const noexcept { return body_.maxThrowingDistance; }
  [[nodiscard]] constexpr bool removeHealth(Health amount) noexcept {
    if (0 >= (body_.health -= amount)) {
      if (body_.immortal) {
        body_.health = body_.maxHealth;
        return false;
      }
      return true;
    }
    return false;
  }
  [[nodiscard]] constexpr Location getLoc() const noexcept { return loc_; }
  [[nodiscard]] constexpr ID getId() const noexcept { return id_; }
  [[nodiscard]] constexpr ID getNext() const noexcept { return next_; }
  [[nodiscard]] constexpr const ObjectContainer &viewInventory() const noexcept { return inventory_; }
  [[nodiscard]] constexpr bool isPlayer() const noexcept { return brain_.config.isPlayer(); }
  [[nodiscard]] constexpr bool isAlive() const noexcept { return body_.alive; }
  [[nodiscard]] constexpr bool canEat(const Object &obj) const noexcept { return getClass() != MonsterClass::Bryozoan && obj.mat() == Material::Flesh; }
  [[nodiscard]] constexpr bool wantsToEat(const Object &obj) const noexcept { return canEat(obj); }
  [[nodiscard]] constexpr bool wantsToKill(const Monster &monst) const noexcept { return MonsterClassHunts(getClass(), monst.getClass()); }
  [[nodiscard]] constexpr bool inLineOfSight(const GameState &state, Position pos) const noexcept;
  [[nodiscard]] constexpr bool caresEvent() const noexcept { return brain_.config.caresEvent(); }
  [[nodiscard]] constexpr bool isOpenMove(GameState &state, Dir d) const noexcept;
  constexpr void informItemPickup(GameState &state, const Monster &grabber, const Object &grabbed) noexcept;
  constexpr void informMonsterHitMonster(GameState &state, const HitReturn &hitinfo, const Monster &attacker, const Monster &attacked) noexcept;
  constexpr void informMonsterHitWall(GameState &state, const Monster &attacker, Location loc) noexcept;
  constexpr void informMonsterAte(GameState &state, const Monster &eater, const Object &eaten) noexcept;
  friend void monsterHitMonster(GameState &state, Monster &attacker, Monster &attacked, AttackInfo info) noexcept;
  [[nodiscard]] HitReturn hitBy(AttackInfo info) noexcept;
  constexpr void gainExp(int n) noexcept { exp_ += n; }
  constexpr void kill(GameState &state, ObjectContainer &itemsTo) noexcept;
  [[nodiscard]] constexpr std::unique_ptr<Object> removeFromInvent(std::size_t i, int count = std::numeric_limits<int>::max()) noexcept {
    if (count >= inventory_[i].count()) {
      return inventory_.remove(i);
    }
    return inventory_[i].split(count);
  }
  [[nodiscard]] TimePeriod goToTarget(GameState &state, NoTarget /*unused*/) noexcept;
  [[nodiscard]] TimePeriod goToTarget(GameState &state, HangTarget target) noexcept;
  [[nodiscard]] TimePeriod goToTarget(GameState &state, ID target) noexcept;
  [[nodiscard]] TimePeriod goToTarget(GameState &state, Location target) noexcept;
  [[nodiscard]] TimePeriod goToTarget(GameState &state, EatTarget target) noexcept;
  [[nodiscard]] TimePeriod pathTo(GameState &state, Location target, MoveMode onceReached) noexcept;
  void findTask(GameState &state) noexcept;
  enum class ReThinkReason : std::uint8_t {
    CanNotMove,
    TargetDead,
    CanNotPathToTarget,
    FailedAttack,
    FailedGetWith,
    ReachedDestination,
    Unknown,
    FoodGone,
    DoneWithSnuggles,
  };
  [[nodiscard]] TimePeriod reThink(ReThinkReason reason) noexcept {
    switch (reason) {
      using enum ReThinkReason;
    case CanNotMove:
      return TimePeriod(10);
    case CanNotPathToTarget:
    case FailedAttack:
    case Unknown:
    case TargetDead:
    case ReachedDestination:
    case FailedGetWith:
    case FoodGone:
    case DoneWithSnuggles:
      brain_.target = NoTarget{};
      return TimePeriod(1);
    }
  }
  [[nodiscard]] constexpr TimePeriod takeItem(GameState &state, ObjectContainer &container, std::size_t index) noexcept;
  [[nodiscard]] constexpr TimePeriod dropItem(GameState &state, std::size_t i) noexcept;
  [[nodiscard]] constexpr TimePeriod throwItem(GameState &state, std::size_t i, Dir dir, int count) noexcept;
  [[nodiscard]] constexpr TimePeriod eatItem(GameState &state, std::size_t i, bool fromFloor) noexcept;
  [[nodiscard]] TimePeriod rest() noexcept;
  [[nodiscard]] TimePeriod hitMonster(GameState &state, Monster &target) noexcept;
  [[nodiscard]] constexpr TimePeriod castBeam(GameState &state, Dir dir, Health damage) noexcept;
  [[nodiscard]] TimePeriod castBeam(GameState &state, Dir dir) noexcept;
  Monster(MonsterBodyInit body, Location loc, ID id, MonsterBrainConfig brain) noexcept : Monster(MonsterBody(body), loc, id, MonsterBrain{.config=brain}) {}
  Monster(MonsterBody body, Location loc, ID id, MonsterBrain brain) noexcept : body_(body), brain_(brain), loc_(loc), id_(id) {};
  std::size_t serializeTo(std::ostream &out) const noexcept;
  static Monster deserializeFrom(std::istream &in, std::size_t &numRead);

  constexpr void setImmortal(bool immortal = true) noexcept { body_.immortal = immortal; }

private:
  constexpr void setDead(bool dead = true) noexcept { body_.alive = !dead; }
  ObjectContainer inventory_;
  MonsterBody body_;
  MonsterBrain brain_;
  Location loc_;
  int exp_ = 2;
  ID id_;
  ID next_;
  ID prev_;
};

[[nodiscard]] constexpr bool operator==(const Monster &lhs, const Monster &rhs) noexcept {
  return &lhs == &rhs;
}

export std::size_t toStream(std::ostream &out, const Monster &input);
export Monster fromStream(std::istream &in, std::size_t &numRead, SerializationLib::Tag<Monster> /**/);

class WorldTile {
public:
  constexpr WorldTile(ObjectContainer &objectsI, Monster::ID &monsterI, TerrainType &terrainTypeI) noexcept : objects(objectsI), monster(monsterI), terrainType(terrainTypeI) {}
  ObjectContainer &objects;
  Monster::ID &monster;
  TerrainType &terrainType;
};

export class ConstWorldTile {
public:
  constexpr ConstWorldTile(const ObjectContainer &objectsI, const Monster::ID &monsterI, const TerrainType &terrainTypeI) noexcept : objects(objectsI), monster(monsterI), terrainType(terrainTypeI) {}
  constexpr ConstWorldTile(const WorldTile &tile) noexcept : objects(tile.objects), monster(tile.monster), terrainType(tile.terrainType) {} // NOLINT(google-explicit-constructor)
  const ObjectContainer &objects;
  const Monster::ID &monster;
  const TerrainType &terrainType;
};
export class WorldFloor {
public:
#define ACCESSORS_WORLD_FLOOR(name)                                                                                                      \
  [[nodiscard]] constexpr auto begin##name(this auto &&self) noexcept { return self.name##Arr_.begin(); }                                \
  [[nodiscard]] constexpr auto end##name(this auto &&self) noexcept { return self.name##Arr_.end(); }                                    \
  [[nodiscard]] constexpr auto iterable##name(this auto &&self) noexcept { return Iterable(self.begin##name(), self.end##name()); }      \
  [[nodiscard]] constexpr auto &get##name##Arr(this auto &&self) noexcept { return std::forward_like<decltype(self)>(self.name##Arr_); } \
  [[nodiscard]] constexpr auto &get##name(this auto &&self, Position pos) noexcept { return self.get##name##Arr()[pos]; }
  ACCESSORS_WORLD_FLOOR(Objects)
  ACCESSORS_WORLD_FLOOR(Monster)
  ACCESSORS_WORLD_FLOOR(TerrainType)
#undef ACCESSORS_WORLD_FLOOR

  [[nodiscard]] constexpr auto getTile(Position pos) noexcept { return WorldTile(getObjects(pos), getMonster(pos), getTerrainType(pos)); }
  [[nodiscard]] constexpr auto getTile(Position pos) const noexcept { return ConstWorldTile(getObjects(pos), getMonster(pos), getTerrainType(pos)); }
  [[nodiscard]] constexpr auto seeThrough(Position pos) const noexcept {
    return inBounds(pos) && getTerrainType(pos) != TerrainType::Wall;
  }
  [[nodiscard]] constexpr auto isOpenTerrain(Position pos) const noexcept {
    return inBounds(pos) && getTerrainType(pos) != TerrainType::Wall;
  }
  [[nodiscard]] constexpr auto isOpenTile(Position pos) const noexcept {
    return isOpenTerrain(pos) && getMonster(pos).isNull();
  }
  [[nodiscard]] constexpr Position findTerrain(TerrainType type) const noexcept {
    for (Position p : getTerrainTypeArr().indexIter()) {
      if (getTerrainType(p) == type)
        return p;
    }
    return Position{-1, -1};
  }
  constexpr WorldFloor(int x, int y) noexcept : ObjectsArr_(x, y), MonsterArr_(x, y), TerrainTypeArr_(x, y) {}
  constexpr WorldFloor(WorldFloor &&) noexcept = default;
  constexpr WorldFloor &operator=(WorldFloor &&) noexcept = default;
  [[nodiscard]] constexpr int rows() const noexcept { return ObjectsArr_.rows(); }
  [[nodiscard]] constexpr int cols() const noexcept { return ObjectsArr_.cols(); }
  [[nodiscard]] constexpr bool inBounds(Position pos) const noexcept {
    return ObjectsArr_.inBounds(pos);
  }
  constexpr void addEventListener(Monster::ID id) {
    EventListenerArr_.push_back(id);
  }
  constexpr void removeEventListener(Monster::ID id) {
    auto val = std::ranges::find(EventListenerArr_, id);
    if constexpr (InDebug) {
      if (val == EventListenerArr_.end()) {
        Logging::log << "removeEventListener: ID not found in listener list\n";
        return;
      }
    }
    *val = EventListenerArr_.back();
    EventListenerArr_.pop_back();
  }
  [[nodiscard]] constexpr auto getEventListeners(GameState &state) noexcept {
    return EventListenerArr_ | std::views::transform(IDToMonster{&state});
  }
  [[nodiscard]] constexpr auto &getEventListenersArr() noexcept {
    return EventListenerArr_;
  }
  [[nodiscard]] constexpr const auto &getEventListenersArr() const noexcept {
    return EventListenerArr_;
  }

private:
  struct IDToMonster {
    GameState *state;
    [[nodiscard]] Monster &operator()(Monster::ID monst) const noexcept;
  };
  StaticPositionArr<ObjectContainer> ObjectsArr_;
  StaticPositionArr<Monster::ID> MonsterArr_;
  StaticPositionArr<TerrainType> TerrainTypeArr_;
  std::vector<Monster::ID> EventListenerArr_;
};
export using WorldFloorFunc = bool (WorldFloor::*)(Position) const noexcept;
export template <WorldFloorFunc F>
struct WorldFloorWrapper {
  const WorldFloor &floor;
  [[nodiscard]] constexpr int extent(int n) const noexcept {
    switch (n) {
    case 0:
      return floor.rows();
    case 1:
      return floor.cols();
    default:
      std::unreachable();
    }
  }
  [[nodiscard]] constexpr bool operator[](int row, int col) const noexcept {
    return operator[](Position{col, row});
  }
  [[nodiscard]] constexpr bool operator[](Position p) const noexcept {
    return std::invoke(F, floor, p);
  }
};

export std::size_t toStream(std::ostream &out, const WorldFloor &input);
export WorldFloor fromStream(std::istream &in, std::size_t &numRead, SerializationLib::Tag<WorldFloor> /**/);

WorldFloor createFloor(int xDim, int yDim, Position upStair, Position downStair) {
  WorldFloor ret(xDim, yDim);
  DungeonMaker::openSimplex(ret.getTerrainTypeArr(), upStair, downStair, 32, 8, -0.2);
  return ret;
}

export class EventViewer {
public:
  virtual void itemPickup(const Monster &grabber, const Object &grabbed) noexcept = 0;
  virtual void monsterHitMonster(const Monster::HitReturn &hitinfo, const Monster &attacker, const Monster &attacked) noexcept = 0;
  virtual void monsterHitWall(const Monster &attacker, Location loc) noexcept = 0;
  virtual void monsterAte(const Monster &eater, const Object &eaten) noexcept = 0;
  virtual void debug(std::string_view message) noexcept = 0;
  virtual ~EventViewer() = default;
};

export class GameState {
public:
  GameState() noexcept = default;
  void generateGame() noexcept;
  void playerDied() {
    monsterEvents_.emplace(currentTime_, player_);
  }
  void addMonsterEvent(TimePeriod time, Monster::ID monst) {
    monsterEvents_.emplace(currentTime_ + time, monst);
  }
  [[nodiscard]] auto &getFloor(this auto &&self, FloorSpecifier floorId) noexcept {
    if constexpr (InDebug) {
      if (floorId.floor < 0 || static_cast<std::size_t>(floorId.floor) >= self.floorData_.size()) {
        Logging::log << "getFloor out of bounds: floor " << floorId.floor << " (size " << self.floorData_.size() << ")\n";
        return std::forward_like<decltype(self)>(self.floorData_[0]);
      }
    }
    return std::forward_like<decltype(self)>(self.floorData_[floorId.floor]);
  }
  [[nodiscard]] bool floorInBound(FloorSpecifier floorId) const noexcept {
    return floorId.floor >= 0 && static_cast<std::size_t>(floorId.floor) < floorData_.size();
  }
  [[nodiscard]] bool inBound(Location loc) const noexcept {
    return floorInBound(loc.mapPos) && getFloor(loc.mapPos).inBounds(loc.pos);
  }
  [[nodiscard]] constexpr auto &getObjects(this auto &&self, Location loc) noexcept {
    return self.getFloor(loc.mapPos).getObjects(loc.pos);
  }
  [[nodiscard]] constexpr auto &getMonster(this auto &&self, Location loc) noexcept {
    return self.getFloor(loc.mapPos).getMonster(loc.pos);
  }
  [[nodiscard]] constexpr auto &getTerrainType(this auto &&self, Location loc) noexcept {
    return self.getFloor(loc.mapPos).getTerrainType(loc.pos);
  }
  [[nodiscard]] constexpr auto getTile(this auto &&self, Location loc) noexcept {
    return self.getFloor(loc.mapPos).getTile(loc.pos);
  }
  [[nodiscard]] constexpr bool isOpenTile(Location loc) const noexcept {
    return floorInBound(loc.mapPos) && getFloor(loc.mapPos).isOpenTile(loc.pos);
  }
  [[nodiscard]] constexpr bool isOpenTerrain(Location loc) const noexcept {
    return floorInBound(loc.mapPos) && getFloor(loc.mapPos).isOpenTerrain(loc.pos);
  }
  [[nodiscard]] auto tryGetMonster(this auto &&self, Monster::ID id) noexcept {
    auto found = self.monsterMap_.find(id);
    return found == self.monsterMap_.end() ? OptionalReference<std::remove_reference_t<decltype(std::forward_like<decltype(self)>(*found->second))>>() : OptionalReference(std::forward_like<decltype(self)>(*found->second));
  }
  [[nodiscard]] auto &getMonster(this auto &&self, Monster::ID id) noexcept {
    return *self.tryGetMonster(id);
  }
  [[nodiscard]] constexpr bool containsMonster(Monster::ID id) const noexcept {
    return monsterMap_.contains(id);
  }
  [[nodiscard]] std::unique_ptr<Monster> removeMonster(Monster::ID id) noexcept {
    return std::move(monsterMap_.extract(id).mapped());
  }
  [[nodiscard]] constexpr Monster::ID nextMonsterId() noexcept {
    return mIdGenerator_.next();
  }
  constexpr Monster &insertMonster(std::unique_ptr<Monster> monst) noexcept {
    auto id = monst->getId();
    auto ret = monsterMap_.insert({id, std::move(monst)});
    return *ret.first->second;
  }
  void passTime(TimePeriod numTurns) noexcept {
    monsterEvents_.emplace(currentTime_ + numTurns, player_);
    while (true) {
      auto [timeOut, mMonst] = monsterEvents_.top();
      monsterEvents_.pop();
      currentTime_ = timeOut;
      if (mMonst == player_) {
        return;
      }
      tryGetMonster(mMonst).doIfValue([this, mMonst](Monster &monst) {
        TimePeriod nextAction = monst.runAI(*this);
        if (nextAction.future()) {
          addMonsterEvent(nextAction, mMonst);
        }
      });
    }
  }
  constexpr void broadcastEvent(Location eventLoc, auto &&func) noexcept;
  constexpr void broadcastItemPickup(const Monster &monster, const Object &object) noexcept;
  constexpr void broadcastMonsterHitMonster(const Monster::HitReturn &hitInfo, const Monster &attacker, Monster &attacked) noexcept;
  constexpr void broadcastMonsterHitWall(const Monster &attacker, Location loc) noexcept;
  constexpr void broadcastMonsterAte(const Monster &eater, const Object &eaten) noexcept;
  void printDebug(std::string_view v) noexcept {
    eventViewer_->debug(v);
  }
  void printItemPickup(const Monster &monster, const Object &object) noexcept {
    eventViewer_->itemPickup(monster, object);
  }
  void printMonsterHitMonster(const Monster::HitReturn &hitinfo, const Monster &attacker, const Monster &attacked) noexcept {
    eventViewer_->monsterHitMonster(hitinfo, attacker, attacked);
  }
  void printMonsterHitWall(const Monster &attacker, Location loc) noexcept {
    eventViewer_->monsterHitWall(attacker, loc);
  }
  void printMonsterAte(const Monster &eater, const Object &eaten) noexcept {
    eventViewer_->monsterAte(eater, eaten);
  }
  [[nodiscard]] Monster &getPlayer() noexcept {
    return getMonster(player_);
  }
  [[nodiscard]] const Monster &getPlayer() const noexcept {
    return getMonster(player_);
  }
  [[nodiscard]] GameTime getTime() const noexcept {
    return currentTime_;
  }
  void setEventViewer(std::unique_ptr<EventViewer> viewer) noexcept {
    eventViewer_ = std::move(viewer);
  }
  struct MonsterActionEvent {
    MonsterActionEvent(GameTime timeOut_, Monster::ID monst_) noexcept : timeOut(timeOut_), monst(monst_) {}
    GameTime timeOut;
    Monster::ID monst;
    [[nodiscard]] constexpr std::weak_ordering operator<=>(const MonsterActionEvent &other) const noexcept {
      return timeOut <=> other.timeOut;
    }
  };

private:
  std::unordered_map<Monster::ID, std::unique_ptr<Monster>> monsterMap_;
  std::priority_queue<MonsterActionEvent, std::vector<MonsterActionEvent>, std::greater<>> monsterEvents_;
  std::vector<WorldFloor> floorData_;
  GameTime currentTime_;
  Monster::ID::Generator mIdGenerator_;
  Monster::ID player_;
  std::unique_ptr<EventViewer> eventViewer_;

  friend std::size_t toStream(std::ostream &out, const GameState &input);
  friend GameState fromStream(std::istream &in, std::size_t &numRead, SerializationLib::Tag<GameState> /**/);
};

export std::size_t toStream(std::ostream &out, const GameState &input);
export GameState fromStream(std::istream &in, std::size_t &numRead, SerializationLib::Tag<GameState> /**/);

export void addMonsters(GameState &state, FloorSpecifier floor, int count) noexcept;

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

export void addMonsters(GameState &state, FloorSpecifier floor, int count) noexcept {
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

Monster &WorldFloor::IDToMonster::operator()(Monster::ID monst) const noexcept {
  return state->getMonster(monst);
}

constexpr void GameState::broadcastEvent(Location eventLoc, auto &&func) noexcept {
  auto [pos, floor] = eventLoc;
  auto listeners = getFloor(floor).getEventListeners(*this) | std::views::filter([this, pos](const Monster &viewer) { return viewer.inLineOfSight(*this, pos); });
  std::ranges::for_each(listeners, func);
}

constexpr void GameState::broadcastItemPickup(const Monster &monster, const Object &object) noexcept {
  broadcastEvent(monster.getLoc(), [this, &monster, &object](Monster &viewer) {
    viewer.informItemPickup(*this, monster, object);
  });
}

constexpr void GameState::broadcastMonsterHitMonster(const Monster::HitReturn &hitInfo, const Monster &attacker, Monster &attacked) noexcept {
  auto inform = [this, &hitInfo, &attacker, &attacked](Monster &viewer) {
    viewer.informMonsterHitMonster(*this, hitInfo, attacker, attacked);
  };
  if (attacked.isAlive() && !attacked.caresEvent())
    inform(attacked);
  broadcastEvent(attacker.getLoc(), inform);
}

constexpr void GameState::broadcastMonsterHitWall(const Monster &attacker, Location loc) noexcept {
  auto inform = [this, &attacker, &loc](Monster &viewer) {
    viewer.informMonsterHitWall(*this, attacker, loc);
  };
  broadcastEvent(attacker.getLoc(), inform);
}

constexpr void GameState::broadcastMonsterAte(const Monster &eater, const Object &eaten) noexcept {
  broadcastEvent(eater.getLoc(), [this, &eater, &eaten](Monster &viewer) {
    viewer.informMonsterAte(*this, eater, eaten);
  });
}

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

constexpr bool Monster::isOpenMove(GameState &state, Dir d) const noexcept {
  return state.getFloor(loc_.mapPos).isOpenTile(loc_.pos + d);
}

constexpr bool Monster::inLineOfSight(const GameState &state, Position pos) const noexcept {
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
constexpr TimePeriod Monster::takeItem(GameState &state, ObjectContainer &container, std::size_t index) noexcept {
  static constexpr std::size_t PickUpItemSpeedFraction = 10;
  state.broadcastItemPickup(*this, container[index]);
  inventory_.addObject(container.remove(index));
  return getSpeed() / PickUpItemSpeedFraction;
}

constexpr TimePeriod Monster::generalMove(GameState &state, Location nLoc, MoveMode mode) noexcept {
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

constexpr TimePeriod Monster::generalMove(GameState &state, Position nPos, MoveMode mode) noexcept {
  return generalMove(state, Location(nPos, loc_.mapPos), mode);
}
constexpr TimePeriod Monster::generalMove(GameState &state, Dir d, MoveMode mode) noexcept {
  return generalMove(state, Location(loc_.pos + d, loc_.mapPos), mode);
}

[[nodiscard]] constexpr TimePeriod Monster::goUpStair(GameState &state, MoveMode m) noexcept {
  if (state.getTerrainType(loc_) == TerrainType::UpStair) {
    return generalMove(state, loc_.up(), m);
  }
  return TimePeriod(0);
}

[[nodiscard]] constexpr TimePeriod Monster::goDownStair(GameState &state, MoveMode m) noexcept {
  if (state.getTerrainType(loc_) == TerrainType::DownStair) {
    return generalMove(state, loc_.down(), m);
  }
  return TimePeriod(0);
}

constexpr void Monster::informItemPickup(GameState &state, const Monster &grabber, const Object &grabbed) noexcept {
  if (isPlayer()) {
    state.printItemPickup(grabber, grabbed);
    return;
  }
  if (brain_.config.hatesItemPickup()) {
    brain_.target = grabber.getId();
  }
}

constexpr void Monster::informMonsterHitMonster(GameState &state, const HitReturn &hitinfo, const Monster &attacker, const Monster &attacked) noexcept {
  if (isPlayer()) {
    state.printMonsterHitMonster(hitinfo, attacker, attacked);
    return;
  }
  if (attacked == *this) {
    brain_.target = attacker.getId();
  }
}

constexpr void Monster::informMonsterHitWall(GameState &state, const Monster &attacker, Location loc) noexcept { // NOLINT(readability-make-member-function-const)
  if (isPlayer()) {
    state.printMonsterHitWall(attacker, loc);
    return;
  }
}

constexpr void Monster::informMonsterAte(GameState &state, const Monster &eater, const Object &eaten) noexcept { // NOLINT(readability-make-member-function-const)
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

constexpr TimePeriod Monster::dropItem(GameState &state, std::size_t i) noexcept {
  static constexpr std::size_t DropItemSpeedFraction = 10;
  auto obj = removeFromInvent(i);
  state.getObjects(getLoc()).addObject(std::move(obj));
  return getSpeed() / DropItemSpeedFraction;
}

[[nodiscard]] constexpr std::pair<bool, bool> getNMirror(WorldFloor &floor, Position lastPos, Position cSpot) noexcept {
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
  return std::make_pair(choice!=0, choice!=1);
}

constexpr Location runPath(GameState &state, Location start, Dir dir, int maxDist, auto& monsterHit, bool bounceOnWall) noexcept { // TODO: ben - add animation for this.
  if(dir.noMove()){
    return start;
  }
  auto [startPos, floorId] = start;
  auto &floor = state.getFloor(floorId);
  Position lastPos = startPos;
  Position basePos = startPos;
  auto iter = ++PathIterable(dir).begin();
  bool mirrorX = false;
  bool mirrorY = false;
  while(true){
    const Dir cDir = (*iter).mirror(mirrorX,mirrorY);
    const int dist = Dir::chessboard(cDir);
    if(dist>maxDist)
      break;
    Position cSpot = basePos+cDir;
    if(!floor.isOpenTerrain(cSpot)){
      if(!bounceOnWall)
        break;
      auto [mX,mY] = getNMirror(floor,lastPos,cSpot);
      mirrorX^=mX;
      mirrorY^=mY;
      const Dir nDir = (*iter).mirror(mirrorX,mirrorY);
      basePos=lastPos-nDir;
      continue;
    }
    if (auto targetID = floor.getMonster(cSpot)) {
      int nRemDist = monsterHit(state.getMonster(targetID),maxDist-dist);
      maxDist = dist+nRemDist;
    }
    lastPos = cSpot;
    ++iter;
  }
  return {lastPos,floorId};
}

constexpr void sendItemFlying(GameState &state, Monster& source, std::unique_ptr<Object> obj, Location start, Dir dir) {
  const int maxDist = source.getMaxThrowingDistance();
  auto onHit = [&state,&source,&obj=*obj , maxDist](Monster& target, int distLeft){
    Health damage = obj.type() == ObjectType::Knife ? 5 : 2;
    if (distLeft < maxDist / 2) // TODO: ben - replace this with to-hit penalty once to-hit is added
      damage /= 2;
    monsterHitMonster(state, source, target, {damage});
    return 0;
  };
  state.getObjects(runPath(state,start,dir,maxDist,onHit,false)).addObject(std::move(obj));
}

constexpr void createBeam(GameState &state, Monster &source, Location start, Dir dir, int maxDist, Dice::Group damage) {
  auto onHit = [&](Monster &target, int distLeft) {
    monsterHitMonster(state, source, target, {damage()});
    return distLeft-Rnd::rnd(5);
  };
  runPath(state,start,dir,maxDist,onHit,true);
}

constexpr TimePeriod Monster::throwItem(GameState &state, std::size_t i, Dir dir, int count) noexcept {
  sendItemFlying(state, *this,removeFromInvent(i, count), getLoc(), dir);
  return getSpeed();
}

constexpr TimePeriod Monster::eatItem(GameState &state, std::size_t i, bool fromFloor) noexcept {
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
  createBeam(state, *this, loc_, dir,Rnd::uniform_int(10,20), body_.damage);
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
constexpr void Monster::kill(GameState &state, ObjectContainer &itemsTo) noexcept {
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
