export module CursesIO;
import CursesLowLevel;
import Common;
import GameInterface;
import Printing;
import CursesIOHelper;

using namespace CursesLowLevel;

namespace IOModule {
export class Interface;
}
using IOModule::Interface;

struct PrintToViewer : public std::basic_streambuf<char> {
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
  void itemEquipped(MonsterInterface wearer, ObjectInterface item) final;
  void itemUnequipped(MonsterInterface wearer, ObjectInterface item) final;
  void equipSlotsFull(MonsterInterface wearer, ObjectInterface item) final;
  void monsterHitMonster(HitInfo hitinfo, MonsterInterface attacker, MonsterInterface attacked) final;
  void monsterHitWall(MonsterInterface attacker, TerrainTypeInterface attacked) final;
  void monsterAte(MonsterInterface eater, ObjectInterface eaten) final;
  void beamStep(Location loc) final;
  void debug(std::string_view message) final;
  void exception(const std::exception &exception) noexcept final;

private:
  PrintToViewer viewer_;
  std::ostream printWith_;
};

namespace Actions {
using ActionType = void (*)(GameInterface &, IOModule::Interface &, ActionMod &);
ActionType getActionFromInput(std::int16_t input) noexcept;
} // namespace Actions

export namespace IOModule {
class Interface {
public:
  explicit Interface(std::unique_ptr<GameInterface> interface);
  Interface(const Interface&) =delete;
  Interface(const Interface&&) =delete;
  Interface& operator=(const Interface&) =delete;
  Interface& operator=(const Interface&&) =delete;
  ~Interface();
  void updateGameScreen();
  void showSuggestion(std::string_view suggestion);
  int eventWindowWidth() const noexcept { return eventWindow_.prntWidth(); }
  [[nodiscard]] bool doAction();
  std::string &addEvent(std::string str) noexcept {
    eventLog_.emplace_back(getTime(), std::move(str));
    return eventLog_.back().second;
  }
  [[nodiscard]] std::ostream &interfacePrinter() { return interfaceStream_; }
  [[nodiscard]] GameTime getTime() const noexcept;
  [[nodiscard]] const StaticPositionArr<TerrainTypeInterface> &getMemory(FloorSpecifier floor);
  bool showSelection(Position pos);
  constexpr void alertBeenHit() noexcept { mod_.setBeenHit(); }
  void drawBeamAt(Location loc);

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
  StaticPositionArr<TerrainTypeInterface> &getMemoryGrid(FloorSpecifier floor, int width, int height);
};
} // namespace IOModule

// 𐁀
