export module GameInterface;
import Common;
export import GameTypes;

export class ObjectInterface {
public:
  explicit ObjectInterface(const Object &obj) noexcept;
  explicit ObjectInterface(const std::unique_ptr<const Object> &obj) noexcept : ObjectInterface(*obj) {}
  explicit ObjectInterface(const std::unique_ptr<Object> &obj) noexcept : ObjectInterface(*obj) {}
  [[nodiscard]] int count() const noexcept;
  [[nodiscard]] ObjectType type() const noexcept;
  [[nodiscard]] Material mat() const noexcept;
  [[nodiscard]] ArtifactId artifactStatus() const noexcept;

private:
  friend class GameInterface;
  const Object *obj_;
};

export class ObjectContainerInterface {
public:
  using iterator = IteratorWrapper<typename ObjectContainer::const_iterator, ObjectInterface>;
  using const_iterator = iterator;
  explicit ObjectContainerInterface(const ObjectContainer &container) noexcept;
  [[nodiscard]] iterator begin() const noexcept {
    return container_->begin();
  }
  [[nodiscard]] iterator end() const noexcept {
    return container_->end();
  }
  [[nodiscard]] std::size_t size() const noexcept;
  [[nodiscard]] bool empty() const noexcept;
  [[nodiscard]] ObjectInterface front() const noexcept;
  [[nodiscard]] ObjectInterface back() const noexcept;
  [[nodiscard]] ObjectInterface operator[](std::size_t i) const noexcept;

private:
  const ObjectContainer *container_;
};

class IMonster;
export class IGameState;
class IWorldFloor;
export enum class TerrainTypeInterface : std::uint8_t {
  Unknown,
  Empty,
  UpStair,
  DownStair,
  CWall,
  HWall,
  VWall,
  UTWall,
  DTWall,
  LTWall,
  RTWall,
  TWall,
  ULCornerWall,
  URCornerWall,
  DLCornerWall,
  DRCornerWall,
  SWall
};

export [[nodiscard]] bool isWall(TerrainTypeInterface) noexcept;

export class MonsterInterface {
public:
  [[nodiscard]] explicit MonsterInterface(const IGameState *gameState, const IMonster &monster) noexcept;
  [[nodiscard]] explicit MonsterInterface(const IGameState *gameState, const IMonster *monster) noexcept;
  [[nodiscard]] explicit MonsterInterface(std::nullptr_t) noexcept;
  [[nodiscard]] MonsterClass getClass() const noexcept;
  [[nodiscard]] bool isPlayer() const noexcept;
  [[nodiscard]] bool isAlive() const noexcept;
  [[nodiscard]] Location getLoc() const noexcept;
  [[nodiscard]] Health getHealth() const noexcept;
  [[nodiscard]] Health getMaxHealth() const noexcept;
  [[nodiscard]] ObjectContainerInterface viewInventory() const noexcept;
  [[nodiscard]] bool isNull() const noexcept;

private:
  const IMonster *monster_;
  const IGameState *gameState_ = nullptr;
};

export class MonsterListInterface {
public:
  class Iterator {
  public:
    using value_type = MonsterInterface;
    using difference_type = std::ptrdiff_t;
    constexpr Iterator() noexcept = default;
    Iterator(const IGameState *gameState, const IMonster *monster) noexcept : gameState_(gameState), monster_(monster) {}
    [[nodiscard]] MonsterInterface operator*() const noexcept;
    Iterator &operator++() noexcept;
    Iterator operator++(int) noexcept {
      auto tmp = *this;
      ++*this;
      return tmp;
    }
    [[nodiscard]] bool operator==(const Iterator &other) const noexcept { return monster_ == other.monster_; }

  private:
    const IGameState *gameState_ = nullptr;
    const IMonster *monster_ = nullptr;
  };

  MonsterListInterface(const IGameState *gameState, const IMonster *monster) noexcept;
  explicit MonsterListInterface(std::nullptr_t) noexcept;
  [[nodiscard]] Iterator begin() const noexcept;
  [[nodiscard]] static Iterator end() noexcept;
  [[nodiscard]] MonsterInterface topMonster() const noexcept;

private:
  const IGameState *gameState_ = nullptr;
  const IMonster *monster_ = nullptr;
};

export class WorldTileInterface {
public:
  ObjectContainerInterface objects;
  MonsterListInterface monsters;
  TerrainTypeInterface terrainType;
};

export class WorldFloorInterface {
public:
  WorldFloorInterface(const IGameState &gameState, const IWorldFloor &floor, MonsterID controlled, bool mapRevealed = false) noexcept;
  [[nodiscard]] WorldTileInterface getTile(Position pos) const noexcept;
  [[nodiscard]] int rows() const noexcept;
  [[nodiscard]] int cols() const noexcept;
  [[nodiscard]] bool inBounds(Position pos) const;
  [[nodiscard]] std::vector<std::pair<Position, WorldTileInterface>> getVisibleTiles() const noexcept;

private:
  const IGameState *gameState_;
  const IWorldFloor *floor_;
  MonsterID controlled_;
  bool mapRevealed_;
};

export class EventViewerInterface {
public:
  struct HitInfo {
    std::optional<Health> damageDone;
    bool killed;
  };
  virtual void itemPickup(MonsterInterface grabber, ObjectInterface grabbed) = 0;
  virtual void debug(std::string_view message) = 0;
  virtual void monsterHitMonster(HitInfo hitinfo, MonsterInterface attacker, MonsterInterface attacked) = 0;
  virtual void monsterHitWall(MonsterInterface attacker, TerrainType attacked) = 0;
  virtual void monsterAte(MonsterInterface eater, ObjectInterface eaten) = 0;
  virtual void exception(const std::exception &e) noexcept = 0;
  virtual ~EventViewerInterface() = default;
};

export class GameInterface {
public:
  explicit GameInterface(IGameState &gs, MonsterID controlled) noexcept;
  void setEventViewer(std::unique_ptr<EventViewerInterface> viewer) noexcept;
  void exit() noexcept;
  void generalMove(Dir d, MoveMode mode) noexcept;
  void goUpStair(MoveMode mode) noexcept;
  void goDownStair(MoveMode mode) noexcept;
  void rest() noexcept;
  void pickUpItem(std::size_t selected) noexcept;
  [[nodiscard]] WorldFloorInterface getFloor(FloorSpecifier floorId) const noexcept;
  [[nodiscard]] ObjectContainerInterface lookAtFloor() const noexcept;
  [[nodiscard]] ObjectContainerInterface lookAtInventory() const noexcept;
  [[nodiscard]] Location getLocation() const noexcept;
  [[nodiscard]] Health getHealth() const noexcept;
  [[nodiscard]] Health getMaxHealth() const noexcept;
  [[nodiscard]] GameTime getTime() const noexcept;
  [[nodiscard]] TimePeriod getSpeed() const noexcept;
  void dropItem(std::size_t i) noexcept;
  void eatItem(std::size_t i, bool fromFloor) noexcept;
  [[nodiscard]] bool canEat(ObjectInterface obj) const noexcept;
  void throwItem(std::size_t i, Dir dir, int count) noexcept;
  void passTime(TimePeriod numTurns) noexcept;
  [[nodiscard]] bool isGodMode() const noexcept;
  [[nodiscard]] bool wasGodMode() const noexcept;
  void enableGodMode() noexcept;
  void disableGodMode() noexcept;
  void mapReveal() noexcept;
  void mapHide() noexcept;
  [[nodiscard]] bool isMapRevealed() const noexcept;
  void teleport(Position pos) noexcept;
  ~GameInterface();

private:
  template <typename F>
  void ifAlive(F &&f) noexcept;
  IGameState *gs_;
  MonsterID controlled_;
  bool godMode_ = false;
  bool wasGodMode_ = false;
  bool mapRevealed_ = false;
};