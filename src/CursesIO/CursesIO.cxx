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

void displayEvents(BoxedWindow &window, const std::vector<std::string> &arr) {
  window.clear();
  const std::size_t printHeight = window.prntHeight();
  const std::size_t offSet = arr.size() < printHeight ? 0 : arr.size() - printHeight;
  for (std::size_t i = 0; i < std::min<std::size_t>(arr.size(), printHeight); i++) {
    window.moveCursor(0, i);
    window << std::string_view(arr[i + offSet]);
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
    return continuePlaying_;
  }
  constexpr void quitGame() noexcept {
    continuePlaying_ = false;
  }

private:
  static constexpr std::size_t NoCount = std::numeric_limits<std::size_t>::max();
  static constexpr MoveMode DefaultMoveMode = MoveMode::move() | MoveMode::fight();
  MoveMode moveMode_ = DefaultMoveMode;
  bool changeDigitLast_ = false;
  bool continuePlaying_ = true;
  std::size_t count_ = NoCount;
};
namespace IOModule {
export class Interface;
}
using IOModule::Interface;
using streambufT = std::remove_pointer_t<decltype(Logging::log.rdbuf())>;
class PrintToViewer : public streambufT {
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
    int Mapwidth = static_cast<int>(currentMap.cols());
    int MapHeight = static_cast<int>(currentMap.rows());
    auto [height, width] = getMaxDims();
    const int mainWindowHeight = height - 6 - MapHeight;
    eventWindow_.setDims(Mapwidth, mainWindowHeight - 2);
    mainWindow_.move(0, mainWindowHeight);
    mainWindow_.setDims(Mapwidth, MapHeight);
    inventWindow_.move(Mapwidth + 2, 0);
    inventWindow_.setDims(width - Mapwidth - 4, height - 2);
    statusWindow_.move(0, height - 4);
    statusWindow_.setDims(Mapwidth, 2);
    auto &memory = getMemoryGrid(gState_->getLocation().mapPos, Mapwidth, MapHeight);
    for (int y = 0; y < MapHeight; y++) {
      mainWindow_.moveCursor(0, y);
      for (int x = 0; x < Mapwidth; x++) {
        Position pos{x, y};
        auto tile = currentMap.getTile(pos);
        if (tile.terrainType != TerrainTypeInterface::Unknown) {
          memory[pos] = tile.terrainType;
          mainWindow_ << TileToSymbol(currentMap, pos);
        } else {
          mainWindow_ << MemoryTerrainToSymbol(memory[pos]);
        }
      }
    }
    ObjectContainerInterface playerInvent = gState_->lookAtInventory();
    displayInvent(inventWindow_, playerInvent);
    inventWindow_.updateScreen();
    mainWindow_.updateScreen();
    statusWindow_.clear();
    statusWindow_.moveCursor(0, 0);
    statusWindow_ << "Health: "sv << gState_->getHealth();
    statusWindow_.updateScreen();
    displayEvents(eventWindow_, eventLog_);
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
  void addEvent(std::string str) noexcept {
    eventLog_.emplace_back(std::move(str));
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
    return getMemoryGrid(floor, static_cast<int>(currentMap.cols()), static_cast<int>(currentMap.rows()));
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

private:
  [[no_unique_address]] CursesRAII Raii_;
  BoxedWindow mainWindow_;
  BoxedWindow inventWindow_;
  BoxedWindow statusWindow_;
  BoxedWindow eventWindow_;
  ActionMod mod_;
  std::vector<std::string> eventLog_;
  std::unique_ptr<GameInterface> gState_;
  std::unordered_map<int, StaticPositionArr<TerrainTypeInterface>> terrainMemory_;
  streambufT *oldBuffer_;
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
struct FloorInterfaceWrapper {
  WorldFloorInterface floor;
  [[nodiscard]] int extent(int n) const noexcept {
    switch (n) {
    case 0:
      return static_cast<int>(floor.rows());
    case 1:
      return static_cast<int>(floor.cols());
    default:
      std::unreachable();
    }
  }
  [[nodiscard]] bool operator[](int row, int col) const noexcept {
    auto tile = floor.getTile(Position{col, row});
    return tile.terrainType != TerrainTypeInterface::Unknown && !isWall(tile.terrainType);
  }
};
struct MemoryFloorWrapper {
  const StaticPositionArr<TerrainTypeInterface> &memory;
  [[nodiscard]] int extent(int n) const noexcept {
    switch (n) {
    case 0:
      return memory.height();
    case 1:
      return memory.width();
    default:
      std::unreachable();
    }
  }
  [[nodiscard]] bool operator[](int row, int col) const noexcept {
    auto terrain = memory[Position{col, row}];
    return terrain != TerrainTypeInterface::Unknown && !isWall(terrain);
  }
};

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
    if (userInput >= 'a' && userInput < 'a' + static_cast<std::int64_t>(items.size())) {
      auto index = static_cast<std::size_t>(userInput - 'a');
      if (settings.isEligible(items[index]))
        return index;
    }
  }
  return NoItem;
}

void toggleMoveMode(GameInterface & /*gState*/, IOModule::Interface & /*unused*/, ActionMod &mod) {
  mod.toggleMoveMode(MoveMode::move());
}

void toggleFightMode(GameInterface & /*gState*/, IOModule::Interface & /*unused*/, ActionMod &mod) {
  mod.toggleMoveMode(MoveMode::fight());
}

void pickUpItem(GameInterface &gState, IOModule::Interface &interface, ActionMod & /*mod*/) {
  std::size_t index = getItemFromInterface(interface, gState.lookAtFloor(), "What do you want to pick up?", {.autoSelectOne = true});
  if (index != NoItem) {
    gState.pickUpItem(index);
  }
}

std::optional<Position> chooseTile(GameInterface &gState, IOModule::Interface &iterface) noexcept {
  auto pos = gState.getLocation().pos;
  iterface.showSelection(pos);
  while (true) {
    auto cmnd = CursesRAII::getChar();
    if (cmnd == SpecialChar::Escape)
      return {};
    if (cmnd == '.')
      return pos;
    auto dir = keyToDir(cmnd);
    int step = (cmnd >= 'A' && cmnd <= 'Z') ? 10 : 1;
    auto jump = Dir(dir.dx * step, dir.dy * step);
    if (iterface.showSelection(pos + jump)) {
      pos += jump;
    }
  }
}

void throwItem(GameInterface &gState, IOModule::Interface &iterface, ActionMod & /*mod*/) noexcept {
  auto inventory = gState.lookAtInventory();
  if (inventory.empty())
    return;
  auto index = getItemFromInterface(iterface, inventory, "What do you want to throw?", {.doStandAloneDisplay = false});
  if (index == NoItem)
    return;
  auto target = chooseTile(gState, iterface);
  if (!target)
    return;
  gState.throwItem(index, (*target) - gState.getLocation().pos);
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
  std::size_t index = getItemFromInterface(interface, gState.lookAtInventory(), "What do you want to drop?", {.doStandAloneDisplay = false});
  if (index != NoItem) {
    gState.dropItem(index);
  }
}

void passTime(GameInterface &gState, IOModule::Interface & /*unused*/, ActionMod &mod) noexcept {
  gState.passTime(TimePeriod(mod.getCount(gState.getSpeed().impl)));
}

void autoPath(GameInterface &gState, IOModule::Interface &interface, ActionMod &mod) noexcept {
  auto target = chooseTile(gState, interface);
  if (!target)
    return;
  const Position goal = *target;
  const FloorSpecifier currentFloor = gState.getLocation().mapPos;
  while (true) {
    const Position currentPos = gState.getLocation().pos;
    if (currentPos == goal)
      break;
    if (CursesRAII::tryGetChar().has_value())
      break;
    if (gState.getLocation().mapPos != currentFloor)
      break;
    const Health healthBefore = gState.getHealth();
    Dir step = FindPath::findPath(
        MemoryFloorWrapper{interface.getMemory(currentFloor)},
        currentPos, goal);
    if (step.noMove())
      break;
    gState.generalMove(step, mod.getMoveMode());
    interface.updateGameScreen();
    if (gState.getHealth() < healthBefore) // at some point change to getting allerted by message.
      break;
    if (gState.getLocation().pos == currentPos)
      break;
  }
}

template <int Dx, int Dy>
void runInDir(GameInterface &gState, IOModule::Interface &interface, ActionMod & /*mod*/) noexcept {
  static_assert(Dx <= 1 && Dy <= 1 && Dx >= -1 && Dy >= -1 && (Dx != Dy || Dx != 0));
  constexpr static Dir D = Dir(Dx, Dy);
  while (true) {
    const Position posBefore = gState.getLocation().pos;
    const Health healthBefore = gState.getHealth();
    gState.generalMove(D, MoveMode::move());
    interface.updateGameScreen();
    if (gState.getLocation().pos == posBefore)
      break;
    if (gState.getHealth() < healthBefore)
      break;
    if (CursesRAII::tryGetChar().has_value())
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
    {'.', passTime},
    {'_', autoPath},

    {SpecialChar::Backspace, quit},
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
    std::string ret = std::to_string(parent_->getTime().impl);
    ret += ": ";
    ret += buffer_;
    parent_->addEvent(std::move(ret));
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