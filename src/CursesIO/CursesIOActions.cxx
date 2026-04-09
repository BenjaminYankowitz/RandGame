module CursesIO;

using namespace CursesLowLevel;

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

std::optional<Position> chooseTile(GameInterface &gState, IOModule::Interface &iterface,
                                   bool calcFrontier = false, int maxRange = 0) noexcept {
  std::vector<Position> cycleTargets;
  const auto playerPos = gState.getLocation().pos;
  auto inRange = [&](Position p) { return maxRange <= 0 || Position::chessboard(p, playerPos) <= maxRange; };
  auto notInRange = [&](Position p) { return !inRange(p); };
  if (calcFrontier) {
    const FloorSpecifier currentFloor = gState.getLocation().mapPos;
    cycleTargets = findUnexploredFrontier(iterface.getMemory(currentFloor), playerPos);
    const auto [remfirst, remlast] = std::ranges::remove_if(cycleTargets,notInRange);
    cycleTargets.erase(remfirst,remlast);
  }
  auto pos = playerPos;
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
      iterface.showSelection(pos);
      cycleIndex = (cycleIndex + 1) % cycleTargets.size();
      continue;
    }
    if (cmnd == '>' || cmnd == '<') {
      auto target = (cmnd == '>') ? TerrainTypeInterface::DownStair : TerrainTypeInterface::UpStair;
      const auto &memory = iterface.getMemory(gState.getLocation().mapPos);
      auto npos = findTerrain(memory, pos, target);
      if (npos.has_value() && inRange(*npos)) {
        pos = *npos;
        iterface.showSelection(pos);
      }
      continue;
    }
    auto dir = keyToDir(std::tolower(cmnd));
    int step = (cmnd >= 'A' && cmnd <= 'Z') ? 10 : 1;
    auto jump = Dir(dir.dx * step, dir.dy * step);
    auto desired = pos + jump;
    auto floor = gState.getFloor(gState.getLocation().mapPos);
    int maxX = floor.cols() - 1;
    int maxY = floor.rows() - 1;
    desired.x = std::clamp(desired.x, 0, maxX);
    desired.y = std::clamp(desired.y, 0, maxY);
    if (inRange(desired) && iterface.showSelection(desired)) {
      pos = desired;
    }
  }
}

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
  auto target = chooseTile(gState, interface, false, gState.getMaxThrowingDistance());
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
  if (!gState.isDebugMode())
    return;
  auto target = chooseTile(gState, interface, true);
  if (!target)
    return;
  gState.teleport(*target);
  interface.updateGameScreen();
}

void saveGame(GameInterface &gState, IOModule::Interface &interface, ActionMod & /*mod*/) noexcept {
  std::ofstream file("RandGameSave", std::ios::binary | std::ios::trunc);
  if (!file) {
    interface.interfacePrinter() << "Failed to open save file.\n";
    interface.updateGameScreen();
    return;
  }
  gState.save(file);
  interface.interfacePrinter() << "Game saved.\n";
  interface.updateGameScreen();
}

void saveAndQuit(GameInterface &gState, IOModule::Interface &interface, ActionMod &mod) noexcept {
  saveGame(gState, interface, mod);
  gState.exit();
  mod.quitGame();
}

void giveUp(GameInterface &gState, IOModule::Interface & /*unused*/, ActionMod &mod) noexcept {
  std::filesystem::remove("RandGameSave");
  gState.exit();
  mod.quitGame();
}

void debugSave(GameInterface &gState, IOModule::Interface &interface, ActionMod & /*mod*/) noexcept {
  if (!gState.isDebugMode())
    return;
  std::string &prompt = interface.addEvent("Save to: ");
  interface.updateGameScreen();
  while (true) {
    auto ch = CursesRAII::getChar();
    if (ch == '\n')
      break;
    if (ch == SpecialChar::Escape)
      return;
    if (ch == SpecialChar::Backspace || ch == 127 || ch == '\b') {
      if (!prompt.empty() && prompt.back() != ' ')
        prompt.pop_back();
    } else if (ch >= ' ' && ch <= '~') {
      prompt += static_cast<char>(ch);
    }
    interface.updateGameScreen();
  }
  std::string filename(std::string_view(prompt).substr(std::string_view("Save to: ").size()));
  if (filename.empty())
    return;
  std::ofstream file(filename, std::ios::binary | std::ios::trunc);
  if (!file) {
    interface.interfacePrinter() << "Failed to open file: " << filename << "\n";
    interface.updateGameScreen();
    return;
  }
  gState.save(file);
  interface.interfacePrinter() << "Game saved to " << filename << ".\n";
  interface.updateGameScreen();
}

void debugLoad(GameInterface &gState, IOModule::Interface &interface, ActionMod & /*mod*/) noexcept {
  if (!gState.isDebugMode())
    return;
  std::string &prompt = interface.addEvent("Load from: ");
  interface.updateGameScreen();
  while (true) {
    auto ch = CursesRAII::getChar();
    if (ch == '\n')
      break;
    if (ch == SpecialChar::Escape)
      return;
    if (ch == SpecialChar::Backspace || ch == 127 || ch == '\b') {
      if (!prompt.empty() && prompt.back() != ' ')
        prompt.pop_back();
    } else if (ch >= ' ' && ch <= '~') {
      prompt += static_cast<char>(ch);
    }
    interface.updateGameScreen();
  }
  std::string filename(std::string_view(prompt).substr(std::string_view("Load from: ").size()));
  if (filename.empty())
    return;
  std::ifstream file(filename, std::ios::binary);
  if (!file) {
    interface.interfacePrinter() << "Failed to open file: " << filename << "\n";
    interface.updateGameScreen();
    return;
  }
  auto result = gState.load(file);
  if (!result.ok()) {
    if (result.error == GameInterface::LoadResult::Error::BadMagic) {
      interface.interfacePrinter() << "Not a valid save file.\n";
    } else {
      interface.interfacePrinter() << "Version mismatch: file version " << result.fileVersion << ", expected " << result.expectedVersion << "\n";
    }
    interface.updateGameScreen();
    return;
  }
  interface.interfacePrinter() << "Game loaded from " << filename << ".\n";
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
    {"wait", passTime},
    {"autoexplore", autoExplore},
    {"debug mode", debugModeOn, false, false},
    {"remove debug mode", debugModeOff, true},
    {"see all", seeAll, true},
    {"remove see all", seeAllOff, true},
    {"teleport", teleport, true},
    {"save", saveGame},
    {"save and quit", saveAndQuit},
    {"give up", giveUp},
    {"debug save", debugSave, true},
    {"debug load", debugLoad, true},
});

auto validCommands(bool debugMode, bool inSuggestion) {
  return std::views::filter(ExtendedCommands, [debugMode, inSuggestion](const ExtendedCommand &cmd) {
    if (cmd.debugModeOnly && !debugMode)
      return false;
    if (inSuggestion && !cmd.autoComplete)
      return false;
    return true;
  });
}

std::string_view findSuggestion(std::string_view typed, bool debugMode) {
  auto possibleExtensions = validCommands(debugMode, true) | std::views::transform([](const ExtendedCommand &cmd) { return cmd.text; }) | std::views::filter([typed](std::string_view cmd) { return cmd.starts_with(typed); });
  if (possibleExtensions.empty()) {
    return {};
  }
  std::string_view prefix = possibleExtensions.front();
  for (auto possibleExtension : possibleExtensions | std::views::drop(1))
    prefix = prefix.substr(0, std::ranges::mismatch(prefix, possibleExtension).in1 - prefix.begin());
  return prefix.substr(typed.size());
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
  auto choices = validCommands(debugMode, false);
  auto cmd = std::ranges::find_if(choices, [name = name.subview(1)](const ExtendedCommand &cmd) { return cmd == name; });
  if (cmd != choices.end())
    cmd->command(gState, interface, mod);
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
    {cntrl('T'), teleport},

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
} // namespace

ActionType getActionFromInput(std::int16_t input) noexcept {
  static constexpr auto CmndMp = CompileTimeHashMap::to_Map<CmndMpPairs, 0, nullptr>();
  return CmndMp.get(input);
}

} // namespace Actions
