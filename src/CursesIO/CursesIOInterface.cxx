module CursesIO;

using namespace std::string_view_literals;
using namespace CursesLowLevel;

IOModule::Interface::Interface(std::unique_ptr<GameInterface> interface) : gState_(std::move(interface)), debugViewer_(this), interfaceViewer_(this), interfaceStream_(&interfaceViewer_) {
  eventWindow_ = BoxedWindow(0, 0, 0, 0);
  mainWindow_ = BoxedWindow(0, 0, 0, 0);
  inventWindow_ = BoxedWindow(0, 0, 0, 0);
  statusWindow_ = BoxedWindow(0, 0, 0, 0);
  oldBuffer_ = Logging::log.rdbuf(&debugViewer_);
  gState_->setEventViewer(std::make_unique<CursesEventViewer>(this));
}

IOModule::Interface::~Interface() {
  Logging::log.rdbuf(oldBuffer_);
}

void IOModule::Interface::updateGameScreen() {
  Raii_.setCursorState(0);
  mainWindow_.clear();
  const auto &currentMap = gState_->getFloor(gState_->getLocation().mapPos);
  const int mapWidth = currentMap.cols();
  const int mapHeight = currentMap.rows();
  auto [height, width] = getMaxDims();
  const int mainWindowHeight = height - 6 - mapHeight;
  eventWindow_.setDims(mapWidth, mainWindowHeight - 2);
  mainWindow_.move(0, mainWindowHeight);
  mainWindow_.setDims(mapWidth, mapHeight);
  inventWindow_.move(mapWidth + 2, 0);
  inventWindow_.setDims(width - mapWidth - 4, height - 2);
  statusWindow_.move(0, height - 4);
  statusWindow_.setDims(mapWidth, 2);
  auto &memory = getMemoryGrid(gState_->getLocation().mapPos, mapWidth, mapHeight);
  for (auto y : std::views::iota(0, mapHeight)) {
    mainWindow_.moveCursor(0, y);
    for (auto x : std::views::iota(0, mapWidth)) {
      mainWindow_ << MemoryTerrainToSymbol(memory[Position{x, y}]);
    }
  }
  for (auto [pos, tile] : currentMap.getVisibleTiles()) {
    memory[pos] = tile.terrainType;
    mainWindow_.moveCursor(pos.x, pos.y);
    mainWindow_ << TileToSymbol(tile);
  }
  ObjectContainerInterface playerInvent = gState_->lookAtInventory();
  displayInvent(inventWindow_, playerInvent);
  inventWindow_.updateScreen();
  mainWindow_.updateScreen();
  statusWindow_.clear();
  statusWindow_.moveCursor(0, 0);
  statusWindow_ << "Health: "sv << gState_->getHealth() << "/"sv << gState_->getMaxHealth() << " MP: "sv << gState_->getMP() << "/"sv << gState_->getMaxMP();
  statusWindow_ << " ["sv;
  MoveMode mode = mod_.getMoveMode();
  if (hasOverlap(mode, MoveMode::Fight))
    statusWindow_ << "F"sv;
  if (hasOverlap(mode, MoveMode::Move))
    statusWindow_ << "M"sv;
  if (hasOverlap(mode, MoveMode::GetWith))
    statusWindow_ << "G"sv;
  statusWindow_ << "]"sv;
  statusWindow_.updateScreen();
  displayEvents(eventWindow_, eventLog_);
  eventWindow_.updateScreen();
}

void IOModule::Interface::showSuggestion(std::string_view suggestion) {
  for (char c : suggestion) {
    Symbol s(c);
    s.setFrontColor(Grey);
    eventWindow_.place(s);
  }
  eventWindow_.updateScreen();
}

bool IOModule::Interface::doAction() {
  chtype userInput = CursesRAII::getChar();
  const auto func = Actions::getActionFromInput(userInput);

  if (func == nullptr) {
    return true;
  }
  func(*gState_, *this, mod_);
  return mod_.betweenRounds();
}

GameTime IOModule::Interface::getTime() const noexcept {
  if (gState_)
    return gState_->getTime();
  return {};
}

const StaticPositionArr<TerrainTypeInterface> &IOModule::Interface::getMemory(FloorSpecifier floor) {
  auto currentMap = gState_->getFloor(floor);
  return getMemoryGrid(floor, currentMap.cols(), currentMap.rows());
}

bool IOModule::Interface::showSelection(Position pos) {
  Raii_.setCursorState(1);
  if (mainWindow_.inBounds(pos.x, pos.y)) {
    mainWindow_.moveCursor(pos.x, pos.y);
    mainWindow_.updateScreen();
    return true;
  }
  return false;
}

void IOModule::Interface::drawBeamAt(Location loc) {
  if(loc.mapPos!=gState_->getLocation().mapPos)
    return;
  if (!mainWindow_.inBounds(loc.pos.x, loc.pos.y))
    return;
  Symbol sym(L'*');
  sym.setFrontColor(Cyan);
  mainWindow_.place(loc.pos.x, loc.pos.y, sym);
  mainWindow_.updateScreen();
  std::this_thread::sleep_for(std::chrono::milliseconds(40));
}

StaticPositionArr<TerrainTypeInterface> &IOModule::Interface::getMemoryGrid(FloorSpecifier floor, int width, int height) {
  auto [it, inserted] = terrainMemory_.try_emplace(floor.floor, width, height);
  return it->second;
}
