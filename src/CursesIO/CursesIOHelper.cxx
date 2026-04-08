module;
#include <cassert>
export module CursesIOHelper;
import CursesLowLevel;
import Common;
import GameInterface;
import Printing;

using namespace CursesLowLevel;

export constexpr Dir keyToDir(chtype key) noexcept {
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

export std::array InventLettersArr = std::to_array({std::views::iota('a', 'z' + 1), std::views::iota('A', 'Z' + 1)});
export constexpr auto InventLetters = std::views::join(InventLettersArr);

export auto firstNInvent(std::size_t n) {
  return std::views::zip(std::views::iota(static_cast<std::size_t>(0), n), InventLetters);
}

export void displayInvent(BoxedWindow &window, ObjectContainerInterface items, int sY = 0) {
  window.clear();
  for (auto [y, c] : firstNInvent(std::min<int>(window.prntHeight(), items.size()))) {
    window.moveCursor(0, sY + y);
    window << c << " - " << items[y];
  }
}

export void displayEvents(BoxedWindow &window, const std::span<std::pair<GameTime, std::string>> arr) {
  window.clear();
  const std::size_t printHeight = window.prntHeight();
  const std::size_t offSet = arr.size() < printHeight ? 0 : arr.size() - printHeight;
  for (auto i : std::views::iota(static_cast<std::size_t>(0), std::min(arr.size(), printHeight))) {
    window.moveCursor(0, i);
    window << arr[i + offSet].first.impl << ": " << arr[i + offSet].second;
  }
}

export class ActionMod {
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

export template <class GetTerrain>
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

export std::vector<Position> findUnexploredFrontier(const StaticPositionArr<TerrainTypeInterface> &memory, Position start) {
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

export std::optional<Position> findTerrain(const StaticPositionArr<TerrainTypeInterface> &memory, Position pos, TerrainTypeInterface target) {
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

export enum class StepResult : std::uint8_t { Continue,
                                              Stop };

export [[nodiscard]] consteval std::uint16_t cntrl(char c) {
  assert(c >= '@' && c <= '_');
  return c - 64;
}

export struct ItemFromInterfaceSettings {
  bool doStandAloneDisplay = true;
  bool autoSelectOne = false;
  const std::function<bool(ObjectInterface)> &isEligible = [](ObjectInterface /**/) { return true; };
};

export constexpr std::size_t NoItem = std::numeric_limits<std::size_t>::max();
export constexpr std::size_t NoChoice = std::numeric_limits<std::size_t>::max() - 1;
