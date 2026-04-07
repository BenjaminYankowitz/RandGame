module;
#include <cassert>
export module CursesIO;
import CursesLowLevel;
import Common;
import GameInterface;
import Printing;

using namespace std::string_view_literals;
using namespace CursesLowLevel;
namespace {
constexpr Dir keyToDir(chtype key) noexcept {
  switch (key) {
  case SpecialChar::Left:
  case 'H':
  case 'h':
    return {-1, 0};
  case SpecialChar::Down:
  case 'J':
  case 'j':
    return {0, 1};
  case SpecialChar::Up:
  case 'K':
  case 'k':
    return {0, -1};
  case SpecialChar::Right:
  case 'L':
  case 'l':
    return {1, 0};
  case 'Y':
  case 'y':
    return {-1, -1};
  case 'U':
  case 'u':
    return {1, -1};
  case 'B':
  case 'b':
    return {-1, 1};
  case 'N':
  case 'n':
    return {1, 1};
  default:
    return {};
  }
}
} // namespace
std::array InventLettersArr = std::to_array({std::views::iota('a', 'z' + 1), std::views::iota('A', 'Z' + 1)});
constexpr auto InventLetters = std::views::join(InventLettersArr);

auto firstNInvent(std::size_t n) {
  return std::views::zip(std::views::iota(static_cast<std::size_t>(0), n), InventLetters);
}

// static BoxedWindow *EventWindow;
namespace {
void displayInvent(BoxedWindow &window, ObjectContainerInterface items, int sY = 0) {
  window.clear();
  for (auto [y, c] : firstNInvent(std::min<int>(window.prntHeight(), items.size()))) {
    window.moveCursor(0, sY + y);
    window << c << " - " << items[y];
  }
}

void displayEvents(BoxedWindow &window, const std::span<std::pair<GameTime, std::string>> arr) {
  window.clear();
  const std::size_t printHeight = window.prntHeight();
  const std::size_t offSet = arr.size() < printHeight ? 0 : arr.size() - printHeight;
  for (auto i : std::views::iota(static_cast<std::size_t>(0), std::min(arr.size(), printHeight))) {
    window.moveCursor(0, i);
    window << arr[i + offSet].first.impl << ": " << arr[i + offSet].second;
  }
}

} // namespace
class ActionMod {
public:
  [[nodiscard]] constexpr MoveMode getMoveMode() noexcept {
    return moveMode_;
  }
  [[nodiscard]] constexpr std::size_t getCount(std::size_t defaultV = 1) const noexcept {
    if (count_ == NoCount) {
      return defaultV;
    }
    return count_;
  }
  constexpr std::size_t addDigit(int n) noexcept {
    changeDigitLast_ = true;
    if (count_ == NoCount) {
      return count_ = n;
    }
    return count_ = (count_ * 10) + n;
  }
  constexpr void toggleMoveMode(MoveMode mode) noexcept {
    moveMode_ ^= mode;
  }
  [[nodiscard]] constexpr bool betweenRounds() noexcept {
    if (!changeDigitLast_) {
      count_ = NoCount;
    }
    changeDigitLast_ = false;
    beenHit_ = false;
    return continuePlaying_;
  }
  constexpr void quitGame() noexcept {
    continuePlaying_ = false;
  }
  [[nodiscard]] bool interuptAction() const noexcept {
    return beenHit_ || CursesRAII::tryGetChar().has_value();
  }
  constexpr void setBeenHit() noexcept {
    beenHit_ = true;
  }

private:
  static constexpr std::size_t NoCount = std::numeric_limits<std::size_t>::max();
  static constexpr MoveMode DefaultMoveMode = MoveMode::Move | MoveMode::Fight;
  MoveMode moveMode_ = DefaultMoveMode;
  bool changeDigitLast_ = false;
  bool continuePlaying_ = true;
  bool beenHit_ = false;
  std::size_t count_ = NoCount;
};
namespace IOModule {
export class Interface;
}
using IOModule::Interface;
class PrintToViewer : public std::basic_streambuf<char> {
public:
  explicit PrintToViewer(Interface *parent) noexcept : parent_(parent) {}
  Interface *parent_;

protected:
  std::streamsize xsputn(const char_type *s, std::streamsize count) final;

private:
  std::string buffer_;
};
class CursesEventViewer final : public EventViewerInterface {
public:
  explicit CursesEventViewer(Interface *parent) noexcept : viewer_(parent), printWith_(&viewer_) {}
  void itemPickup(MonsterInterface grabber, ObjectInterface grabed) final;
  void monsterHitMonster(HitInfo hitinfo, MonsterInterface attacker, MonsterInterface attacked) final;
  void monsterHitWall(MonsterInterface attacker, TerrainType attacked) final;
  void monsterAte(MonsterInterface eater, ObjectInterface eaten) final;
  void debug(std::string_view message) final;
  void exception(const std::exception &exception) noexcept final;

private:
  PrintToViewer viewer_;
  std::ostream printWith_;
};

namespace Actions {
using ActionType = void (*)(GameInterface &, IOModule::Interface &, ActionMod &);
namespace {
[[nodiscard]] constexpr ActionType getActionFromInput(std::int16_t input) noexcept;
}
} // namespace Actions
export namespace IOModule {
class Interface {
public:
  explicit Interface(std::unique_ptr<GameInterface> interface) : gState_(std::move(interface)), debugViewer_(this), interfaceViewer_(this), interfaceStream_(&interfaceViewer_) {
    eventWindow_ = BoxedWindow(0, 0, 0, 0);
    mainWindow_ = BoxedWindow(0, 0, 0, 0);
    inventWindow_ = BoxedWindow(0, 0, 0, 0);
    statusWindow_ = BoxedWindow(0, 0, 0, 0);
    oldBuffer_ = Logging::log.rdbuf(&debugViewer_);
    gState_->setEventViewer(std::make_unique<CursesEventViewer>(this));
  }
  ~Interface() {
    Logging::log.rdbuf(oldBuffer_);
  }
  void updateGameScreen() {
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
    statusWindow_ << "Health: "sv << gState_->getHealth() << "/"sv << gState_->getMaxHealth();
    statusWindow_.updateScreen();
    displayEvents(eventWindow_, eventLog_);
    eventWindow_.updateScreen();
  }
  void showSuggestion(std::string_view suggestion) {
    for (char c : suggestion) {
      Symbol s(c);
      s.setFrontColor(Grey);
      eventWindow_.place(s);
    }
    eventWindow_.updateScreen();
  }
  int eventWindowWidth() const noexcept {
    return eventWindow_.prntWidth();
  }
  [[nodiscard]] bool doAction() {
    chtype userInput = CursesRAII::getChar();
    const auto func = Actions::getActionFromInput(userInput);

    if (func == nullptr) {
      return true;
    }
    func(*gState_, *this, mod_);
    return mod_.betweenRounds();
  }
  std::string &addEvent(std::string str) noexcept {
    eventLog_.emplace_back(getTime(), std::move(str));
    return eventLog_.back().second;
  }
  [[nodiscard]] std::ostream &interfacePrinter() {
    return interfaceStream_;
  }
  [[nodiscard]] GameTime getTime() const noexcept {
    if (gState_)
      return gState_->getTime();
    return {};
  }
  [[nodiscard]] const StaticPositionArr<TerrainTypeInterface> &getMemory(FloorSpecifier floor) {
    auto currentMap = gState_->getFloor(floor);
    return getMemoryGrid(floor, currentMap.cols(), currentMap.rows());
  }
  bool showSelection(Position pos) {
    Raii_.setCursorState(1);
    if (mainWindow_.inBounds(pos.x, pos.y)) {
      mainWindow_.moveCursor(pos.x, pos.y);
      mainWindow_.updateScreen();
      return true;
    }
    return false;
  }
  constexpr void alertBeenHit() noexcept {
    mod_.setBeenHit();
  }

private:
  [[no_unique_address]] CursesRAII Raii_;
  BoxedWindow mainWindow_;
  BoxedWindow inventWindow_;
  BoxedWindow statusWindow_;
  BoxedWindow eventWindow_;
  ActionMod mod_;
  std::vector<std::pair<GameTime, std::string>> eventLog_;
  std::unique_ptr<GameInterface> gState_;
  std::unordered_map<int, StaticPositionArr<TerrainTypeInterface>> terrainMemory_;
  std::basic_streambuf<char> *oldBuffer_;
  PrintToViewer debugViewer_;
  PrintToViewer interfaceViewer_;
  std::ostream interfaceStream_;
  StaticPositionArr<TerrainTypeInterface> &getMemoryGrid(FloorSpecifier floor, int width, int height) {
    auto [it, inserted] = terrainMemory_.try_emplace(floor.floor, width, height);
    return it->second;
  }
};
} // namespace IOModule

namespace Actions {

namespace {
template <int Dx, int Dy>
void movePlayer(GameInterface &gState, IOModule::Interface & /*unused*/, ActionMod &modifer) noexcept {
  static_assert(Dx <= 1 && Dy <= 1 && Dx >= -1 && Dy >= -1 && (Dx != Dy || Dx != 0));
  constexpr static Dir D = Dir(Dx, Dy);
  gState.generalMove(D, modifer.getMoveMode());
}

void goUpStair(GameInterface &gState, IOModule::Interface & /*unused*/, ActionMod &modifer) noexcept {
  gState.goUpStair(modifer.getMoveMode());
}

void goDownStair(GameInterface &gState, IOModule::Interface & /*unused*/, ActionMod &modifer) noexcept {
  gState.goDownStair(modifer.getMoveMode());
}
template <class GetTerrain>
struct TerrainWrapper {
  GetTerrain getTerrain_;
  int height_;
  int width_;
  [[nodiscard]] int extent(int n) const noexcept {
    switch (n) {
    case 0:
      return height_;
    case 1:
      return width_;
    default:
      std::unreachable();
    }
  }
  [[nodiscard]] bool operator[](int row, int col) const noexcept {
    auto terrain = getTerrain_(Position{col, row});
    return terrain != TerrainTypeInterface::Unknown && !isWall(terrain);
  }
};

std::vector<Position> findUnexploredFrontier(const StaticPositionArr<TerrainTypeInterface> &memory, Position start) {
  std::vector<Position> frontier;
  StaticPositionArr<bool> visited(memory.width(), memory.height());
  std::queue<Position> queue;
  StaticPositionArr<bool> frontierAdded(memory.width(), memory.height());
  visited[start] = true;
  queue.push(start);
  while (!queue.empty()) {
    Position cur = queue.front();
    queue.pop();
    for (Dir dir : Dir::boxDirs()) {
      Position next = cur + dir;
      if (!memory.inBounds(next))
        continue;
      auto terrain = memory[next];
      if (terrain == TerrainTypeInterface::Unknown) {
        if (!frontierAdded[next]) {
          frontierAdded[next] = true;
          frontier.push_back(next);
        }
        continue;
      }
      if (!visited[next] && !isWall(terrain)) {
        visited[next] = true;
        queue.push(next);
      }
    }
  }
  if (frontier.empty())
    frontier.push_back(start);
  return frontier;
}

bool askYesNo(IOModule::Interface &iterface, std::string_view question) noexcept {
  std::ostream &out = iterface.interfacePrinter();
  out << question << " (y/n)\n";
  iterface.updateGameScreen();
  while (true) {
    auto input = CursesRAII::getChar();
    if (input == 'y')
      return true;
    if (input == 'n' || input == SpecialChar::Escape)
      return false;
  }
}
struct ItemFromInterfaceSettings {
  bool doStandAloneDisplay = true;
  bool autoSelectOne = false;
  const std::function<bool(ObjectInterface)> &isEligible = [](ObjectInterface /**/) { return true; };
};

constexpr std::size_t NoItem = std::numeric_limits<std::size_t>::max();
constexpr std::size_t NoChoice = std::numeric_limits<std::size_t>::max() - 1;
[[nodiscard]] std::size_t displayItemInterfaceForChoosing(IOModule::Interface &interface, ObjectContainerInterface items, std::string_view prompt, const ItemFromInterfaceSettings &settings) noexcept {
  if (items.empty()) {
    return NoItem;
  }
  if (settings.doStandAloneDisplay) {
    if (settings.autoSelectOne && items.size() == 1) {
      return 0;
    }
    auto [height, width] = getMaxDims();
    height = std::min<int>(height - 2, items.size() + 2);
    constexpr int DesiredWidth = 40;
    width = std::min<int>(width - 3, DesiredWidth);
    auto window = BoxedWindow(width, height, interface.eventWindowWidth() - DesiredWidth, 0);
    displayInvent(window, items, 2);
    window.moveCursor(0, 0);
    window << prompt;
    window.updateScreen();
  } else {
    std::ostream &out = interface.interfacePrinter();
    std::size_t cnt = 0;
    std::size_t item1;
    char letter1;
    auto validItems = firstNInvent(items.size()) | std::views::filter([&settings, &items](auto v) { return settings.isEligible(items[std::get<0>(v)]); });
    for (auto [i, letter] : validItems) {
      cnt++;
      if (cnt == 1) {
        item1 = i;
        letter1 = letter;
        continue;
      }
      if (cnt == 2) {
        out << prompt << " [" << letter1;
      }
      out << letter;
    }
    if (cnt == 0)
      return NoItem;
    if (cnt == 1) {
      if (settings.autoSelectOne) {
        return item1;
      }
      out << prompt << " [" << letter1;
    }
    out << "]\n";
    interface.updateGameScreen();
  }
  return NoChoice;
}
std::size_t getItemFromInterface(IOModule::Interface &interface, ObjectContainerInterface items, std::string_view prompt, const ItemFromInterfaceSettings &settings = {}) noexcept {
  auto ret = displayItemInterfaceForChoosing(interface, items, prompt, settings);
  if (ret != NoChoice) {
    return ret;
  }
  while (true) {
    auto userInput = CursesRAII::getChar();
    if (userInput == SpecialChar::Escape) {
      break;
    }
    std::optional<std::size_t> index;
    if (userInput >= 'a' && userInput <= 'z') {
      index = static_cast<std::size_t>(userInput - 'a');
    } else if (userInput >= 'A' && userInput <= 'Z') {
      index = static_cast<std::size_t>(userInput - 'A') + 26;
    }
    if (index && *index < items.size() && settings.isEligible(items[*index]))
      return *index;
  }
  return NoItem;
}

void toggleMoveMode(GameInterface & /*gState*/, IOModule::Interface & /*unused*/, ActionMod &mod) {
  mod.toggleMoveMode(MoveMode::Move);
}

void toggleFightMode(GameInterface & /*gState*/, IOModule::Interface & /*unused*/, ActionMod &mod) {
  mod.toggleMoveMode(MoveMode::Fight);
}

void selectAndAct(IOModule::Interface &interface, ObjectContainerInterface items, const char *prompt,
                  ItemFromInterfaceSettings settings, auto action) {
  std::size_t index = getItemFromInterface(interface, items, prompt, settings);
  if (index != NoItem)
    action(index);
}

void pickUpItem(GameInterface &gState, IOModule::Interface &interface, ActionMod & /*mod*/) {
  selectAndAct(interface, gState.lookAtFloor(), "What do you want to pick up?", {.autoSelectOne = true},
               [&](std::size_t i) { gState.pickUpItem(i); });
}

std::optional<Position> findTerrain(const StaticPositionArr<TerrainTypeInterface> &memory, Position pos, TerrainTypeInterface target) {
  auto indices = memory.indexIter();
  auto cIndex = memory.flatIndex(pos);
  auto total = memory.size();
  for (auto i : std::views::join(std::array{std::views::iota(cIndex + 1, total), std::views::iota(0, cIndex)})) {
    auto p = indices[i];
    if (memory[p] == target) {
      return p;
    }
  }
  return {};
}

std::optional<Position> chooseTile(GameInterface &gState, IOModule::Interface &iterface,
                                   bool calcFrontier = false) noexcept {
  std::vector<Position> cycleTargets;
  if (calcFrontier) {
    const FloorSpecifier currentFloor = gState.getLocation().mapPos;
    cycleTargets = findUnexploredFrontier(iterface.getMemory(currentFloor), gState.getLocation().pos);
  }
  auto pos = gState.getLocation().pos;
  iterface.showSelection(pos);
  std::size_t cycleIndex = 0;
  while (true) {
    auto cmnd = CursesRAII::getChar();
    if (cmnd == SpecialChar::Escape)
      return {};
    if (cmnd == '.')
      return pos;
    if (cmnd == 'x' && !cycleTargets.empty()) {
      pos = cycleTargets[cycleIndex];
      cycleIndex = (cycleIndex + 1) % cycleTargets.size();
      iterface.showSelection(pos);
      continue;
    }
    if (cmnd == '>' || cmnd == '<') {
      auto target = (cmnd == '>') ? TerrainTypeInterface::DownStair : TerrainTypeInterface::UpStair;
      const auto &memory = iterface.getMemory(gState.getLocation().mapPos);
      auto npos = findTerrain(memory, pos, target);
      if (npos.has_value()) {
        pos = *npos;
        iterface.showSelection(pos);
      }
      continue;
    }
    auto dir = keyToDir(cmnd);
    int step = (cmnd >= 'A' && cmnd <= 'Z') ? 10 : 1;
    auto jump = Dir(dir.dx * step, dir.dy * step);
    auto desired = pos + jump;
    auto floor = gState.getFloor(gState.getLocation().mapPos);
    int maxX = floor.cols() - 1;
    int maxY = floor.rows() - 1;
    desired.x = std::clamp(desired.x, 0, maxX);
    desired.y = std::clamp(desired.y, 0, maxY);
    if (iterface.showSelection(desired)) {
      pos = desired;
    }
  }
}

enum class StepResult : std::uint8_t { Continue,
                                       Stop };

StepResult takePathStep(GameInterface &gState, IOModule::Interface &interface, ActionMod &mod,
                        const auto &mapWrapper, FloorSpecifier currentFloor, Position goal) {
  const Position currentPos = gState.getLocation().pos;
  if (currentPos == goal)
    return StepResult::Stop;
  if (gState.getLocation().mapPos != currentFloor)
    return StepResult::Stop;
  Dir step = FindPath::findPath(mapWrapper, currentPos, goal);
  if (step.noMove())
    return StepResult::Stop;
  gState.generalMove(step, MoveMode::Move);
  interface.updateGameScreen();
  if (mod.interuptAction())
    return StepResult::Stop;
  if (gState.getLocation().pos == currentPos)
    return StepResult::Stop;
  return StepResult::Continue;
}

void pathTo(GameInterface &gState, IOModule::Interface &interface, ActionMod &mod, Position goal, std::size_t maxStepCount = 10000) {
  FloorSpecifier currentFloor = gState.getLocation().mapPos;
  const auto &mem = interface.getMemory(currentFloor);
  auto mapWrapper = TerrainWrapper{[&mem](Position p) { return mem[p]; }, mem.height(), mem.width()};
  for (auto _ : std::views::repeat(std::monostate{}, maxStepCount)) {
    if (takePathStep(gState, interface, mod, mapWrapper, currentFloor, goal) == StepResult::Stop)
      break;
  }
}

void throwItem(GameInterface &gState, IOModule::Interface &interface, ActionMod &mod) noexcept {
  auto inventory = gState.lookAtInventory();
  if (inventory.empty())
    return;
  auto index = getItemFromInterface(interface, inventory, "What do you want to throw?", {.doStandAloneDisplay = false});
  if (index == NoItem)
    return;
  auto target = chooseTile(gState, interface);
  if (!target)
    return;
  gState.throwItem(index, (*target) - gState.getLocation().pos, mod.getCount());
}

void eatItem(GameInterface &gState, IOModule::Interface &iterface, ActionMod & /*mod*/) noexcept {
  bool fromFloor = false;
  ObjectContainerInterface floorItems = gState.lookAtFloor();
  if (std::ranges::any_of(floorItems, [&gState](ObjectInterface item) { return gState.canEat(item); })) {
    fromFloor = askYesNo(iterface, "Do you want to eat from the floor?");
  }
  ObjectContainerInterface items = fromFloor ? gState.lookAtFloor() : gState.lookAtInventory();
  auto index = getItemFromInterface(iterface, items, "What do you want to eat?",
                                    {.doStandAloneDisplay = fromFloor, .isEligible = [&gState](ObjectInterface item) { return gState.canEat(item); }});
  if (index != NoItem)
    gState.eatItem(index, fromFloor);
}

void dropItem(GameInterface &gState, IOModule::Interface &interface, ActionMod & /*mod*/) noexcept {
  selectAndAct(interface, gState.lookAtInventory(), "What do you want to drop?", {.doStandAloneDisplay = false},
               [&](std::size_t i) { gState.dropItem(i); });
}

void passTime(GameInterface &gState, IOModule::Interface & /*unused*/, ActionMod &mod) noexcept {
  gState.passTime(TimePeriod(mod.getCount(gState.getSpeed().impl)));
}

void rest(GameInterface &gState, IOModule::Interface & /*unused*/, ActionMod &mod) noexcept {
  bool wasFull = gState.getHealth() == gState.getMaxHealth();
  for (auto _ : std::views::repeat(std::monostate{}, mod.getCount())) {
    gState.rest();
    if (mod.interuptAction())
      break;
    if (!wasFull && gState.getHealth() == gState.getMaxHealth())
      break;
  }
}

void autoPath(GameInterface &gState, IOModule::Interface &interface, ActionMod &mod) noexcept {
  auto target = chooseTile(gState, interface, true);
  if (!target)
    return;
  const Position goal = *target;
  pathTo(gState, interface, mod, goal);
}

template <int Dx, int Dy>
void runInDir(GameInterface &gState, IOModule::Interface &interface, ActionMod &mod) noexcept {
  static_assert(Dx <= 1 && Dy <= 1 && Dx >= -1 && Dy >= -1 && (Dx != Dy || Dx != 0));
  constexpr static Dir D = Dir(Dx, Dy);
  while (true) {
    const Position posBefore = gState.getLocation().pos;
    gState.generalMove(D, MoveMode::Move);
    interface.updateGameScreen();
    if (gState.getLocation().pos == posBefore)
      break;
    if (mod.interuptAction())
      break;
  }
}

template <int n>
void addDigit(GameInterface & /*gState*/, IOModule::Interface & /*unused*/, ActionMod &mod) {
  static_assert(n >= 0 && n <= 9);
  mod.addDigit(n);
}

void quit(GameInterface &gState, IOModule::Interface & /*unused*/, ActionMod &mod) noexcept {
  gState.exit();
  mod.quitGame();
}

void debugModeOn(GameInterface &gState, IOModule::Interface & /*unused*/, ActionMod & /*mod*/) noexcept {
  gState.enableDebugMode();
}

void debugModeOff(GameInterface &gState, IOModule::Interface & /*unused*/, ActionMod & /*mod*/) noexcept {
  gState.disableDebugMode();
}

void seeAll(GameInterface &gState, IOModule::Interface &interface, ActionMod & /*mod*/) noexcept {
  gState.mapReveal();
  interface.updateGameScreen();
}

void seeAllOff(GameInterface &gState, IOModule::Interface &interface, ActionMod & /*mod*/) noexcept {
  gState.mapHide();
  interface.updateGameScreen();
}

void teleport(GameInterface &gState, IOModule::Interface &interface, ActionMod & /*mod*/) noexcept {
  if(!gState.isDebugMode())
    return;
  auto target = chooseTile(gState, interface, true);
  if (!target)
    return;
  gState.teleport(*target);
  interface.updateGameScreen();
}

void autoExplore(GameInterface &gState, IOModule::Interface &interface, ActionMod &mod) noexcept {
  const FloorSpecifier currentFloor = gState.getLocation().mapPos;
  const auto &mem = interface.getMemory(currentFloor);
  auto mapWrapper = TerrainWrapper{[&mem](Position p) { return mem[p]; }, mem.height(), mem.width()};
  while (true) {
    auto frontier = findUnexploredFrontier(interface.getMemory(currentFloor), gState.getLocation().pos);
    if (takePathStep(gState, interface, mod, mapWrapper, currentFloor, frontier[0]) == StepResult::Stop)
      break;
  }
}

struct ExtendedCommand {
  std::string_view text;
  ActionType command;
  bool debugModeOnly = false;
  bool autoComplete = true;
  [[nodiscard]] constexpr bool operator==(std::string_view other) const noexcept {
    return text == other;
  }
  [[nodiscard]] constexpr bool operator==(const ExtendedCommand &other) const noexcept {
    if (text != other.text) {
      return false;
    }
    if constexpr (InDebug) {
      if (command != other.command || debugModeOnly != other.debugModeOnly || autoComplete != other.autoComplete) {
        Logging::log << "Command: " << text << " does not match: " << (command == other.command) << ',' << (debugModeOnly == other.debugModeOnly) << ',' << (autoComplete == other.autoComplete) << '\n';
      }
    }
    return true;
  }
};

constexpr std::array ExtendedCommands = std::to_array<ExtendedCommand>({
    {"quit", quit},
    {"wait", passTime},
    {"autoexplore", autoExplore},
    {"debug mode", debugModeOn, false, false},
    {"remove debug mode", debugModeOff, true},
    {"see all", seeAll, true},
    {"remove see all", seeAllOff, true},
    {"teleport", teleport, true},
});

auto validCommands(bool debugMode, bool inSuggestion) {
  return std::views::filter(ExtendedCommands, [debugMode, inSuggestion](const ExtendedCommand& cmd) {
    if (cmd.debugModeOnly && !debugMode)
      return false;
    if (inSuggestion && !cmd.autoComplete)
      return false;
    return true;
  });
}

std::string_view findSuggestion(std::string_view typed, bool debugMode) {
  auto possibleExtensions = std::views::filter(validCommands(debugMode, true),[typed](const ExtendedCommand& cmd){
    return cmd.text.starts_with(typed) && cmd.text != typed;
  });
  return std::ranges::distance(possibleExtensions)==1 ? possibleExtensions.front().text.substr(typed.size()) : std::string_view{};
}

void extendedCommand(GameInterface &gState, IOModule::Interface &interface, ActionMod &mod) {
  std::string &name = interface.addEvent("#");
  interface.updateGameScreen();
  const bool debugMode = gState.isDebugMode();
  auto suggest = [&] {
    auto suggestion = findSuggestion(std::string_view(name).substr(1), debugMode);
    if (!suggestion.empty())
      interface.showSuggestion(suggestion);
  };
  suggest();
  while (true) {
    auto ch = CursesRAII::getChar();
    if (ch == '\n')
      break;
    if (ch == SpecialChar::Escape)
      return;
    if (ch == '\t') {
      auto suggestion = findSuggestion(std::string_view(name).substr(1), debugMode);
      if (!suggestion.empty())
        name += suggestion;
    } else if (ch == SpecialChar::Backspace || ch == 127 || ch == '\b') {
      if (name.size() > 1)
        name.pop_back();
    } else if (ch >= ' ' && ch <= '~') {
      name += static_cast<char>(ch);
    }
    interface.updateGameScreen();
    suggest();
  }
  auto choices = validCommands(debugMode,false);
  auto cmd = std::ranges::find_if(choices, [name = name.subview(1)](const ExtendedCommand& cmd){return cmd==name;});
  if(cmd!=choices.end())
    cmd->command(gState,interface,mod);
}

[[nodiscard]] consteval std::uint16_t cntrl(char c) {
  assert(c>='@' && c <= '_');
  return c-64;
}

constexpr auto CmndMpPairs = CompileTimeHashMap::to_Pairing<std::uint16_t, ActionType>({
    {SpecialChar::Left, movePlayer<-1, 0>},
    {'h', movePlayer<-1, 0>},
    {SpecialChar::Down, movePlayer<0, 1>},
    {'j', movePlayer<0, 1>},
    {SpecialChar::Up, movePlayer<0, -1>},
    {'k', movePlayer<0, -1>},
    {SpecialChar::Right, movePlayer<1, 0>},
    {'l', movePlayer<1, 0>},
    {'y', movePlayer<-1, -1>},
    {'u', movePlayer<1, -1>},
    {'b', movePlayer<-1, 1>},
    {'n', movePlayer<1, 1>},

    {SpecialChar::ShiftLeft, runInDir<-1, 0>},
    {'H', runInDir<-1, 0>},
    {SpecialChar::ShiftDown, runInDir<0, 1>},
    {'J', runInDir<0, 1>},
    {SpecialChar::ShiftUp, runInDir<0, -1>},
    {'K', runInDir<0, -1>},
    {SpecialChar::ShiftRight, runInDir<1, 0>},
    {'L', runInDir<1, 0>},
    {'Y', runInDir<-1, -1>},
    {'U', runInDir<1, -1>},
    {'B', runInDir<-1, 1>},
    {'N', runInDir<1, 1>},
    {cntrl('T'),teleport},

    {'<', goUpStair},
    {'>', goDownStair},

    {'F', toggleFightMode},
    {'m', toggleMoveMode},

    {'0', addDigit<0>},
    {'1', addDigit<1>},
    {'2', addDigit<2>},
    {'3', addDigit<3>},
    {'4', addDigit<4>},
    {'5', addDigit<5>},
    {'6', addDigit<6>},
    {'7', addDigit<7>},
    {'8', addDigit<8>},
    {'9', addDigit<9>},

    {'d', dropItem},
    {'e', eatItem},
    {'t', throwItem},
    {',', pickUpItem},
    {'.', rest},

    {'_', autoPath},
    {'#', extendedCommand},
});

[[nodiscard]] constexpr ActionType getActionFromInput(std::int16_t input) noexcept {
  static constexpr auto CmndMp = CompileTimeHashMap::to_Map<CmndMpPairs, 0, nullptr>();
  return CmndMp.get(input);
}
} // namespace
} // namespace Actions

std::streamsize PrintToViewer::xsputn(const char_type *s, std::streamsize count) {
  std::string_view input(s, count);
  std::size_t currentStart = 0;
  while (true) {
    auto nextNewLine = input.find('\n', currentStart);
    if (nextNewLine == std::string_view::npos) {
      buffer_ += input.substr(currentStart);
      break;
    }
    buffer_ += input.substr(currentStart, nextNewLine - currentStart);
    parent_->addEvent(std::move(buffer_));
    buffer_.clear();
    currentStart = nextNewLine + 1;
  }
  return count;
}

void CursesEventViewer::debug(std::string_view message) {
  printWith_ << message << '\n';
}

void CursesEventViewer::exception(const std::exception &exception) noexcept {
  const auto time = viewer_.parent_->getTime().impl;
  std::fstream logfile("log.txt", std::ios_base::out | std::ios_base::app);
  if (!logfile.is_open()) {
    std::cerr << "Unhandeled exception, and log file does not open\n"
              << time << ": " << exception.what() << '\n';
    std::exit(1);
  }
  logfile << time << ": " << exception.what() << '\n';
  if (logfile.bad()) {
    std::cerr << "I/O error while reading - badbit is true\n"
              << exception.what() << '\n';
    std::exit(1);
  } else if (logfile.fail()) {
    std::cerr << "Logical error on i/o operation - failbit is true\n"
              << exception.what() << '\n';
    std::exit(1);
  }
  logfile.sync();
}

void CursesEventViewer::itemPickup(MonsterInterface grabber, ObjectInterface grabed) {
  printWith_ << grabber << " picked up " << grabed << '\n';
}

void CursesEventViewer::monsterHitMonster(HitInfo info, MonsterInterface attacker, MonsterInterface attacked) {
  if (attacked.isPlayer()) {
    viewer_.parent_->alertBeenHit();
  }
  printWith_ << attacker << ' ' << (info.killed ? "killed" : "hit") << ' ' << attacked;
  if (info.damageDone) {
    printWith_ << ' ' << (info.killed ? "by dealing" : "for") << ' ' << *info.damageDone << " damage";
  }
  printWith_ << '\n';
}

void CursesEventViewer::monsterHitWall(MonsterInterface attacker, TerrainType attacked) {
  printWith_ << attacker << " hit " << attacked << '\n';
}

void CursesEventViewer::monsterAte(MonsterInterface eater, ObjectInterface eaten) {
  printWith_ << eater << " ate " << eaten << '\n';
}

// 𐁀