export module CursesIO;
import CursesLowLevel;
import Common;
import GameInterface;

using namespace std::literals;
export namespace IOExceptions {
using IOModuleException = CursesLowLevel::IOExceptions::IOModuleException;
}
using namespace CursesLowLevel;

attr_t toModifierChar(const MonsterInterface &monst) noexcept {
  if (monst.isPlayer()) {
    return Modifier::Standout;
  }
  return Modifier::Normal;
}

Color toColorChar(const MonsterInterface &monst) noexcept {
  switch (monst.getClass()) {
    using enum MonsterClass;
  case Human:
    return White;
  case Cat:
    return White;
  case SeaSlug:
    return Magenta;
  case GreedyWeasel:
    return Brown;
  case Bryozoan:
    return DarkBlue;
  }
}

chtype toDisplayChar(const MonsterInterface &monst) noexcept {
  switch (monst.getClass()) {
    using enum MonsterClass;
  case Human:
    return '@';
  case Cat:
    return 'f';
  case SeaSlug:
    return '~';
  case GreedyWeasel:
    return 'w';
  case Bryozoan:
    return L'˚';
  }
}

constexpr std::array Vowels = {'a', 'e', 'i', 'o', 'u', 'y'};
template <class T, std::size_t size>
constexpr bool inArray(const std::array<T, size> &arr, const T &v) noexcept {
  return std::ranges::any_of(arr, [v](const T &i) { return i == v; });
}

class Word {
  public:
  std::string_view word;
  bool weirdAn = false;
};

class Noun : public Word {
  public:
  std::string_view weirdPlural{}; //NOLINT(readability-redundant-member-init)
};

class Adjective : public Word {};

[[nodiscard]] constexpr bool usesAn(Word word) noexcept{
  return word.weirdAn != inArray(Vowels, word.word[0]);
}

class PrintableObject {
  using DescriptorsType = std::array<Adjective, 4>;

public:
  constexpr explicit PrintableObject(Noun name, std::size_t count = 1) noexcept : name_{name}, count_{count}{}
  constexpr void addDescriptor(Adjective descriptor) noexcept { descriptors_[numDescriptors_++] = descriptor; }
  constexpr void setUseThe() { useThe_ = true; }
  constexpr void setCount(std::size_t count) { count_ = count; }
  [[nodiscard]] constexpr std::string_view getNameSingular() const noexcept {
    return name_.word;
  }
  [[nodiscard]] constexpr std::string_view getNameWeirdPlural() const noexcept {
    return name_.weirdPlural;
  }
  [[nodiscard]] constexpr std::string_view getNamePluralSuffix() const noexcept {
    return name_.word.back() == 's' ? "es" : "s";
  }
  [[nodiscard]] constexpr std::size_t getCount() const noexcept {
    return count_;
  }
  [[nodiscard]] constexpr std::string_view getSingularPrefix() const noexcept {
    if (useThe_) {
      return "the";
    }
    return (numDescriptors_ == 0 ? usesAn(name_) : usesAn(descriptors_[0])) ? "an" : "a";
  }
  [[nodiscard]] constexpr Iterable<DescriptorsType::const_iterator> getDescriptors() const noexcept {
    return {descriptors_.begin(), descriptors_.begin() + numDescriptors_};
  }

private:
  Noun name_;
  std::size_t count_ = 1;
  std::size_t numDescriptors_ = 0;
  DescriptorsType descriptors_;
  bool useThe_ = false;
};

template <class Printer>
Printer &operator<<(Printer &str, const PrintableObject &obj) noexcept {
  if (obj.getCount() == 1) {
    str << obj.getSingularPrefix();
  } else {
    str << obj.getCount();
  }
  str << ' ';
  for (const auto& word : obj.getDescriptors()) {
    str << word.word << ' ';
  }
  std::string_view word;
  std::string_view plural;
  if (obj.getCount() != 1) {
    word = obj.getNameWeirdPlural();
  }
  if (word.empty()) {
    word = obj.getNameSingular();
    if (obj.getCount() != 1) {
      plural = obj.getNamePluralSuffix();
    }
  }
  str << word << plural;
  return str;
}

[[nodiscard]] Noun toName(ObjectInterface obj) noexcept {
  switch (obj.type()) {
    using enum ObjectType;
  case KingsCoin:
    return {{"coin"}};
  case Knife:
    return {{"knife"}, "knives"};
  case Die:
    return {{"die"}, "dice"};
  }
}

[[nodiscard]] Adjective getMatAdj(ObjectInterface obj) noexcept {
  switch (obj.mat()) {
    using enum Material;
  case Gold:
    return {obj.type() == ObjectType::KingsCoin ? "gold" : "golden"};
  case Iron:
    return {"iron"};
  case Plastic:
    return {"plastic"};
  case Wood:
    return {"wooden"};
  }
}

[[nodiscard]] PrintableObject toPrintAbleObject(ObjectInterface obj) noexcept {
  const Noun ObjName = toName(obj);
  const Adjective matDescriptor = getMatAdj(obj);
  PrintableObject printer(ObjName,obj.count());
  printer.addDescriptor(matDescriptor);
  if (obj.artifactStatus() != ArtifactId::Normal) {
    printer.setUseThe();
  }
  return printer;
}

template <class Printer>
Printer &operator<<(Printer &str, const ObjectInterface &obj) noexcept {
  return str << toPrintAbleObject(obj);
}

Symbol MonsterToSymbol(MonsterInterface monst) noexcept {
  Symbol sym = toDisplayChar(monst);
  sym.addModifier(toModifierChar(monst));
  sym.setFrontColor(toColorChar(monst));
  sym.setBackColor(Black);
  return sym;
}

constexpr chtype ObjectTypeToCharacter(ObjectType otype) noexcept {
  switch (otype) {
    using enum ObjectType;
  case KingsCoin:
    return '$';
  case Knife:
    return ')';
  case Die:
    return '(';
  }
}

constexpr Color ObjectMaterialToColor(Material otype) noexcept {
  switch (otype) {
    using enum Material;
  case Gold:
    return Yellow;
  case Iron:
    return White;
  case Plastic:
    return BrightWhite;
  case Wood:
    return Brown;
  }
}

Symbol ObjectToSymbol(ObjectInterface obj) noexcept {
  Symbol sym = ObjectTypeToCharacter(obj.type());
  Color c = ObjectMaterialToColor(obj.mat());
  sym.setFrontColor(c);
  return sym;
}

Symbol TerrainTypeToSymbol(WorldFloorInterface floor, Position pos) noexcept {
  const auto tile = floor.getTile(pos);
  const auto c = tile.terrainType;
  switch (c) {
    using enum TerrainType;
  case Empty:
    return '.';
  case Wall:
    auto getType = [floor](Position pos){
      return floor.inBounds(pos) && floor.getTile(pos).terrainType==Wall;
    };
    auto check = [&getType,pos](Dir dir){
      if(!getType(pos+dir)){
        return false;
      }
      auto [dx,dy] = dir;
      Dir oDir(dy,dx);
      // return true;
      return !(getType(pos+oDir)&&getType(pos-oDir)&&getType(pos+dir+oDir)&&getType(pos+dir-oDir));
    };
    using enum SpecialChar::Directions;
    SpecialChar::Directions dir = None;
    if(check(Dir::up()))
      dir|=Up;
    if(check(Dir::down()))
      dir|=Down;
    if(check(Dir::left()))
      dir|=Left;
    if(check(Dir::right()))
      dir|=Right;
    if(dir==None && getType(pos.up())){
      return ' ';
    }
    return SpecialChar::Walls[dir];
  }
}

Symbol TileToSymbol(WorldFloorInterface floor, Position pos) noexcept {
  const auto tile = floor.getTile(pos);
  auto monstPtr = tile.monster;
  if (!monstPtr.isNull()) {
    return MonsterToSymbol(monstPtr);
  }
  if (!tile.objects.empty()) {
    return ObjectToSymbol(tile.objects.back());
  }
  return TerrainTypeToSymbol(floor,pos);
}

std::string_view getName(MonsterInterface monster) noexcept {
  switch (monster.getClass()) {
    using enum MonsterClass;
  case Human:
    return "human";
  case Cat:
    return "cat";
  case SeaSlug:
    return "sea slug";
  case GreedyWeasel:
    return "greedy weasel";
  case Bryozoan:
    return "bryozoan";
  }
}

// constexpr Dir keyToDir(chtype key) noexcept {
//   switch (key) {
//   case SpecialChar::Left:
//   case 'h':
//     return Dir(-1, 0);
//   case SpecialChar::Down:
//   case 'j':
//     return Dir(0, 1);
//   case SpecialChar::Up:
//   case 'k':
//     return Dir(0, -1);
//   case SpecialChar::Right:
//   case 'l':
//     return Dir(1, 0);
//   case 'y':
//     return Dir(-1, -1);
//   case 'u':
//     return Dir(1, -1);
//   case 'b':
//     return Dir(-1, 1);
//   case 'n':
//     return Dir(1, 1);
//   default:
//     return Dir();
//   }
// }

// static BoxedWindow *EventWindow;

void displayInvent(BoxedWindow &window, ObjectContainerInterface items) {
  window.clear();
  std::array front = {'a', ' ', '-', ' '};
  for (int y = 0; y < std::min(window.prntHeight(), static_cast<int>(items.size())); y++) {
    window.moveCursor(0, y);
    window << std::string_view(front);
    front[0]++;
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

class ActionMod {
public:
  MoveMode moveMode = MoveMode::move() | MoveMode::fight();
};

template <int Dx, int Dy>
constexpr bool movePlayer(GameInterface &gState, ActionMod &modifer) noexcept {
  static_assert(Dx <= 1 && Dy <= 1 && Dx >= -1 && Dy >= -1 && (Dx != Dy || Dx != 0));
  constexpr static Dir D = Dir(Dx, Dy);
  gState.generalMove(D, modifer.moveMode);
  return true;
}

struct ItemFromInterfaceSettings {
  bool doDisplay = true;
  bool autoSelectOne = true;
};

static constexpr std::size_t NoItem = std::numeric_limits<std::size_t>::max();
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
    auto userInput = getChar();
    if (userInput == SpecialChar::Escape) {
      return NoItem;
    }
    if (userInput >= 'a' && userInput  < 'a'+static_cast<std::int64_t>(interface.size())) {
      return userInput - 'a';
    }
  }
}

bool toggleMoveMode(GameInterface & /*gState*/, ActionMod &mod) {
  mod.moveMode ^= MoveMode::move();
  return true;
}

bool toggleFightMode(GameInterface & /*gState*/, ActionMod &mod) {
  mod.moveMode ^= MoveMode::fight();
  return true;
}

bool pickUpItem(GameInterface &gState, ActionMod & /*mod*/) {
  std::size_t index = getItemFromInterface(gState.lookAtFloor());
  if (index != NoItem) {
    gState.pickUpItem(index);
  }
  return true;
}

bool throwItem(GameInterface &gState, ActionMod & /*mod*/) noexcept {
  std::size_t index = getItemFromInterface<{.doDisplay = false, .autoSelectOne = false}>(gState.lookAtInventory());
  if (index != NoItem) {
    gState.throwItem(index);
  }
  return true;
}

bool dropItem(GameInterface &gState, ActionMod & /*mod*/) noexcept {
  std::size_t index = getItemFromInterface<{.doDisplay = false, .autoSelectOne = false}>(gState.lookAtInventory());
  if (index != NoItem) {
    gState.dropItem(index);
  }
  return true;
}

bool quit(GameInterface &gState, ActionMod & /*mod*/) noexcept {
  gState.exit();
  return false;
}

using ActionType = bool (*)(GameInterface &, ActionMod &);
static constexpr auto CmndMpPairs = CompileTimeHashMap::to_Pairing<std::uint16_t, ActionType>({
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

    {'F', toggleFightMode},
    {'m', toggleMoveMode},

    {'d', dropItem},
    {'t', throwItem},
    {',', pickUpItem},

    {SpecialChar::Backspace, quit},
});
namespace IOModule {
export class Interface;
}
using IOModule::Interface;

class CursesEventViewer final : public EventViewerInteface {
public:
  explicit CursesEventViewer(Interface &parent) noexcept : parent_(&parent) {}
  void itemPickup(MonsterInterface grabber, ObjectInterface grabed) final;
  void monsterAttack(HitInfo hitinfo, MonsterInterface attacker, MonsterInterface attacked) final;
  void debug(std::string_view message) final;
  void exception(const std::exception &exception) noexcept final;
private:
  Interface *parent_;
};

constexpr auto CmndMp = CompileTimeHashMap::to_Map<CmndMpPairs, 0, nullptr>();
constexpr std::size_t CmndMpSize = CmndMp.size();
export namespace IOModule {
class Interface {
public:
  Interface() {
    eventWindow_ = BoxedWindow(0, 0, 0, 0);
    mainWindow_ = BoxedWindow(0, 0, 0, 0);
    inventWindow_ = BoxedWindow(0, 0, 0, 0);
    statusWindow_ = BoxedWindow(0, 0, 0, 0);
  }
  void createTiedGameInterface() noexcept {
    gState_ = std::make_unique<GameInterface>(std::make_unique<CursesEventViewer>(*this));
  }
  void tieGameInterface(std::unique_ptr<GameInterface> gState) {
    gState_ = std::move(gState);
    gState_->setEventViewer(std::make_unique<CursesEventViewer>(*this));
  }
  [[nodiscard]] std::unique_ptr<GameInterface> getGameInterface() noexcept {
    return std::move(gState_);
  }
  void updateGameScreen() {
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
        mainWindow_ << TileToSymbol(currentMap,{x,y});
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
    chtype userInput = getChar();
    const auto func = CmndMp.get(userInput);

    if (func == nullptr) {
      return true;
    }
    return func(*gState_, mod_);
  }
  void addEvent(std::string str) noexcept {
    eventLog_.emplace_back(std::move(str));
  }
  [[nodiscard]] GameTime getTime() const noexcept {
    return gState_->getTime();
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
};
} // namespace IOModule

void CursesEventViewer::debug(std::string_view message) {
  std::stringstream stream;
  stream << parent_->getTime().impl << ": " << message;
  parent_->addEvent(stream.str());
}
void CursesEventViewer::exception(const std::exception &exception) noexcept {
  const auto time = parent_->getTime().impl;
  std::fstream logfile("log.txt", std::ios_base::out | std::ios_base::app);
  logfile.exceptions(std::fstream::goodbit);
  logfile << time << ": " << exception.what() << '\n';
  if (!logfile.is_open()) {
    std::cerr << "Unhandeled exception, and log file does not open\n"
              << time << ": " << exception.what() << '\n';
    std::exit(1);
  }
  if (logfile.bad()) {
    std::cerr << "I/O error while reading - badbit is true\n"
              << exception.what() << '\n';
    std::exit(1);
  } else if (logfile.fail()) {
    std::cerr << "#4. Non-integer data encountered - failbit is true\n"
              << exception.what() << '\n';
  }
  logfile.sync();
}

std::ostream& operator<<(std::ostream& out, GameTime time){
  return out << time.impl;
}


std::ostream& operator<<(std::ostream& out, MonsterInterface monster){
  if(monster.isPlayer()){
    out << "you";
  } else {
    out << "a " << getName(monster);
  }
  return out;
}

void CursesEventViewer::itemPickup(MonsterInterface grabber, ObjectInterface grabed) {
  std::stringstream stream;
  stream << parent_->getTime() << ": " << grabber << " picked up " << grabed;
  parent_->addEvent(stream.str());
}


void CursesEventViewer::monsterAttack(HitInfo info, MonsterInterface attacker, MonsterInterface attacked) {
  std::stringstream stream;
  const std::string_view verb = info.killed ? "killed" : "hit";
  stream << parent_->getTime() << ": " << attacker << " " << verb << " " << attacked;
  parent_->addEvent(stream.str());
}

// 𐁀