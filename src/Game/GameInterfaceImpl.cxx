module GameInterface;
import Game;
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
WorldTileInterface WorldFloorInterface::getTile(Position pos) const noexcept { return toInterface(*gameState_,floor_->getTile(pos)); }
std::size_t WorldFloorInterface::rows() const noexcept { return floor_->rows(); }
std::size_t WorldFloorInterface::cols() const noexcept { return floor_->cols(); }
bool WorldFloorInterface::inBounds(Position pos) const { return floor_->inBounds(pos); }

ObjectContainerInterface::ObjectContainerInterface(const ObjectContainer &container) noexcept : container_(&container) {}
std::size_t ObjectContainerInterface::size() const noexcept { return container_->size(); }
bool ObjectContainerInterface::empty() const noexcept { return container_->empty(); }
ObjectInterface ObjectContainerInterface::front() const noexcept { return ObjectInterface(container_->front()); }
ObjectInterface ObjectContainerInterface::back() const noexcept { return ObjectInterface(container_->back()); }
ObjectInterface ObjectContainerInterface::operator[](std::size_t i) const noexcept { return ObjectInterface((*container_)[i]); }


GameInterface::GameInterface(std::unique_ptr<EventViewerInteface>  viewer) noexcept : gs_(std::make_unique<GameState>()) {
  setEventViewer(std::move(viewer));
}

class EventViewerTranslator : public EventViewer {
  public:
  explicit EventViewerTranslator(std::unique_ptr<EventViewerInteface> impl) noexcept : impl_(std::move(impl)) {}
  void itemPickup(const Monster& grabber,const  Object& grabbed) noexcept final {
    try {
      impl_->itemPickup(MonsterInterface(grabber),ObjectInterface(grabbed));
    } catch (const std::exception& e){
      impl_->exception(e);
    } catch(...){}
  }
   void debug(std::string_view message) noexcept final {
    try {
      impl_->debug(message);
    } catch (const std::exception& e){
      impl_->exception(e);
    } catch(...){}
  }
  void monsterAttack(HitReturn hitreturn, const Monster& attacker, const Monster& attacked) noexcept final {
    try {
      impl_->monsterAttack({hitreturn.killed},MonsterInterface(attacker),MonsterInterface(attacked));
    } catch (const std::exception& e){
      impl_->exception(e);
    } catch(...){}
  }
  private:
  std::unique_ptr<EventViewerInteface> impl_;
};


void GameInterface::setEventViewer(std::unique_ptr<EventViewerInteface> viewer) noexcept {
  gs_->setEventViewer(std::make_unique<EventViewerTranslator>(std::move(viewer)));
}
void GameInterface::exit() noexcept {}
void GameInterface::generalMove(Dir d, MoveMode mode) noexcept {
  if(!gs_->getPlayer().isAlive()){
    return;
  }
  passTime(gs_->getPlayer().generalMove(*gs_, capDir(d), mode));
}
void GameInterface::pickUpItem(std::size_t selected) noexcept {
  if(!gs_->getPlayer().isAlive()){
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
  if(!gs_->getPlayer().isAlive()){
    return;
  }
  const auto &invent = gs_->getPlayer().viewInventory();
  if (i >= invent.size()) {
    return;
  }
  gs_->passTime(gs_->getPlayer().dropItem(*gs_, i));
}

void GameInterface::throwItem(std::size_t i) noexcept {
  if(!gs_->getPlayer().isAlive()){
    return;
  }
  const ObjectContainer &invent = gs_->getPlayer().viewInventory();
  if (i >= invent.size()) {
    return;
  }
}
void GameInterface::passTime(TimePeriod numTurns) noexcept {
  if(!gs_->getPlayer().isAlive()){
    return;
  }
  gs_->passTime(numTurns);
}

GameInterface::~GameInterface() {}
