module;
#include "../Common/EnumBitOps.h"
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

MonsterInterface::MonsterInterface(const IGameState *gameState, const IMonster &monster) noexcept : monster_(&monster), gameState_(gameState) {}
MonsterInterface::MonsterInterface(const IGameState *gameState, const IMonster *monster) noexcept : monster_(monster), gameState_(gameState) {}
MonsterInterface::MonsterInterface(std::nullptr_t) noexcept : monster_(nullptr), gameState_(nullptr) {}
MonsterClass MonsterInterface::getClass() const noexcept { return monster_->getClass(); };
bool MonsterInterface::isPlayer() const noexcept { return monster_->isPlayer(); }
bool MonsterInterface::isAlive() const noexcept { return monster_->isAlive(); }
Location MonsterInterface::getLoc() const noexcept { return monster_->getLoc(); }
Health MonsterInterface::getHealth() const noexcept { return monster_->getHealth(); }
Health MonsterInterface::getMaxHealth() const noexcept { return monster_->getMaxHealth(); }
ObjectContainerInterface MonsterInterface::viewInventory() const noexcept { return ObjectContainerInterface(monster_->viewInventory()); }
bool MonsterInterface::isNull() const noexcept { return monster_ == nullptr; }

MonsterListInterface::MonsterListInterface(const IGameState *gameState, const IMonster *monster) noexcept : gameState_(gameState), monster_(monster) {}
MonsterListInterface::MonsterListInterface(std::nullptr_t) noexcept : gameState_(nullptr), monster_(nullptr) {}
MonsterListInterface::Iterator MonsterListInterface::begin() const noexcept { return {gameState_, monster_}; }
MonsterListInterface::Iterator MonsterListInterface::end() noexcept { return {nullptr, nullptr}; }
MonsterInterface MonsterListInterface::topMonster() const noexcept { return MonsterInterface(gameState_, monster_); }

MonsterInterface MonsterListInterface::Iterator::operator*() const noexcept { return MonsterInterface(gameState_, monster_); }
MonsterListInterface::Iterator &MonsterListInterface::Iterator::operator++() noexcept {
  auto nextId = monster_->getNext();
  if (nextId.isNull()) {
    monster_ = nullptr;
  } else {
    monster_ = static_cast<const IMonster *>(&gameState_->getMonster(nextId));
  }
  return *this;
}

[[nodiscard]] MonsterInterface toInterface(const IGameState &gameState, const Monster &m) noexcept {
  return MonsterInterface(&gameState, static_cast<const IMonster &>(m));
}
[[nodiscard]] MonsterInterface toInterface(const IGameState &gameState, const Monster *m) noexcept {
  return MonsterInterface(&gameState, static_cast<const IMonster *>(m));
}
[[nodiscard]] MonsterInterface toInterface(const IGameState &gameState, Monster::ID id) noexcept {
  return id.isNull() ? MonsterInterface(nullptr) : toInterface(gameState, gameState.getMonster(id));
}
[[nodiscard]] MonsterListInterface toMonsterList(const IGameState &gameState, Monster::ID id) noexcept {
  return id.isNull() ? MonsterListInterface(nullptr) : MonsterListInterface(&gameState, static_cast<const IMonster *>(&gameState.getMonster(id)));
}

enum class Directions : std::uint8_t {
  None = 0,
  Up = 1,
  Left = Up << 1,
  Right = Left << 1,
  Down = Right << 1
};

DEFINE_ENUM_BIT_OPS(Directions)
constexpr auto WallType = []() {
  class RetType {
  public:
    [[nodiscard]] constexpr TerrainTypeInterface &operator[](Directions dir) noexcept {
      return impl_[std::to_underlying(dir)];
    }
    [[nodiscard]] constexpr TerrainTypeInterface operator[](Directions dir) const noexcept {
      return impl_[std::to_underlying(dir)];
    }
    [[nodiscard]] constexpr TerrainTypeInterface &operator[](int dir) noexcept {
      return impl_[dir];
    }
    [[nodiscard]] constexpr TerrainTypeInterface operator[](int dir) const noexcept {
      return impl_[dir];
    }

  private:
    std::array<TerrainTypeInterface, (1 << 4)> impl_;
  };
  using enum Directions;
  using enum TerrainTypeInterface;
  RetType ret;
  ret[None] = CWall;
  ret[Up] = VWall;
  ret[Down] = VWall;
  ret[Left] = HWall;
  ret[Right] = HWall;
  ret[Up | Down] = VWall;
  ret[Up | Left] = DRCornerWall;
  ret[Up | Right] = DLCornerWall;
  ret[Down | Left] = URCornerWall;
  ret[Down | Right] = ULCornerWall;
  ret[Left | Right] = HWall;
  ret[Up | Down | Left] = RTWall;
  ret[Up | Down | Right] = LTWall;
  ret[Up | Left | Right] = DTWall;
  ret[Down | Left | Right] = UTWall;
  ret[Up | Down | Left | Right] = TWall;
  return ret;
}();

[[nodiscard]] TerrainTypeInterface toInterface(const WorldFloor &floor, Position pos, TerrainType t) noexcept {
  switch (t) {
  case TerrainType::DownStair:
    return TerrainTypeInterface::DownStair;
  case TerrainType::UpStair:
    return TerrainTypeInterface::UpStair;
  case TerrainType::Empty:
    return TerrainTypeInterface::Empty;
  case TerrainType::Wall:
    auto getType = [&floor, pos](Dir dir) {
      return !floor.inBounds(pos + dir) || floor.getTerrainType(pos + dir) == TerrainType::Wall;
    };
    auto check = [&getType](Dir dir) {
      if (!getType(dir)) {
        return false;
      }
      auto [dx, dy] = dir;
      Dir oDir(dy, dx);
      return !(getType(oDir) && getType(-oDir) && getType(dir + oDir) && getType(dir - oDir));
    };
    int dirs = 0;
    for (auto [i, checkD] : std::views::zip(std::views::iota(0), Dir::directDirs())) {
      if (check(checkD))
        dirs |= (1 << i);
    }
    if (dirs == 0 && getType(Dir::up())) {
      return TerrainTypeInterface::SWall;
    }
    return WallType[dirs];
  }
}

bool isWall(TerrainTypeInterface type) noexcept {
  switch (type) {
    using enum TerrainTypeInterface;
  case TerrainTypeInterface::CWall:
  case TerrainTypeInterface::HWall:
  case TerrainTypeInterface::VWall:
  case TerrainTypeInterface::UTWall:
  case TerrainTypeInterface::DTWall:
  case TerrainTypeInterface::LTWall:
  case TerrainTypeInterface::RTWall:
  case TerrainTypeInterface::TWall:
  case TerrainTypeInterface::ULCornerWall:
  case TerrainTypeInterface::URCornerWall:
  case TerrainTypeInterface::DLCornerWall:
  case TerrainTypeInterface::DRCornerWall:
  case TerrainTypeInterface::SWall:
    return true;
  default:
    return false;
  }
}

WorldFloorInterface::WorldFloorInterface(const IGameState &gameState, const IWorldFloor &floor, MonsterID controlled, bool mapRevealed) noexcept : gameState_(&gameState), floor_(&floor), controlled_(controlled), mapRevealed_(mapRevealed) {}
WorldTileInterface WorldFloorInterface::getTile(Position pos) const noexcept {
  static constexpr ObjectContainer EmptyContainer;
  if (!mapRevealed_) {
    const auto &monster = gameState_->getMonster(controlled_);
    if (!(inBounds(pos) && monster.inLineOfSight(*gameState_, pos)))
      return {ObjectContainerInterface(EmptyContainer), MonsterListInterface(nullptr), TerrainTypeInterface::Unknown};
  } else if (!inBounds(pos)) {
    return {ObjectContainerInterface(EmptyContainer), MonsterListInterface(nullptr), TerrainTypeInterface::Unknown};
  }
  auto tile = floor_->getTile(pos);
  WorldTileInterface ret(ObjectContainerInterface(tile.objects), toMonsterList(*gameState_, tile.monster), toInterface(*floor_, pos, tile.terrainType));
  return ret;
}
int WorldFloorInterface::rows() const noexcept { return floor_->rows(); }
int WorldFloorInterface::cols() const noexcept { return floor_->cols(); }
bool WorldFloorInterface::inBounds(Position pos) const { return floor_->inBounds(pos); }
std::vector<std::pair<Position, WorldTileInterface>> WorldFloorInterface::getVisibleTiles() const noexcept {
  if (mapRevealed_) {
    std::vector<std::pair<Position, WorldTileInterface>> ret;
    ret.reserve(static_cast<std::size_t>(rows()) * static_cast<std::size_t>(cols()));
    for (int r = 0; r < rows(); ++r) {
      for (int c = 0; c < cols(); ++c) {
        Position pos(c, r);
        auto tile = floor_->getTile(pos);
        ret.emplace_back(pos, WorldTileInterface(ObjectContainerInterface(tile.objects), toMonsterList(*gameState_, tile.monster), toInterface(*floor_, pos, tile.terrainType)));
      }
    }
    return ret;
  }
  const auto &monster = gameState_->getMonster(controlled_);
  auto positions = LineOfSight::allInLineOfSight(WorldFloorWrapper<&WorldFloor::seeThrough>(*floor_), monster.getLoc().pos);
  std::vector<std::pair<Position, WorldTileInterface>> ret;
  ret.reserve(positions.size());
  for (auto pos : positions) {
    auto tile = floor_->getTile(pos);
    ret.emplace_back(pos, WorldTileInterface(ObjectContainerInterface(tile.objects), toMonsterList(*gameState_, tile.monster), toInterface(*floor_, pos, tile.terrainType)));
  }
  return ret;
}

ObjectContainerInterface::ObjectContainerInterface(const ObjectContainer &container) noexcept : container_(&container) {}
std::size_t ObjectContainerInterface::size() const noexcept { return container_->size(); }
bool ObjectContainerInterface::empty() const noexcept { return container_->empty(); }
ObjectInterface ObjectContainerInterface::front() const noexcept { return ObjectInterface(container_->front()); }
ObjectInterface ObjectContainerInterface::back() const noexcept { return ObjectInterface(container_->back()); }
ObjectInterface ObjectContainerInterface::operator[](std::size_t i) const noexcept { return ObjectInterface((*container_)[i]); }

GameInterface::GameInterface(IGameState &gs, MonsterID controlled) noexcept : gs_(&gs), controlled_(controlled) {}
namespace {
class EventViewerTranslator : public EventViewer {
public:
  explicit EventViewerTranslator(const IGameState &gameState, std::unique_ptr<EventViewerInterface> impl) noexcept : gameState_(&gameState), impl_(std::move(impl)) {}
  void itemPickup(const Monster &grabber, const Object &grabbed) noexcept final {
    safeCall([&] { impl_->itemPickup(toInterface(*gameState_, grabber), ObjectInterface(grabbed)); });
  }
  void debug(std::string_view message) noexcept final {
    safeCall([&] { impl_->debug(message); });
  }
  void monsterHitMonster(const Monster::HitReturn &hitreturn, const Monster &attacker, const Monster &attacked) noexcept final {
    safeCall([&] { impl_->monsterHitMonster({hitreturn.damageDone, !!hitreturn.killed}, toInterface(*gameState_, attacker), toInterface(*gameState_, attacked)); });
  }
  void monsterHitWall(const Monster &attacker, TerrainType attacked) noexcept final {
    safeCall([&] { impl_->monsterHitWall(toInterface(*gameState_, attacker), attacked); });
  }
  void monsterAte(const Monster &eater, const Object &eaten) noexcept final {
    safeCall([&] { impl_->monsterAte(toInterface(*gameState_, eater), ObjectInterface(eaten)); });
  }

private:
  template <typename F>
  void safeCall(F &&f) noexcept {
    try {
      f();
    } catch (const std::exception &e) {
      impl_->exception(e);
    } catch (...) {
    }
  }
  const IGameState *gameState_;
  std::unique_ptr<EventViewerInterface> impl_;
};
} // namespace

void GameInterface::setEventViewer(std::unique_ptr<EventViewerInterface> viewer) noexcept {
  gs_->setEventViewer(std::make_unique<EventViewerTranslator>(*gs_, std::move(viewer)));
}
void GameInterface::exit() noexcept {}
template <typename F>
void GameInterface::ifAlive(F &&f) noexcept {
  auto &self = gs_->getMonster(controlled_);
  if (self.isAlive()) {
    f(self);
  }
}
void GameInterface::generalMove(Dir d, MoveMode mode) noexcept {
  ifAlive([&](auto &self) { passTime(self.generalMove(*gs_, capDir(d), mode)); });
}
void GameInterface::goUpStair(MoveMode mode) noexcept {
  ifAlive([&](auto &self) { passTime(self.goUpStair(*gs_, mode)); });
}
void GameInterface::rest() noexcept {
  ifAlive([&](auto &self) { passTime(self.rest()); });
}
void GameInterface::goDownStair(MoveMode mode) noexcept {
  ifAlive([&](auto &self) { passTime(self.goDownStair(*gs_, mode)); });
}
void GameInterface::pickUpItem(std::size_t selected) noexcept {
  ifAlive([&](auto &self) {
    ObjectContainer &floorItems = gs_->getObjects(self.getLoc());
    if (selected >= floorItems.size()) {
      return;
    }
    passTime(self.takeItem(*gs_, floorItems, selected));
  });
}

WorldFloorInterface GameInterface::getFloor(FloorSpecifier floorId) const noexcept {
  return {static_cast<const IGameState &>(*gs_), static_cast<const IWorldFloor &>(gs_->getFloor(floorId)), controlled_, mapRevealed_};
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

Health GameInterface::getMaxHealth() const noexcept {
  return gs_->getMonster(controlled_).getMaxHealth();
}

GameTime GameInterface::getTime() const noexcept {
  return gs_->getTime();
}

TimePeriod GameInterface::getSpeed() const noexcept {
  return gs_->getMonster(controlled_).getSpeed();
}

void GameInterface::dropItem(std::size_t i) noexcept {
  ifAlive([&](auto &self) {
    const auto &invent = self.viewInventory();
    if (i >= invent.size()) {
      return;
    }
    passTime(self.dropItem(*gs_, i));
  });
}

void GameInterface::eatItem(std::size_t i, bool fromFloor) noexcept {
  ifAlive([&](auto &self) {
    const ObjectContainer &container = fromFloor ? gs_->getObjects(self.getLoc()) : self.viewInventory();
    if (i >= container.size()) {
      return;
    }
    if (!self.canEat(container[i])) {
      return;
    }
    passTime(self.eatItem(*gs_, i, fromFloor));
  });
}

bool GameInterface::canEat(ObjectInterface obj) const noexcept {
  return gs_->getMonster(controlled_).canEat(*obj.obj_);
}

void GameInterface::throwItem(std::size_t i, Dir dir, int count) noexcept {
  ifAlive([&](auto &self) {
    const ObjectContainer &invent = self.viewInventory();
    if (i >= invent.size()) {
      return;
    }
    passTime(self.throwItem(*gs_, i, dir, count));
  });
}
void GameInterface::passTime(TimePeriod numTurns) noexcept {
  if (!gs_->getMonster(controlled_).isAlive()) {
    return;
  }
  gs_->passTime(numTurns);
}

bool GameInterface::isDebugMode() const noexcept { return debugMode_; }
bool GameInterface::wasDebugMode() const noexcept { return wasDebugMode_; }
void GameInterface::enableDebugMode() noexcept {
  debugMode_ = true;
  wasDebugMode_ = true;
}
void GameInterface::disableDebugMode() noexcept {
  debugMode_ = false;
  mapRevealed_ = false;
}
void GameInterface::mapReveal() noexcept {
  if (debugMode_)
    mapRevealed_ = true;
}
void GameInterface::mapHide() noexcept {
  if (debugMode_)
    mapRevealed_ = false;
}
bool GameInterface::isMapRevealed() const noexcept { return mapRevealed_; }
void GameInterface::teleport(Position pos) noexcept {
  if (!debugMode_)
    return;
  ifAlive([&](auto &self) { passTime(self.generalMove(*gs_, pos, MoveMode::Move)); });
}

GameInterface::~GameInterface() {}
