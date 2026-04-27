export module GameState;
import Common;
import DungeonMaker;
export import GameTypes;
import MonsterClassConfig;
import SerializationLib;

export class GameState;
export class Monster {
public:
  struct MonsterBodyInit {
    [[nodiscard]] static constexpr MonsterBodyInit make(const MonsterClassInfo &mInfo) noexcept {
      return {.speed = mInfo.speed, .plan = mInfo.bodyPlan, .maxHealth = mInfo.baseHealth, .maxMP = mInfo.baseMP, .damage = mInfo.naturalWeapon, .mClass = mInfo.mClass, .maxThrowingDistance = mInfo.maxThrowingDistance};
    }
    TimePeriod speed;
    MustInit<BodyPlan> plan;
    MustInit<Health> maxHealth;
    MustInit<MP> maxMP;
    Health currentHealth = maxHealth;
    MP currentMP = maxMP;
    Dice::Group damage;
    MustInit<MonsterClass> mClass;
    MustInit<int> maxThrowingDistance;
    bool alive = true;
    bool immortal = false;
  };
  struct MonsterBody {
    explicit MonsterBody(MonsterBodyInit body) noexcept : speed(body.speed), plan(body.plan), maxHealth(body.maxHealth), maxMP(body.maxMP), health(body.currentHealth), mp(body.currentMP), maxThrowingDistance(body.maxThrowingDistance), damage(body.damage), mClass(body.mClass), alive(body.alive), immortal(body.immortal) {}
    TimePeriod speed;
    BodyPlan plan;
    Health maxHealth;
    MP maxMP;
    Health health;
    MP mp;
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
  [[nodiscard]] TimePeriod generalMove(GameState &state, Location nLoc, MoveMode m) noexcept;
  [[nodiscard]] TimePeriod generalMove(GameState &state, Position nPos, MoveMode m) noexcept;
  [[nodiscard]] TimePeriod generalMove(GameState &state, Dir d, MoveMode m) noexcept;
  [[nodiscard]] TimePeriod goUpStair(GameState &state, MoveMode m) noexcept;
  [[nodiscard]] TimePeriod goDownStair(GameState &state, MoveMode m) noexcept;
  [[nodiscard]] TimePeriod runAI(GameState &state) noexcept;
  [[nodiscard]] constexpr MonsterClass getClass() const noexcept { return body_.mClass; };
  [[nodiscard]] constexpr TimePeriod getSpeed() const noexcept { return body_.speed; };
  [[nodiscard]] constexpr Health getHealth() const noexcept { return body_.health; }
  [[nodiscard]] constexpr Health getMaxHealth() const noexcept { return body_.maxHealth; }
  [[nodiscard]] constexpr int getMP() const noexcept { return body_.mp; }
  [[nodiscard]] constexpr bool useMP(MP amount) noexcept {
    if (amount > body_.mp)
      return false;
    body_.mp -= amount;
    return true;
  }
  constexpr void drainMP(MP amount) noexcept {
    body_.mp = std::max(0, body_.mp - amount);
  }
  [[nodiscard]] constexpr int getMaxMP() const noexcept { return body_.maxMP; }
  [[nodiscard]] constexpr int getMaxThrowingDistance() const noexcept { return body_.maxThrowingDistance; }
  [[nodiscard]] constexpr bool removeHealth(Health amount) noexcept {
    body_.health -= amount;
    if (body_.health > 0)
      return false;
    if (!body_.immortal)
      return true;
    body_.health = body_.maxHealth;
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
  [[nodiscard]] bool inLineOfSight(const GameState &state, Position pos) const noexcept;
  [[nodiscard]] constexpr bool caresEvent() const noexcept { return brain_.config.caresEvent(); }
  [[nodiscard]] bool isOpenMove(GameState &state, Dir d) const noexcept;
  void informItemPickup(GameState &state, const Monster &grabber, const Object &grabbed) noexcept;
  void informMonsterHitMonster(GameState &state, const HitReturn &hitinfo, const Monster &attacker, const Monster &attacked) noexcept;
  void informMonsterHitWall(GameState &state, const Monster &attacker, Location loc) noexcept;
  void informMonsterAte(GameState &state, const Monster &eater, const Object &eaten) noexcept;
  friend void monsterHitMonster(GameState &state, Monster &attacker, Monster &attacked, AttackInfo info) noexcept;
  [[nodiscard]] HitReturn hitBy(AttackInfo info) noexcept;
  constexpr void gainExp(int n) noexcept { exp_ += n; }
  void kill(GameState &state, ObjectContainer &itemsTo) noexcept;
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
  [[nodiscard]] TimePeriod takeItem(GameState &state, ObjectContainer &container, std::size_t index) noexcept;
  [[nodiscard]] TimePeriod dropItem(GameState &state, std::size_t i) noexcept;
  [[nodiscard]] TimePeriod throwItem(GameState &state, std::size_t i, Dir dir, int count) noexcept;
  [[nodiscard]] TimePeriod eatItem(GameState &state, std::size_t i, bool fromFloor) noexcept;
  [[nodiscard]] TimePeriod equipItem(GameState &state, std::size_t inventoryIdx) noexcept;
  [[nodiscard]] TimePeriod unequipItem(GameState &state, std::int8_t slot) noexcept;
  [[nodiscard]] constexpr const BodyPlan &getBodyPlan() const noexcept { return body_.plan; }
  [[nodiscard]] constexpr std::int8_t equipmentSize() const noexcept { return body_.plan.totalSlots(); }
  [[nodiscard]] constexpr const Object *viewEquipped(std::int8_t slot) const noexcept { return equipment_[slot].get(); }
  [[nodiscard]] TimePeriod rest() noexcept;
  [[nodiscard]] TimePeriod hitMonster(GameState &state, Monster &target) noexcept;
  [[nodiscard]] TimePeriod castBeam(GameState &state, Dir dir) noexcept;
  Monster(MonsterBodyInit body, Location loc, ID id, MonsterBrainConfig brain) noexcept : Monster(MonsterBody(body), loc, id, MonsterBrain{.config = brain}) {}
  Monster(MonsterBody body, Location loc, ID id, MonsterBrain brain) noexcept : body_(body), brain_(brain), loc_(loc), id_(id) {
    equipment_ = std::make_unique<std::unique_ptr<Object>[]>(body_.plan.totalSlots());
  };
  void serializeTo(std::ostream &out) const noexcept;
  static Monster deserializeFrom(std::istream &in);

  constexpr void setImmortal(bool immortal = true) noexcept { body_.immortal = immortal; }

private:
  constexpr void setDead(bool dead = true) noexcept { body_.alive = !dead; }
  ObjectContainer inventory_;
  std::unique_ptr<std::unique_ptr<Object>[]> equipment_;
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
export void toStream(std::ostream &out, const Monster &input);
export Monster fromStream(std::istream &in, SerializationLib::Tag<Monster> /**/);

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
  [[nodiscard]] constexpr auto &get##name(this auto &&self, Position pos) noexcept { return self.get##name##Arr()[pos]; } // NOLINT(bugprone-macro-parentheses)
  ACCESSORS_WORLD_FLOOR(Objects)
  ACCESSORS_WORLD_FLOOR(Monster)
  ACCESSORS_WORLD_FLOOR(TerrainType)
#undef ACCESSORS_WORLD_FLOOR

  [[nodiscard]] constexpr auto getTile(Position pos) noexcept { return WorldTile(getObjects(pos), getMonster(pos), getTerrainType(pos)); }
  [[nodiscard]] constexpr auto getTile(Position pos) const noexcept { return ConstWorldTile(getObjects(pos), getMonster(pos), getTerrainType(pos)); }
  [[nodiscard]] constexpr auto isOpenTerrain(Position pos) const noexcept {
    return inBounds(pos) && getTerrainType(pos) != TerrainType::Wall;
  }
  [[nodiscard]] constexpr auto seeThrough(Position pos) const noexcept {
    return isOpenTerrain(pos);
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
  constexpr WorldFloor(WorldFloor &) noexcept = delete;
  constexpr WorldFloor &operator=(WorldFloor &) noexcept = delete;
  constexpr ~WorldFloor() noexcept = default;
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
  [[nodiscard]] constexpr auto &getEventListeners() noexcept {
    return EventListenerArr_;
  }
  [[nodiscard]] constexpr auto &getEventListenersArr() noexcept {
    return EventListenerArr_;
  }
  [[nodiscard]] constexpr const auto &getEventListenersArr() const noexcept {
    return EventListenerArr_;
  }

private:
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

export void toStream(std::ostream &out, const WorldFloor &input);
export WorldFloor fromStream(std::istream &in, SerializationLib::Tag<WorldFloor> /**/);
export class EventViewer {
public:
  virtual void itemPickup(const Monster &grabber, const Object &grabbed) noexcept = 0;
  virtual void itemEquipped(const Monster &wearer, const Object &item) noexcept = 0;
  virtual void itemUnequipped(const Monster &wearer, const Object &item) noexcept = 0;
  virtual void equipSlotsFull(const Monster &wearer, const Object &item) noexcept = 0;
  virtual void monsterHitMonster(const Monster::HitReturn &hitinfo, const Monster &attacker, const Monster &attacked) noexcept = 0;
  virtual void monsterHitWall(const Monster &attacker, Location loc) noexcept = 0;
  virtual void monsterAte(const Monster &eater, const Object &eaten) noexcept = 0;
  virtual void beamStep(Location loc) noexcept = 0;
  virtual void debug(std::string_view message) noexcept = 0;
  EventViewer() noexcept = default;
  EventViewer(const EventViewer &) noexcept = default;
  EventViewer &operator=(const EventViewer &) noexcept = default;
  EventViewer(EventViewer &&) noexcept = default;
  EventViewer &operator=(EventViewer &&) noexcept = default;
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
    return OptionalReference(found == self.monsterMap_.end() ? nullptr : &std::forward_like<decltype(self)>(*found->second));
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
  void broadcastEvent(Location eventLoc, auto &&func) noexcept;
  void broadcastItemPickup(const Monster &monster, const Object &object) noexcept;
  void broadcastMonsterHitMonster(const Monster::HitReturn &hitInfo, const Monster &attacker, Monster &attacked) noexcept;
  void broadcastMonsterHitWall(const Monster &attacker, Location loc) noexcept;
  void broadcastMonsterAte(const Monster &eater, const Object &eaten) noexcept;
  void broadcastBeamStep(Location loc) noexcept;
  void printDebug(std::string_view v) noexcept {
    eventViewer_->debug(v);
  }
  void printItemPickup(const Monster &monster, const Object &object) noexcept {
    eventViewer_->itemPickup(monster, object);
  }
  void printItemEquipped(const Monster &wearer, const Object &item) noexcept {
    eventViewer_->itemEquipped(wearer, item);
  }
  void printItemUnequipped(const Monster &wearer, const Object &item) noexcept {
    eventViewer_->itemUnequipped(wearer, item);
  }
  void printEquipSlotsFull(const Monster &wearer, const Object &item) noexcept {
    eventViewer_->equipSlotsFull(wearer, item);
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
  void printBeamStep(Location loc) noexcept {
    eventViewer_->beamStep(loc);
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

  friend void toStream(std::ostream &out, const GameState &input);
  friend GameState fromStream(std::istream &in, SerializationLib::Tag<GameState> /**/);
};

export void toStream(std::ostream &out, const GameState &input);
export GameState fromStream(std::istream &in, SerializationLib::Tag<GameState> /**/);
