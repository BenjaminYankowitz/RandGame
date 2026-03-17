export module GameInterface;
import Common;
import Game;
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

export class WorldFloorInterface {
public:
  WorldFloorInterface(const GameState &gameState, const WorldFloor &floor) noexcept;
  [[nodiscard]] WorldTileInterface getTile(Position pos) const noexcept;
  [[nodiscard]] std::size_t rows() const noexcept;
  [[nodiscard]] std::size_t cols() const noexcept;
  [[nodiscard]] bool inBounds(Position pos) const;

private:
  const GameState *gameState_;
  const WorldFloor *floor_;
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
  virtual void exception(const std::exception &e) noexcept = 0;
  virtual ~EventViewerInterface() {};
};

export class GameInterface {
public:
  explicit GameInterface(std::unique_ptr<EventViewerInterface> viewer) noexcept;
  void setEventViewer(std::unique_ptr<EventViewerInterface> viewer) noexcept;
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

ObjectInterface::ObjectInterface(const Object &obj) noexcept : obj_(&obj) {}
int ObjectInterface::count() const noexcept { return obj_->count(); }
ObjectType ObjectInterface::type() const noexcept { return obj_->type(); }
Material ObjectInterface::mat() const noexcept { return obj_->mat(); }
ArtifactId ObjectInterface::artifactStatus() const noexcept { return obj_->artifactStatus(); }

MonsterInterface::MonsterInterface(const Monster &monster) noexcept : monster_(&monster) {}
MonsterInterface::MonsterInterface(const Monster *monster) noexcept : monster_(monster) {}
MonsterClass MonsterInterface::getClass() const noexcept { return monster_->getClass(); };
bool MonsterInterface::isPlayer() const noexcept { return monster_->isPlayer(); }
bool MonsterInterface::isAlive() const noexcept { return monster_->isAlive(); }
Location MonsterInterface::getLoc() const noexcept { return monster_->getLoc(); }
Health MonsterInterface::getHealth() const noexcept { return monster_->getHealth(); }
ObjectContainerInterface MonsterInterface::viewInventory() const noexcept { return ObjectContainerInterface(monster_->viewInventory()); }
bool MonsterInterface::isNull() const noexcept { return monster_ == nullptr; }

WorldTileInterface toInterface(const GameState &gameState, ConstWorldTile tile) noexcept {
  MonsterInterface mInter(tile.monster.isNull() ? nullptr : &gameState.getMonster(tile.monster));
  WorldTileInterface ret(ObjectContainerInterface(tile.objects), mInter, tile.terrainType);
  return ret;
}

WorldFloorInterface::WorldFloorInterface(const GameState &gameState, const WorldFloor &floor) noexcept : gameState_(&gameState), floor_(&floor) {}
WorldTileInterface WorldFloorInterface::getTile(Position pos) const noexcept { return toInterface(*gameState_, floor_->getTile(pos)); }
std::size_t WorldFloorInterface::rows() const noexcept { return floor_->rows(); }
std::size_t WorldFloorInterface::cols() const noexcept { return floor_->cols(); }
bool WorldFloorInterface::inBounds(Position pos) const { return floor_->inBounds(pos); }

ObjectContainerInterface::ObjectContainerInterface(const ObjectContainer &container) noexcept : container_(&container) {}
std::size_t ObjectContainerInterface::size() const noexcept { return container_->size(); }
bool ObjectContainerInterface::empty() const noexcept { return container_->empty(); }
ObjectInterface ObjectContainerInterface::front() const noexcept { return ObjectInterface(container_->front()); }
ObjectInterface ObjectContainerInterface::back() const noexcept { return ObjectInterface(container_->back()); }
ObjectInterface ObjectContainerInterface::operator[](std::size_t i) const noexcept { return ObjectInterface((*container_)[i]); }

GameInterface::GameInterface(std::unique_ptr<EventViewerInterface> viewer) noexcept : gs_(std::make_unique<GameState>()) {
  setEventViewer(std::move(viewer));
}
namespace {
class EventViewerTranslator : public EventViewer {
public:
  explicit EventViewerTranslator(std::unique_ptr<EventViewerInterface> impl) noexcept : impl_(std::move(impl)) {}
  void itemPickup(const Monster &grabber, const Object &grabbed) noexcept final {
    try {
      impl_->itemPickup(MonsterInterface(grabber), ObjectInterface(grabbed));
    } catch (const std::exception &e) {
      impl_->exception(e);
    } catch (...) {
    }
  }
  void debug(std::string_view message) noexcept final {
    try {
      impl_->debug(message);
    } catch (const std::exception &e) {
      impl_->exception(e);
    } catch (...) {
    }
  }
  void monsterHitMonster(const Monster::HitReturn &hitreturn, const Monster &attacker, const Monster &attacked) noexcept final {
    try {
      impl_->monsterHitMonster({hitreturn.damageDone, !!hitreturn.killed}, MonsterInterface(attacker), MonsterInterface(attacked));
    } catch (const std::exception &e) {
      impl_->exception(e);
    } catch (...) {
    }
  }
  void monsterHitWall(const Monster &attacker, TerrainType attacked) noexcept final {
    try {
      impl_->monsterHitWall(MonsterInterface(attacker), attacked);
    } catch (const std::exception &e) {
      impl_->exception(e);
    } catch (...) {
    }
  }

private:
  std::unique_ptr<EventViewerInterface> impl_;
};
}  // namespace

void GameInterface::setEventViewer(std::unique_ptr<EventViewerInterface> viewer) noexcept {
  gs_->setEventViewer(std::make_unique<EventViewerTranslator>(std::move(viewer)));
}
void GameInterface::exit() noexcept {}
void GameInterface::generalMove(Dir d, MoveMode mode) noexcept {
  if (!gs_->getPlayer().isAlive()) {
    return;
  }
  passTime(gs_->getPlayer().generalMove(*gs_, capDir(d), mode));
}
void GameInterface::pickUpItem(std::size_t selected) noexcept {
  if (!gs_->getPlayer().isAlive()) {
    return;
  }
  ObjectContainer &floorItems = gs_->getObjects(gs_->getPlayer().getLoc());
  if (selected >= floorItems.size()) {
    return;
  }
  TimePeriod timePassed = gs_->getPlayer().takeItem(*gs_, floorItems, selected);

  passTime(timePassed);
}

WorldFloorInterface GameInterface::getFloor(FloorSpecifier floorId) const noexcept {
  return {*gs_, gs_->getFloor(floorId)};
}

ObjectContainerInterface GameInterface::lookAtFloor() const noexcept {
  return ObjectContainerInterface(gs_->getObjects(gs_->getPlayer().getLoc()));
}

ObjectContainerInterface GameInterface::lookAtInventory() const noexcept {
  return ObjectContainerInterface(gs_->getPlayer().viewInventory());
}

Location GameInterface::getLocation() const noexcept {
  return gs_->getPlayer().getLoc();
}

Health GameInterface::getHealth() const noexcept {
  return gs_->getPlayer().getHealth();
}

GameTime GameInterface::getTime() const noexcept {
  return gs_->getTime();
}

TimePeriod GameInterface::getSpeed() const noexcept {
  return gs_->getPlayer().getSpeed();
}

void GameInterface::dropItem(std::size_t i) noexcept {
  if (!gs_->getPlayer().isAlive()) {
    return;
  }
  const auto &invent = gs_->getPlayer().viewInventory();
  if (i >= invent.size()) {
    return;
  }
  passTime(gs_->getPlayer().dropItem(*gs_, i));
}

void GameInterface::throwItem(std::size_t i, Dir dir) noexcept {
  if (!gs_->getPlayer().isAlive()) {
    return;
  }
  const ObjectContainer &invent = gs_->getPlayer().viewInventory();
  if (i >= invent.size()) {
    return;
  }
  passTime(gs_->getPlayer().throwItem(*gs_, i, dir));
}
void GameInterface::passTime(TimePeriod numTurns) noexcept {
  if (!gs_->getPlayer().isAlive()) {
    return;
  }
  gs_->passTime(numTurns);
}

GameInterface::~GameInterface() {}
