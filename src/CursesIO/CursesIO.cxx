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
  case 'h':
    return {-1, 0};
  case SpecialChar::Down:
  case 'j':
    return {0, 1};
  case SpecialChar::Up:
  case 'k':
    return {0, -1};
  case SpecialChar::Right:
  case 'l':
    return {1, 0};
  case 'y':
    return {-1, -1};
  case 'u':
    return {1, -1};
  case 'b':
    return {-1, 1};
  case 'n':
    return {1, 1};
  default:
    return {};
  }
}
} // namespace

// static BoxedWindow *EventWindow;
namespace {
void displayInvent(BoxedWindow &window, ObjectContainerInterface items) {
  window.clear();
  std::array front = {'a', ' ', '-', ' '};
  for (int y = 0; y < std::min(window.prntHeight(), static_cast<int>(items.size())); y++) {
    window.moveCursor(0, y);
    window << std::string_view(front);
    front[0]++;
    if (front[0] == 'z' + 1)
      front[0] = 'A';
    if (front[0] == 'Z' + 1)
      break;
    window << items[y];
  }
  window.updateScreen();
}

void displayEvents(BoxedWindow &window, const std::vector<std::string> &arr) {
  window.clear();
  const std::size_t printHeight = window.prntHeight();
  const std::size_t offSet = arr.size() < printHeight ? 0 : arr.size() - printHeight;
  for (std::size_t i = 0; i < std::min<std::size_t>(arr.size(), printHeight); i++) {
    window.moveCursor(0, i);
    window << std::string_view(arr[i + offSet]);
  }
  window.updateScreen();
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
  Interface() : printToViewer_(this) {
    eventWindow_ = BoxedWindow(0, 0, 0, 0);
    mainWindow_ = BoxedWindow(0, 0, 0, 0);
    inventWindow_ = BoxedWindow(0, 0, 0, 0);
    statusWindow_ = BoxedWindow(0, 0, 0, 0);
    oldBuffer_ = Logging::log.rdbuf(&printToViewer_);
    gState_ = std::make_unique<GameInterface>(std::make_unique<CursesEventViewer>(this));
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
    for (int y = 0; y < MapHeight; y++) {
      mainWindow_.moveCursor(0, y);
      for (int x = 0; x < Mapwidth; x++) {
        mainWindow_ << TileToSymbol(currentMap, {x, y});
      }
    }
    ObjectContainerInterface playerInvent = gState_->lookAtInventory();
    displayInvent(inventWindow_, playerInvent);
    mainWindow_.updateScreen();
    statusWindow_.clear();
    statusWindow_.moveCursor(0, 0);
    statusWindow_ << "Health: "sv << gState_->getHealth();
    statusWindow_.updateScreen();
    displayEvents(eventWindow_, eventLog_);
    // eventWindow_.updateScreen();
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
  [[nodiscard]] GameTime getTime() const noexcept {
    if (gState_)
      return gState_->getTime();
    return {};
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
  streambufT *oldBuffer_;
  PrintToViewer printToViewer_;
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
struct ItemFromInterfaceSettings {
  bool doDisplay = true;
  bool autoSelectOne = true;
};

constexpr std::size_t NoItem = std::numeric_limits<std::size_t>::max();
template <ItemFromInterfaceSettings settings = {}>
std::size_t getItemFromInterface(ObjectContainerInterface interface) noexcept {
  if (interface.size() == 0) {
    return NoItem;
  }
  if (settings.autoSelectOne && interface.size() == 1) {
    return 0;
  }
  if constexpr (settings.doDisplay) {
    auto [height, width] = getMaxDims();
    height = std::min<int>(height - 2, interface.size());
    constexpr int DesiredWidth = 30;
    width = std::min<int>(width - 3, DesiredWidth);
    auto window = BoxedWindow(width, height, 1, 1);
    displayInvent(window, interface);
  }
  while (true) {
    auto userInput = CursesRAII::getChar();
    if (userInput == SpecialChar::Escape) {
      return NoItem;
    }
    if (userInput >= 'a' && userInput < 'a' + static_cast<std::int64_t>(interface.size())) {
      return userInput - 'a';
    }
  }
}

void toggleMoveMode(GameInterface & /*gState*/, IOModule::Interface & /*unused*/, ActionMod &mod) {
  mod.toggleMoveMode(MoveMode::move());
}

void toggleFightMode(GameInterface & /*gState*/, IOModule::Interface & /*unused*/, ActionMod &mod) {
  mod.toggleMoveMode(MoveMode::fight());
}

void pickUpItem(GameInterface &gState, IOModule::Interface & /*unused*/, ActionMod & /*mod*/) {
  std::size_t index = getItemFromInterface(gState.lookAtFloor());
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
    if (iterface.showSelection(pos + dir)) {
      pos += dir;
    }
  }
}

void throwItem(GameInterface &gState, IOModule::Interface &iterface, ActionMod & /*mod*/) noexcept {
  std::size_t index = getItemFromInterface<{.doDisplay = false, .autoSelectOne = false}>(gState.lookAtInventory());
  if (index == NoItem)
    return;
  auto target = chooseTile(gState, iterface);
  if (!target)
    return;
  gState.throwItem(index, (*target) - gState.getLocation().pos);
}

void dropItem(GameInterface &gState, IOModule::Interface & /*unused*/, ActionMod & /*mod*/) noexcept {
  std::size_t index = getItemFromInterface<{.doDisplay = false, .autoSelectOne = false}>(gState.lookAtInventory());
  if (index != NoItem) {
    gState.dropItem(index);
  }
}

void passTime(GameInterface &gState, IOModule::Interface & /*unused*/, ActionMod &mod) noexcept {
  gState.passTime(TimePeriod(mod.getCount(gState.getSpeed().impl)));
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
    {'t', throwItem},
    {',', pickUpItem},
    {'.', passTime},

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
  printWith_ << attacker << " hit " << toPrintAbleObject(attacked) << '\n';
}

// 𐁀