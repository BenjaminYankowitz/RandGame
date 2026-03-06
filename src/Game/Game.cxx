export module Game;
import Common;
import DungeonMaker;
import GameTypes;
import MonsterClassConfig;

export struct HitReturn {
  MustInit<bool> killed;
};

struct NoTarget{};

struct AttackInfo {
  MustInit<Health> damage;
};

export class Object {
public:
  Object(int count, ObjectType type, Material mat) noexcept : count_(count), type_(type), mat_(mat) {}
  [[nodiscard]] constexpr bool isCombinable() const noexcept {
    return artifactStatus_ == ArtifactId::Normal;
  }
  [[nodiscard]] constexpr bool canCombine(const Object &other) const noexcept {
    return other.type_ == type_ && other.mat_ == mat_ && isCombinable() && other.isCombinable();
  }
  constexpr void combine(std::unique_ptr<Object> other) noexcept {
    count_ += other->count_;
  }
  [[nodiscard]] constexpr int count() const noexcept { return count_; }
  [[nodiscard]] constexpr ObjectType type() const noexcept { return type_; }
  [[nodiscard]] constexpr Material mat() const noexcept { return mat_; }
  [[nodiscard]] constexpr ArtifactId artifactStatus() const noexcept { return artifactStatus_; }

private:
  int count_;
  ObjectType type_;
  Material mat_;
  ArtifactId artifactStatus_ = ArtifactId::Normal;
};

export class ObjectContainer {
  using iterator = IteratorImpl<std::unique_ptr<Object>, ObjectContainer, Object &, [](std::unique_ptr<Object> &p) -> Object & { return *p; }>;
  using const_iterator = IteratorImpl<const std::unique_ptr<Object>, ObjectContainer, const Object &, [](const std::unique_ptr<Object> &p) -> const Object & { return *p; }>;

public:
  ObjectContainer() = default;
  ObjectContainer(ObjectContainer &) = delete;
  ObjectContainer(ObjectContainer &&) = default;
  constexpr void addObject(std::unique_ptr<Object> obj) noexcept {
    auto v = std::ranges::find_if(*this, [&obj = *obj](const Object &oObj) { return obj.canCombine(oObj); });
    if (v != end()) {
      v->combine(std::move(obj));
    } else {
      impl_.push_back(std::move(obj));
    }
  }
  [[nodiscard]] constexpr std::size_t size() const noexcept {
    return impl_.size();
  }
  [[nodiscard]] constexpr bool empty() const noexcept {
    return size() == 0;
  }
  [[nodiscard]] constexpr Object &operator[](std::size_t i) {
    return *impl_[i];
  }
  [[nodiscard]] constexpr const Object &operator[](std::size_t i) const {
    return *impl_[i];
  }
  [[nodiscard]] constexpr iterator begin() noexcept {
    return iterator(impl_.data());
  }
  [[nodiscard]] constexpr const_iterator begin() const noexcept {
    return const_iterator(impl_.data());
  }
  [[nodiscard]] constexpr iterator end() noexcept {
    return iterator(impl_.data() + impl_.size());
  }
  [[nodiscard]] constexpr const_iterator end() const noexcept {
    return const_iterator(impl_.data() + impl_.size());
  }
  [[nodiscard]] constexpr std::unique_ptr<Object> remove(std::size_t i) {
    std::unique_ptr<Object> ptr = std::move(impl_[i]);
    if (i + 1 != impl_.size()) {
      impl_[i] = std::move(impl_.back());
    }
    impl_.pop_back();
    return ptr;
  }
  [[nodiscard]] constexpr Object &front() noexcept {
    return operator[](0);
  }
  [[nodiscard]] constexpr const Object &front() const noexcept {
    return operator[](0);
  }
  [[nodiscard]] constexpr Object &back() noexcept {
    return operator[](size() - 1);
  }
  [[nodiscard]] constexpr const Object &back() const noexcept {
    return operator[](size() - 1);
  }

private:
  SmallVector<std::unique_ptr<Object>> impl_;
};

class MonsterId {
  friend class MonsterIdGenerator;
  using idImpl = int;

public:
  [[nodiscard]] constexpr MonsterId() noexcept : id_(0) {}
  [[nodiscard]] constexpr std::size_t hash() const noexcept { return std::hash<idImpl>{}(id_); }
  [[nodiscard]] bool operator==(const MonsterId &o) const noexcept = default;
  [[nodiscard]] static constexpr MonsterId null() noexcept { return MonsterId{NullV}; }
  constexpr void clear() noexcept { id_ = NullV; }
  [[nodiscard]] constexpr bool isNull() const noexcept { return id_ == NullV; }

private:
  static constexpr idImpl NullV = 0;
  [[nodiscard]] constexpr explicit MonsterId(std::size_t id) noexcept : id_(id) {}
  idImpl id_;
};

class MonsterIdGenerator {
public:
  MonsterId next() noexcept { return MonsterId(value_++); }

private:
  MonsterId::idImpl value_ = MonsterId::NullV + 1;
};
export class GameState;

export class Monster {
  private:
  struct CreateKey{};

  struct MonsterBody {
    TimePeriod speed;
    MustInit<Health> health;
    Dice::Group damage;
    MustInit<MonsterClass> mClass;
    bool alive = true;
  };

public:
  static MonsterId createMonster(GameState &game, Location loc, MonsterClass mClass, bool isPlayer = false) noexcept;
  [[nodiscard]] constexpr TimePeriod generalMove(GameState &state, Dir d, MoveMode m) noexcept;
  [[nodiscard]] TimePeriod runAI(GameState &state) noexcept;
  [[nodiscard]] constexpr MonsterClass getClass() const noexcept { return mClass_; };
  [[nodiscard]] constexpr TimePeriod getSpeed() const noexcept { return speed_; };
  [[nodiscard]] constexpr Health getHealth() const noexcept { return health_; }
  [[nodiscard]] constexpr Location getLoc() const noexcept { return loc_; }
  [[nodiscard]] constexpr MonsterId getId() const noexcept { return id_; }
  [[nodiscard]] constexpr const ObjectContainer &viewInventory() const noexcept { return inventory_; }
  [[nodiscard]] constexpr bool isPlayer() const noexcept { return brain_.isPlayer(); }
  [[nodiscard]] constexpr bool isAlive() const noexcept { return alive_; }
  [[nodiscard]] constexpr bool caresEvent() const noexcept { return brain_.caresEvent(); }
  [[nodiscard]] constexpr bool isOpenMove(GameState &state, Dir d) const noexcept;
  constexpr void informItemPickup(GameState &state, const Monster &grabber, const Object &grabbed) noexcept;
  constexpr void informMonsterAttack(GameState &state, HitReturn hitinfo, const Monster &attacker, const Monster &attacked) noexcept;
  [[nodiscard]] HitReturn hitBy(GameState &state, AttackInfo info) noexcept;
  constexpr void kill(GameState &state) noexcept;
  [[nodiscard]] constexpr std::unique_ptr<Object> removeFromInvent(std::size_t i) noexcept { return inventory_.remove(i); }
  [[nodiscard]] TimePeriod goToTarget(GameState &state, NoTarget /*unused*/) noexcept {
    std::array IntToDir = {
        Dir(-1, -1), Dir(-1, 0), Dir(-1, 1), Dir(0, -1),
        Dir(0, 1), Dir(1, -1), Dir(1, 0), Dir(1, 1)};
    auto *const endIter = std::ranges::remove_if(IntToDir, [this, &state](Dir d) {
                            return !isOpenMove(state, d);
                          }).begin();
    const std::size_t validDirs = std::distance(IntToDir.begin(), endIter);
    if (validDirs == 0) {
      return TimePeriod(4);
    }
    std::uniform_int_distribution<std::size_t> dist(0, validDirs - 1);
    const std::size_t index = Rnd::get(dist);
    TimePeriod ret = generalMove(state, IntToDir[index], MoveMode::move());
    if (ret.future()) {
      return ret;
    }
    return TimePeriod(4);
  }
  [[nodiscard]] TimePeriod goToTarget(GameState &state, MonsterId targetId) noexcept;
  [[nodiscard]] TimePeriod goToTarget(GameState &state, Location target) noexcept {
    return pathTo(state, target, false);
  }
  [[nodiscard]] TimePeriod pathTo(GameState &state, Location target, bool attack) noexcept;
  [[nodiscard]] TimePeriod reThink() noexcept {
    target_ = NoTarget{};
    return TimePeriod(1);
  }
  [[nodiscard]] constexpr TimePeriod takeItem(GameState &state, ObjectContainer &container, std::size_t index) noexcept;
  [[nodiscard]] constexpr TimePeriod dropItem(GameState &state, std::size_t i) noexcept;
  [[nodiscard]] constexpr TimePeriod dropEverthing(GameState &state) noexcept;
  [[nodiscard]] TimePeriod hitMonster(GameState &state, Monster &target) noexcept;
  Monster(CreateKey /*unused*/, MonsterBody body, Location loc, MonsterId id, MonsterBrain brain) noexcept : 
  speed_(body.speed), loc_(loc), health_(body.health), damage_(body.damage),id_(id), brain_(brain), mClass_(body.mClass), alive_(body.alive){};
private:
  constexpr void seAlive(bool alive = true) noexcept { alive_ = alive; }
  constexpr void setDead(bool dead = true) noexcept { alive_ = !dead; }                                                                                          
  ObjectContainer inventory_;
  TimePeriod speed_;
  Location loc_;
  std::variant<NoTarget, MonsterId, Location> target_;
  Health health_;
  Dice::Group damage_;
  MonsterId id_;
  MonsterBrain brain_;
  MonsterClass mClass_;
  bool alive_;
};
[[nodiscard]] constexpr bool operator==(const Monster& lhs, const Monster& rhs) noexcept {
  return &lhs==&rhs;
}

template <>
struct std::hash<MonsterId> {
  std::size_t operator()(MonsterId s) const noexcept { return s.hash(); }
};

class WorldTile {
public:
  constexpr WorldTile(ObjectContainer &objectsI, MonsterId &monsterI, TerrainType &terrainTypeI) noexcept : objects(objectsI), monster(monsterI), terrainType(terrainTypeI) {}
  ObjectContainer &objects;
  MonsterId &monster;
  TerrainType &terrainType;
};

export class ConstWorldTile {
public:
  constexpr ConstWorldTile(const ObjectContainer &objectsI, const MonsterId &monsterI, const TerrainType &terrainTypeI) noexcept : objects(objectsI), monster(monsterI), terrainType(terrainTypeI) {}
  constexpr ConstWorldTile(const WorldTile &tile) noexcept : objects(tile.objects), monster(tile.monster), terrainType(tile.terrainType) {} //NOLINT(google-explicit-constructor)
  const ObjectContainer &objects;
  const MonsterId &monster;
  const TerrainType &terrainType;
};

export class WorldFloor {
public:
#define ACCESSORS_WORLD_FLOOR(name)                                                                                                      \
  [[nodiscard]] constexpr auto begin##name(this auto &&self) noexcept { return self.name##Arr_.begin(); }                                \
  [[nodiscard]] constexpr auto end##name(this auto &&self) noexcept { return self.name##Arr_.end(); }                                    \
  [[nodiscard]] constexpr auto iterable##name(this auto &&self) noexcept { return Iterable(self.begin##name(), self.end##name()); }      \
  [[nodiscard]] constexpr auto &get##name##Arr(this auto &&self) noexcept { return std::forward_like<decltype(self)>(self.name##Arr_); } \
  [[nodiscard]] constexpr auto &get##name(this auto &&self, Position pos) noexcept { return self.get##name##Arr()[pos.y, pos.x]; }
  ACCESSORS_WORLD_FLOOR(Objects)
  ACCESSORS_WORLD_FLOOR(Monster)
  ACCESSORS_WORLD_FLOOR(TerrainType)
#undef Accessors_WorldFloor

  [[nodiscard]] constexpr auto getTile(Position pos) noexcept { return WorldTile(getObjects(pos), getMonster(pos), getTerrainType(pos)); }
  [[nodiscard]] constexpr auto getTile(Position pos) const noexcept { return ConstWorldTile(getObjects(pos), getMonster(pos), getTerrainType(pos)); }

  constexpr WorldFloor(std::size_t x, std::size_t y) noexcept : ObjectsArr_(y, x), MonsterArr_(y, x), TerrainTypeArr_(y, x) {}
  [[nodiscard]] constexpr std::size_t rows() const noexcept { return ObjectsArr_.rows(); }
  [[nodiscard]] constexpr std::size_t cols() const noexcept { return ObjectsArr_.cols(); }
  [[nodiscard]] constexpr bool inBounds(Position pos) const noexcept { 
    const auto [x,y] = pos;
    return ObjectsArr_.inBounds(y,x);
   }
  constexpr void addEventListener(MonsterId id) { EventListenerArr_.push_back(id); }
  [[nodiscard]] constexpr auto getEventListeners(GameState &state) noexcept {
    return EventListenersIterable(state, EventListenerArr_);
  }

private:
  Static2DArr<ObjectContainer> ObjectsArr_;
  Static2DArr<MonsterId> MonsterArr_;
  Static2DArr<TerrainType> TerrainTypeArr_;
  std::vector<MonsterId> EventListenerArr_;
  class EventListenersIterable {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = Monster;
    constexpr EventListenersIterable(GameState &state, std::vector<MonsterId> &arr) noexcept : state_(&state), arr_(&arr) {}
    class EventListenerSentinal {};
    class EventListenerIterator {
    public:
      using difference_type = difference_type;
      using value_type = value_type;
      [[nodiscard]] constexpr bool operator==(EventListenerSentinal /*unused*/) noexcept {
        return pos_ == arr_->size();
      }
      constexpr EventListenerIterator &operator++() noexcept;
      [[nodiscard]] constexpr Monster &operator*() const noexcept;
      constexpr EventListenerIterator operator++(int) noexcept {
        auto cp = *this;
        operator++();
        return cp;
      }

    private:
      friend EventListenersIterable;
      constexpr EventListenerIterator(GameState *state, std::vector<MonsterId> *arr) noexcept : state_(state), arr_(arr) {}
      GameState *state_;
      std::vector<MonsterId> *arr_;
      std::size_t pos_ = 0;
    };
    [[nodiscard]] constexpr EventListenerIterator begin() noexcept { return {state_, arr_}; }
    [[nodiscard]] constexpr static EventListenerSentinal end() noexcept { return {}; }

  private:
    GameState *state_;
    std::vector<MonsterId> *arr_;
  };
  friend constexpr bool operator==(EventListenersIterable::EventListenerSentinal end, EventListenersIterable::EventListenerIterator iter) noexcept;
  static_assert(std::input_or_output_iterator<EventListenersIterable::EventListenerIterator>);
};

WorldFloor createDungeon(std::size_t xDim, std::size_t yDim) {
  WorldFloor ret(xDim, yDim);
  // DungeonMaker::perlin<TerrainType::Wall, TerrainType::Empty>(ret.getTerrainTypeArr(), 16,4);
  return ret;
}

export class EventViewer {
public:
  virtual void itemPickup(const Monster& grabber, const Object& grabbed) noexcept = 0;
  virtual void monsterAttack(HitReturn hitinfo, const Monster& attacker, const Monster& attacked) noexcept = 0;
  virtual void debug(std::string_view message) noexcept = 0;
  virtual ~EventViewer(){};
};

template<class T, class Container, class Compare>
void clearQueue(std::priority_queue<T,Container,Compare>& queue) noexcept{
  using QueueT = std::priority_queue<T,Container,Compare>;
  struct Child : public QueueT {  
    using QueueT::c;
    explicit Child(QueueT&& queue) : QueueT{std::move(queue)} {}
    void clear() noexcept{
      c.clear();
    }
  };
  Child{std::move(queue)}.clear();
}

export class GameState  {
public:
  GameState() noexcept;
  void playerDied(){
    clearQueue(monsterEvents_);
    monsterEvents_.emplace(currentTime_,player_);
  }
  void addMonsterEvent(TimePeriod time, MonsterId monst) {
    monsterEvents_.emplace(currentTime_ + time, monst);
  }
  [[nodiscard]] auto &getFloor(this auto&& self, FloorSpecifier floorId) noexcept {
    return std::forward_like<decltype(self)>(self.floorData_[floorId.floor]);
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
  [[nodiscard]] auto &getMonster(this auto &&self, MonsterId id) noexcept {
    return *self.monsterMap_.find(id)->second;
  }
  [[nodiscard]] OptionalReference<Monster> tryGetMonster(MonsterId id) noexcept {
    auto found = monsterMap_.find(id);
    return found == monsterMap_.end() ? OptionalReference<Monster>() : OptionalReference<Monster>(*found->second);
  }
  [[nodiscard]] constexpr bool containsMonster(MonsterId id) const noexcept {
    return monsterMap_.contains(id);
  }
  std::unique_ptr<Monster> removeMonster(MonsterId id) noexcept {
    return std::move(monsterMap_.extract(id).mapped());
  }
  [[nodiscard]] constexpr MonsterId nextMonsterId() noexcept {
    return mIdGenerator_.next();
  }
  constexpr Monster& insertMonster(std::unique_ptr<Monster> monst) noexcept {
    auto& ret = *monst;
    monsterMap_.insert({monst->getId(), std::move(monst)});
    return ret;
  }
  void passTime(TimePeriod numTurns) noexcept {
    monsterEvents_.emplace(currentTime_ + numTurns, player_);
    currentTime_ += numTurns;
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
  constexpr void broadcastEvent(FloorSpecifier floor, auto&& func) noexcept;
  constexpr void broadcastItemPickup(const Monster &monster, const Object &object) noexcept;
  constexpr void broadcastMonsterAttack(HitReturn hitInfo, const Monster &attacker, Monster &attacked) noexcept;
  void printDebug(std::string_view v) noexcept {
    eventViewer_->debug(v);
  }
  void printItemPickup(const Monster &monster, const Object &object) noexcept {
    eventViewer_->itemPickup(monster, object);
  }
  void printMonsterAttack(HitReturn hitinfo, const Monster &attacker, const Monster &attacked) noexcept {
    eventViewer_->monsterAttack(hitinfo,attacker, attacked);
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
    MonsterActionEvent(GameTime timeOut_, MonsterId monst_) noexcept : timeOut(timeOut_), monst(monst_) {}
    GameTime timeOut;
    MonsterId monst;
    [[nodiscard]] constexpr std::weak_ordering operator<=>(const MonsterActionEvent &other) const noexcept {
      return timeOut <=> other.timeOut;
    }
  };
private:
  std::unordered_map<MonsterId, std::unique_ptr<Monster>> monsterMap_;
  std::priority_queue<MonsterActionEvent, std::vector<MonsterActionEvent>, std::greater<>> monsterEvents_;
  std::vector<WorldFloor> floorData_;
  GameTime currentTime_;
  MonsterIdGenerator mIdGenerator_;
  MonsterId player_;
  std::unique_ptr<EventViewer> eventViewer_;
};

GameState::GameState() noexcept {
  constexpr int DungeonWidth = 90;
  constexpr int DungeonHeight = 30;
  floorData_.push_back(createDungeon(DungeonWidth, DungeonHeight));
  auto tryPlaceMonster = [this](Position pos, MonsterClass mClass, bool isPlayer = false) {
    const auto cFloor = FloorSpecifier(0);
    while (true) {
      Location cLoc(pos, cFloor);
      if (getTerrainType(cLoc) != TerrainType::Empty || !getMonster(cLoc).isNull()) {
        pos.x++;
        if(pos.x==DungeonWidth){
          pos.x=0;
          pos.y++;
        }
      } else {
        break;
      }
    }
    return Monster::createMonster(*this, {pos, cFloor}, mClass, isPlayer);
  };
  player_ = tryPlaceMonster({0,0},MonsterClass::Human, true);
  tryPlaceMonster({0,2}, MonsterClass::SeaSlug);
  tryPlaceMonster({2,4}, MonsterClass::GreedyWeasel);
  tryPlaceMonster({4,4}, MonsterClass::Bryozoan);
  WorldFloor &startingFloor = floorData_[0];
  startingFloor.getObjects({1, 0}).addObject(std::make_unique<Object>(1, ObjectType::KingsCoin, Material::Gold));
  startingFloor.getObjects({4, 2}).addObject(std::make_unique<Object>(1, ObjectType::KingsCoin, Material::Gold));
  startingFloor.getObjects({1, 0}).addObject(std::make_unique<Object>(1, ObjectType::Knife, Material::Iron));
  startingFloor.getObjects({1, 0}).addObject(std::make_unique<Object>(1, ObjectType::Knife, Material::Gold));
}

[[nodiscard]] constexpr bool operator==(WorldFloor::EventListenersIterable::EventListenerSentinal end, WorldFloor::EventListenersIterable::EventListenerIterator iter) noexcept {
  return iter == end;
}
constexpr WorldFloor::EventListenersIterable::EventListenerIterator &WorldFloor::EventListenersIterable::EventListenerIterator::operator++() noexcept {
  ++pos_;
  while (pos_ != arr_->size() && !state_->containsMonster((*arr_)[pos_])) {
    (*arr_)[pos_] = arr_->back();
    arr_->pop_back();
  }
  return *this;
}
[[nodiscard]] constexpr Monster &WorldFloor::EventListenersIterable::EventListenerIterator::operator*() const noexcept {
  return state_->getMonster((*arr_)[pos_]);
}

constexpr void GameState::broadcastEvent(FloorSpecifier floor, auto&& func) noexcept {
  auto ItemPickers = getFloor(floor).getEventListeners(*this);
  std::ranges::for_each(ItemPickers, func);
}

constexpr void GameState::broadcastItemPickup(const Monster &monster, const Object &object) noexcept {
  broadcastEvent(monster.getLoc().mapPos,[this, &monster, &object](Monster &viewer) {
    viewer.informItemPickup(*this, monster, object);
  });
}

constexpr void GameState::broadcastMonsterAttack(HitReturn hitInfo, const Monster &attacker, Monster &attacked) noexcept {
  auto inform = [this, hitInfo, &attacker, &attacked](Monster &viewer) {
    viewer.informMonsterAttack(*this,hitInfo, attacker, attacked);
  };
  if(attacked.isAlive() && !attacked.caresEvent())
    inform(attacked);
  broadcastEvent(attacker.getLoc().mapPos,inform);
}

Dir monsterPath(const GameState& state, const Monster& start, Location end){
  struct WorldFloorWrapper {
    const WorldFloor &floor;
    [[nodiscard]] int extent(int n) const noexcept {
      switch (n) {
      case 0:
        return floor.rows();
      case 1:
        return floor.cols();
      default:
        std::unreachable();
      }
    }
    [[nodiscard]] bool operator[](int row, int col) const noexcept {
      auto tile = floor.getTile(Position{col,row});
      return tile.monster.isNull() && tile.objects.empty() && tile.terrainType==TerrainType::Empty;
    }
  };
  auto [cPos,floor] = start.getLoc();
  if(end.mapPos!=floor){ // at some point add ability to travel to different floor
    return Dir{0,0};
  }
  if(Position::chessboard(cPos,end.pos)<=1){
    return end.pos-cPos;
  }
  return FindPath::findPath(WorldFloorWrapper(state.getFloor(floor)),cPos,end.pos);
}

constexpr bool Monster::isOpenMove(GameState &state, Dir d) const noexcept {
  const WorldFloor &cfloor = state.getFloor(loc_.mapPos);
  const Position nPos = loc_.pos + d;
  if (!cfloor.inBounds(nPos)) {
    return false;
  }
  auto [_, destMonsterId, destTerrainType] = cfloor.getTile(nPos);
  if (destTerrainType != TerrainType::Empty) {
    return false;
  }
  if (!destMonsterId.isNull()) {
    return false;
  }
  return true;
}
TimePeriod Monster::goToTarget(GameState &state, MonsterId targetId) noexcept {
  auto mMonster = state.tryGetMonster(targetId);
  if (!mMonster) {
    return reThink();
  }
  Monster &target = *mMonster;
  return pathTo(state, target.getLoc(), true);
}
TimePeriod Monster::pathTo(GameState &state, Location target, bool attack) noexcept {
  auto movePlan = monsterPath(state,*this,target);
  if(movePlan==Dir{0,0}){
    return reThink();
  }
  const Position tPos = target.pos;
  const Position cPos = getLoc().pos;
  const Position gPos = cPos+movePlan;
  if (gPos == tPos) {
    const TimePeriod tTaken = generalMove(state, movePlan, attack ? MoveMode::fight() : MoveMode::move());
    if (tTaken.future()) {
      return tTaken;
    }
    if (!attack) {
      return reThink();
    }
    state.printDebug("Attack attempted but no time taken. This should not be possible.");
    state.printDebug("If it is possible logic should probably be reworked.");
    return reThink();
  }
  const TimePeriod tTaken = generalMove(state, movePlan, MoveMode::move());
  if (tTaken.future()) {
    return tTaken;
  }
  return reThink();
}
constexpr TimePeriod Monster::takeItem(GameState &state, ObjectContainer &container, std::size_t index) noexcept {
  static constexpr std::size_t PickUpItemSpeedFraction = 10;
  state.broadcastItemPickup(*this, container[index]);
  inventory_.addObject(container.remove(index));
  return getSpeed() / PickUpItemSpeedFraction;
}
constexpr TimePeriod Monster::generalMove(GameState &state, Dir d, MoveMode mode) noexcept {
  Position nPos = loc_.pos + d;
  WorldFloor &cfloor = state.getFloor(loc_.mapPos);
  if (!cfloor.inBounds(nPos)) {
    return TimePeriod(0);
  }
  auto [_, destMonster, destTerrainType] = cfloor.getTile(nPos);
  if (destTerrainType != TerrainType::Empty) {
    return TimePeriod(0);
  }
  if (destMonster.isNull() && mode.isMove()) {
    MonsterId &currentSpot = cfloor.getMonster(loc_.pos);
    destMonster = currentSpot;
    currentSpot.clear();
    loc_.pos = nPos;
    return getSpeed();
  }
  if (!destMonster.isNull() && mode.isFight()) {
    return hitMonster(state, state.getMonster(destMonster));
  }
  return TimePeriod{0};
}
constexpr void Monster::informItemPickup(GameState &state, const Monster &grabber, const Object &grabbed) noexcept {
  if (isPlayer()) {
    state.printItemPickup(grabber, grabbed);
    return;
  }
  if (brain_.hatesItemPickup()) {
    target_ = grabber.getId();
  }
}
constexpr void Monster::informMonsterAttack(GameState &state, HitReturn hitinfo, const Monster &attacker, const Monster &attacked) noexcept {
  if (isPlayer()) {
    state.printMonsterAttack(hitinfo,attacker, attacked);
    return;
  }
  if(attacked == *this){
    target_ = attacker.getId();
  }
}

TimePeriod Monster::runAI(GameState &state) noexcept {
  if (!isAlive()) {
    return TimePeriod(0);
  }
  return std::visit([&state, this](auto target) { return goToTarget(state, target); }, target_);
}
constexpr TimePeriod Monster::dropItem(GameState &state, std::size_t i) noexcept {
  static constexpr std::size_t DropItemSpeedFraction = 10;
  auto obj = removeFromInvent(i);
  state.getObjects(getLoc()).addObject(std::move(obj));
  return getSpeed() / DropItemSpeedFraction;
}
constexpr TimePeriod Monster::dropEverthing(GameState &state) noexcept {
  TimePeriod totalTime(0);
  while(!inventory_.empty()){
    totalTime+=dropItem(state,inventory_.size()-1);
  }
  return totalTime;
}


TimePeriod Monster::hitMonster(GameState &state, Monster &target) noexcept {
  const auto ret=target.hitBy(state, {damage_()});
  state.broadcastMonsterAttack(ret,*this,target);
  return getSpeed();
}
MonsterId Monster::createMonster(GameState &game, Location loc, MonsterClass mClass, bool isPlayer) noexcept {
  auto &monsterDest = game.getMonster(loc);
  if (!monsterDest.isNull()) {
    return MonsterId::null();
  }
  const auto &mInfo = MonsterClassInfoArr[mClass];
  MonsterId id = game.nextMonsterId();
  MonsterBody body{mInfo.speed, mInfo.maxHealth, mInfo.damage, mClass};
  MonsterBrain brain = isPlayer ? PlayerBrain : mInfo.brain;
  auto& mstr = game.insertMonster(std::make_unique<Monster>(CreateKey{},body, loc, id, brain));
  mstr.inventory_.addObject(std::make_unique<Object>(1,ObjectType::Die,Material::Plastic));
  if (mstr.caresEvent()) {
    game.getFloor(loc.mapPos).addEventListener(id);
  }
  monsterDest = id;
  if (!isPlayer && mstr.getSpeed().future()) {
    game.addMonsterEvent(TimePeriod(0), id);
  }
  return id;
}
constexpr void Monster::kill(GameState &state) noexcept {
  setDead();
  state.getMonster(loc_).clear();
  if(isPlayer()){
    state.playerDied();
    return;
  }
  (void) dropEverthing(state);
  state.removeMonster(getId());
}
HitReturn Monster::hitBy(GameState &state, AttackInfo info) noexcept {
  health_ -= info.damage;
  if (health_ > 0) {
    return {false};
  }
  kill(state);
  return {true};
}

