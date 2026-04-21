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

class IMonster {
public:
  const void *monster;
  [[nodiscard]] constexpr bool operator==(const IMonster &) const noexcept = default;
};
export class IGameState {
public:
  void *gameState;
};
class IWorldFloor {
public:
  void *worldFloor;
};
export enum class TerrainTypeInterface : std::uint8_t {
  Unknown,
  StoneFloor,
  GrassFloor,
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
  [[nodiscard]] MonsterInterface(IGameState gameState, IMonster monster) noexcept;
  [[nodiscard]] explicit MonsterInterface(std::nullptr_t) noexcept;
  [[nodiscard]] MonsterClass getClass() const noexcept;
  [[nodiscard]] bool isPlayer() const noexcept;
  [[nodiscard]] bool isAlive() const noexcept;
  [[nodiscard]] Location getLoc() const noexcept;
  [[nodiscard]] Health getHealth() const noexcept;
  [[nodiscard]] Health getMaxHealth() const noexcept;
  [[nodiscard]] int getMP() const noexcept;
  [[nodiscard]] int getMaxMP() const noexcept;
  [[nodiscard]] ObjectContainerInterface viewInventory() const noexcept;
  [[nodiscard]] bool isNull() const noexcept;

private:
  IMonster monster_;
  IGameState gameState_;
};

export class MonsterListInterface {
public:
  class Iterator {
  public:
    using value_type = MonsterInterface;
    using difference_type = std::ptrdiff_t;
    constexpr Iterator() noexcept = default;
    Iterator(IGameState gameState, IMonster monster) noexcept : gameState_(gameState), monster_(monster) {}
    [[nodiscard]] MonsterInterface operator*() const noexcept;
    Iterator &operator++() noexcept;
    Iterator operator++(int) noexcept {
      auto tmp = *this;
      ++*this;
      return tmp;
    }
    [[nodiscard]] bool operator==(const Iterator &other) const noexcept { return monster_ == other.monster_; }

  private:
    IGameState gameState_ = {nullptr};
    IMonster monster_ = {nullptr};
  };

  MonsterListInterface(IGameState gameState, IMonster monster) noexcept;
  explicit MonsterListInterface(std::nullptr_t) noexcept;
  [[nodiscard]] Iterator begin() const noexcept;
  [[nodiscard]] static Iterator end() noexcept;
  [[nodiscard]] MonsterInterface topMonster() const noexcept;

private:
  IGameState gameState_ = {nullptr};
  IMonster monster_ = {nullptr};
};

export class WorldTileInterface {
public:
  ObjectContainerInterface objects;
  MonsterListInterface monsters;
  TerrainTypeInterface terrainType;
};

export class WorldFloorInterface {
public:
  WorldFloorInterface(IGameState gameState, IWorldFloor floor, MonsterID controlled, bool mapRevealed = false) noexcept;
  [[nodiscard]] WorldTileInterface getTile(Position pos) const noexcept;
  [[nodiscard]] int rows() const noexcept;
  [[nodiscard]] int cols() const noexcept;
  [[nodiscard]] bool inBounds(Position pos) const;
  [[nodiscard]] std::vector<std::pair<Position, WorldTileInterface>> getVisibleTiles() const noexcept;

private:
  IGameState gameState_;
  IWorldFloor floor_;
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
  virtual void itemEquipped(MonsterInterface wearer, ObjectInterface item) = 0;
  virtual void itemUnequipped(MonsterInterface wearer, ObjectInterface item) = 0;
  virtual void equipSlotsFull(MonsterInterface wearer, ObjectInterface item) = 0;
  virtual void debug(std::string_view message) = 0;
  virtual void monsterHitMonster(HitInfo hitinfo, MonsterInterface attacker, MonsterInterface attacked) = 0;
  virtual void monsterHitWall(MonsterInterface attacker, TerrainTypeInterface attacked) = 0;
  virtual void monsterAte(MonsterInterface eater, ObjectInterface eaten) = 0;
  virtual void beamStep(Location pos) = 0;
  virtual void exception(const std::exception &e) noexcept = 0;
  EventViewerInterface() = default;
  EventViewerInterface(const EventViewerInterface &) = default;
  EventViewerInterface &operator=(const EventViewerInterface &) = default;
  EventViewerInterface(EventViewerInterface &&) = default;
  EventViewerInterface &operator=(EventViewerInterface &&) = default;
  virtual ~EventViewerInterface() = default;
};

export class GameInterface {
public:
  explicit GameInterface(IGameState gs) noexcept;
  void setControlled(MonsterID mId) noexcept { controlled_ = mId; }
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
  [[nodiscard]] int getMP() const noexcept;
  [[nodiscard]] int getMaxMP() const noexcept;
  [[nodiscard]] GameTime getTime() const noexcept;
  [[nodiscard]] TimePeriod getSpeed() const noexcept;
  void dropItem(std::size_t i) noexcept;
  void eatItem(std::size_t i, bool fromFloor) noexcept;
  [[nodiscard]] bool canEat(ObjectInterface obj) const noexcept;
  [[nodiscard]] int getMaxThrowingDistance() const noexcept;
  void throwItem(std::size_t i, Dir dir, int count) noexcept;
  void equipItem(std::size_t inventoryIdx) noexcept;
  void unequipItem(std::int8_t slot) noexcept;
  [[nodiscard]] std::int8_t equipmentSize() const noexcept;
  [[nodiscard]] std::optional<ObjectInterface> viewEquipped(std::int8_t slot) const noexcept;
  void castBeam(Dir dir) noexcept;
  void passTime(TimePeriod numTurns) noexcept;
  [[nodiscard]] bool isDebugMode() const noexcept;
  [[nodiscard]] bool wasDebugMode() const noexcept;
  void enableDebugMode() noexcept;
  void disableDebugMode() noexcept;
  void mapReveal() noexcept;
  void mapHide() noexcept;
  [[nodiscard]] bool isMapRevealed() const noexcept;
  void teleport(Position pos) noexcept;
  void setPlayerImmortal() noexcept;
  void setPlayerMortal() noexcept;
  struct LoadResult {
    enum class Error : std::uint8_t { None,
                                      BadMagic,
                                      VersionMismatch,
                                      Exception };
    Error error = Error::None;
    std::size_t bytesRead = 0;
    int fileVersion = 0;
    int expectedVersion = 0;
    std::string message;
    [[nodiscard]] bool ok() const noexcept { return error == Error::None; }
    static LoadResult success(std::size_t bytes) noexcept { return {Error::None, bytes, SaveVersion, SaveVersion}; }
    static LoadResult badMagic() noexcept { return {Error::BadMagic}; }
    static LoadResult versionMismatch(int file, int expected) noexcept { return {Error::VersionMismatch, 0, file, expected}; }
    static LoadResult exception(std::string what) noexcept { return {Error::Exception, 0, 0, 0, std::move(what)}; }
  };

  std::size_t save(std::ostream &out) const noexcept;
  LoadResult load(std::istream &in) noexcept;

private:
  static constexpr int SaveVersion = 1;
  static constexpr std::uint64_t MagicNumber = 8360033890706637073;
  template <typename F>
  void ifAlive(const F &f) noexcept;
  IGameState gs_;
  MonsterID controlled_;
  bool debugMode_ = false;
  bool wasDebugMode_ = false;
  bool mapRevealed_ = false;
};