module GameInterface;
import Common;
import Game;

class IMonster : public Monster {};
class IWorldFloor : public WorldFloor {};
class IGameState : public GameState {};

ObjectInterface::ObjectInterface(const Object &obj) noexcept : obj_(&obj) {}
int ObjectInterface::count() const noexcept { return obj_->count(); }
ObjectType ObjectInterface::type() const noexcept { return obj_->type(); }
Material ObjectInterface::mat() const noexcept { return obj_->mat(); }
ArtifactId ObjectInterface::artifactStatus() const noexcept { return obj_->artifactStatus(); }

MonsterInterface::MonsterInterface(const IMonster &monster) noexcept : monster_(&monster) {}
MonsterInterface::MonsterInterface(const IMonster *monster) noexcept : monster_(monster) {}
MonsterClass MonsterInterface::getClass() const noexcept { return monster_->getClass(); };
bool MonsterInterface::isPlayer() const noexcept { return monster_->isPlayer(); }
bool MonsterInterface::isAlive() const noexcept { return monster_->isAlive(); }
Location MonsterInterface::getLoc() const noexcept { return monster_->getLoc(); }
Health MonsterInterface::getHealth() const noexcept { return monster_->getHealth(); }
ObjectContainerInterface MonsterInterface::viewInventory() const noexcept { return ObjectContainerInterface(monster_->viewInventory()); }
bool MonsterInterface::isNull() const noexcept { return monster_ == nullptr; }

MonsterInterface toInterface(const Monster &m) noexcept {
  return MonsterInterface(static_cast<const IMonster &>(m));
}
MonsterInterface toInterface(const Monster *m) noexcept {
  return MonsterInterface(static_cast<const IMonster *>(m));
}

WorldTileInterface toInterface(const GameState &gameState, ConstWorldTile tile) noexcept {
  auto mInter = toInterface(tile.monster.isNull() ? nullptr : &gameState.getMonster(tile.monster));
  WorldTileInterface ret(ObjectContainerInterface(tile.objects), mInter, tile.terrainType);
  return ret;
}

WorldFloorInterface::WorldFloorInterface(const IGameState &gameState, const IWorldFloor &floor) noexcept : gameState_(&gameState), floor_(&floor) {}
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

GameInterface::GameInterface(IGameState& gs, MonsterID controlled) noexcept : gs_(&gs), controlled_(controlled) {}
namespace {
class EventViewerTranslator : public EventViewer {
public:
  explicit EventViewerTranslator(std::unique_ptr<EventViewerInterface> impl) noexcept : impl_(std::move(impl)) {}
  void itemPickup(const Monster &grabber, const Object &grabbed) noexcept final {
    try {
      impl_->itemPickup(toInterface(grabber), ObjectInterface(grabbed));
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
      impl_->monsterHitMonster({hitreturn.damageDone, !!hitreturn.killed}, toInterface(attacker), toInterface(attacked));
    } catch (const std::exception &e) {
      impl_->exception(e);
    } catch (...) {
    }
  }
  void monsterHitWall(const Monster &attacker, TerrainType attacked) noexcept final {
    try {
      impl_->monsterHitWall(toInterface(attacker), attacked);
    } catch (const std::exception &e) {
      impl_->exception(e);
    } catch (...) {
    }
  }

private:
  std::unique_ptr<EventViewerInterface> impl_;
};
} // namespace

void GameInterface::setEventViewer(std::unique_ptr<EventViewerInterface> viewer) noexcept {
  gs_->setEventViewer(std::make_unique<EventViewerTranslator>(std::move(viewer)));
}
void GameInterface::exit() noexcept {}
void GameInterface::generalMove(Dir d, MoveMode mode) noexcept {
  auto& self = gs_->getMonster(controlled_);
  if (!self.isAlive()) {
    return;
  }
  passTime(self.generalMove(*gs_, capDir(d), mode));
}
void GameInterface::goUpStair(MoveMode mode) noexcept {
  auto& self = gs_->getMonster(controlled_);
  if (!self.isAlive()) {
    return;
  }
  passTime(self.goUpStair(*gs_, mode));
}
void GameInterface::goDownStair(MoveMode mode) noexcept {
  auto& self = gs_->getMonster(controlled_);
  if (!self.isAlive()) {
    return;
  }
  passTime(self.goDownStair(*gs_, mode));
}
void GameInterface::pickUpItem(std::size_t selected) noexcept {
  auto& self = gs_->getMonster(controlled_);
  if (!self.isAlive()) {
    return;
  }
  ObjectContainer &floorItems = gs_->getObjects(self.getLoc());
  if (selected >= floorItems.size()) {
    return;
  }
  TimePeriod timePassed = self.takeItem(*gs_, floorItems, selected);

  passTime(timePassed);
}

WorldFloorInterface GameInterface::getFloor(FloorSpecifier floorId) const noexcept {
  return {static_cast<const IGameState &>(*gs_), static_cast<const IWorldFloor &>(gs_->getFloor(floorId))};
}

ObjectContainerInterface GameInterface::lookAtFloor() const noexcept {
  return ObjectContainerInterface(gs_->getObjects(gs_->getMonster(controlled_).getLoc()));
}

ObjectContainerInterface GameInterface::lookAtInventory() const noexcept {
  return ObjectContainerInterface(gs_->getMonster(controlled_).viewInventory());
}

Location GameInterface::getLocation() const noexcept {
  return gs_->getMonster(controlled_).getLoc();
}

Health GameInterface::getHealth() const noexcept {
  return gs_->getMonster(controlled_).getHealth();
}

GameTime GameInterface::getTime() const noexcept {
  return gs_->getTime();
}

TimePeriod GameInterface::getSpeed() const noexcept {
  return gs_->getMonster(controlled_).getSpeed();
}

void GameInterface::dropItem(std::size_t i) noexcept {
  auto& self = gs_->getMonster(controlled_);
  if (!self.isAlive()) {
    return;
  }
  const auto &invent = self.viewInventory();
  if (i >= invent.size()) {
    return;
  }
  passTime(self.dropItem(*gs_, i));
}

void GameInterface::eatItem(std::size_t i, bool fromFloor) noexcept {
  auto& self = gs_->getMonster(controlled_);
  if (!self.isAlive()) {
    return;
  }
  const ObjectContainer &container = fromFloor ? gs_->getObjects(self.getLoc()) : self.viewInventory();
  if (i >= container.size()) {
    return;
  }
  if (!self.canEat(container[i])) {
    return;
  }
  passTime(self.eatItem(*gs_, i, fromFloor));
}

bool GameInterface::canEat(ObjectInterface obj) const noexcept {
  return gs_->getMonster(controlled_).canEat(*obj.obj_);
}

void GameInterface::throwItem(std::size_t i, Dir dir) noexcept {
  auto& self = gs_->getMonster(controlled_);
  if (!self.isAlive()) {
    return;
  }
  const ObjectContainer &invent = self.viewInventory();
  if (i >= invent.size()) {
    return;
  }
  passTime(self.throwItem(*gs_, i, dir));
}
void GameInterface::passTime(TimePeriod numTurns) noexcept {
  if (!gs_->getMonster(controlled_).isAlive()) {
    return;
  }
  gs_->passTime(numTurns);
}

GameInterface::~GameInterface() {}
