export module GameInterface;
export import GameTypes;
import Common;

class Object;

export class ObjectInterface {
public:
  explicit ObjectInterface(const Object &obj) noexcept;
  [[nodiscard]] int count() const noexcept;
  [[nodiscard]] ObjectType type() const noexcept;
  [[nodiscard]] Material mat() const noexcept;
  [[nodiscard]] ArtifactId artifactStatus() const noexcept;

private:
  const Object *obj_;
};

class ObjectContainer;
export class ObjectContainerInterface {
public:
  using iterator = IteratorImpl<ObjectInterface,ObjectContainerInterface,ObjectInterface>;
  using const_iterator = iterator;
  explicit ObjectContainerInterface(const ObjectContainer &container) noexcept;
  [[nodiscard]] iterator begin() const noexcept;
  [[nodiscard]] iterator end() const noexcept;
  [[nodiscard]] std::size_t size() const noexcept;
  [[nodiscard]] bool empty() const noexcept;
  [[nodiscard]] ObjectInterface front() const noexcept;
  [[nodiscard]] ObjectInterface back() const noexcept;
  [[nodiscard]] ObjectInterface operator[](std::size_t i) const noexcept;
private:
  const ObjectContainer *container_;
};

class Monster;
export class MonsterInterface {
public:
  [[nodiscard]] explicit MonsterInterface(const Monster &monster) noexcept;
  [[nodiscard]] explicit MonsterInterface(const Monster *monster) noexcept;
  [[nodiscard]] MonsterClass getClass() const noexcept;
  [[nodiscard]] bool isPlayer() const noexcept;
  [[nodiscard]] bool isAlive() const noexcept;
  [[nodiscard]] Location getLoc() const noexcept;
  [[nodiscard]] Health getHealth() const noexcept;
  [[nodiscard]] ObjectContainerInterface viewInventory() const noexcept;
  [[nodiscard]] bool isNull() const noexcept;
private:
  const Monster *monster_;
};

export class WorldTileInterface {
public:
  ObjectContainerInterface objects;
  MonsterInterface monster;
  TerrainType terrainType;
};

class GameState;
class WorldFloor;
export class WorldFloorInterface {
public:
  WorldFloorInterface(const GameState& gameState, const WorldFloor &floor) noexcept;
  [[nodiscard]] WorldTileInterface getTile(Position pos) const noexcept;
  [[nodiscard]] std::size_t rows() const noexcept;
  [[nodiscard]] std::size_t cols() const noexcept;
  [[nodiscard]] bool inBounds(Position pos) const;

private:
  const GameState *gameState_;
  const WorldFloor *floor_;
};

export class EventViewerInteface {
public:
  struct HitInfo{
    bool killed;
  };
  virtual void itemPickup(MonsterInterface grabber, ObjectInterface grabbed) = 0;
  virtual void debug(std::string_view message) = 0;
  virtual void monsterAttack(HitInfo hitinfo, MonsterInterface attacker, MonsterInterface attacked) = 0;
  virtual void exception(const std::exception& e) noexcept = 0;
  virtual ~EventViewerInteface(){};
};

export class GameInterface {
public:
  explicit GameInterface(std::unique_ptr<EventViewerInteface> viewer) noexcept;
  void setEventViewer(std::unique_ptr<EventViewerInteface> viewer) noexcept;
  void exit() noexcept;
  void generalMove(Dir d, MoveMode mode) noexcept;
  void pickUpItem(std::size_t selected) noexcept;
  [[nodiscard]] WorldFloorInterface getFloor(FloorSpecifier floorId) const noexcept;
  [[nodiscard]] ObjectContainerInterface lookAtFloor() const noexcept;
  [[nodiscard]] ObjectContainerInterface lookAtInventory() const noexcept;
  [[nodiscard]] Location getLocation() const noexcept;
  [[nodiscard]] Health getHealth() const noexcept;
  [[nodiscard]] GameTime getTime() const noexcept;
  [[nodiscard]] TimePeriod getSpeed() const noexcept;
  void dropItem(std::size_t i) noexcept;
  void throwItem(std::size_t i, Dir dir) noexcept;
  void passTime(TimePeriod numTurns) noexcept;
  ~GameInterface();
  
private:
  std::unique_ptr<GameState> gs_;
};