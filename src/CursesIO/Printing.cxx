export module Printing;
import Common;
import CursesLowLevel;
import GameInterface;
using namespace std::literals;
export namespace IOExceptions {
using IOModuleException = CursesLowLevel::IOExceptions::IOModuleException;
}
using namespace CursesLowLevel;

export attr_t toModifierChar(const MonsterInterface &monst) noexcept {
  if (monst.isPlayer()) {
    return Modifier::Standout;
  }
  return Modifier::Normal;
}

export Color toColorChar(const MonsterInterface &monst) noexcept {
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

export chtype toDisplayChar(const MonsterInterface &monst) noexcept {
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

export class Word {
public:
  std::string_view word;
  bool weirdAn = false;
};

export class Noun : public Word {
public:
  std::string_view weirdPlural{}; // NOLINT(readability-redundant-member-init)
};

export class Adjective : public Word {};

export [[nodiscard]] constexpr bool usesAn(Word word) noexcept {
  return word.weirdAn != std::ranges::contains(Vowels, word.word[0]);
}

export class PrintableObject {
  using DescriptorsType = std::array<Adjective, 4>;

public:
  constexpr explicit PrintableObject(Noun name, std::size_t count = 1) noexcept : name_{name}, count_{count} {}
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
    auto frontWord = numDescriptors_ == 0 ? usesAn(name_) : usesAn(descriptors_[0]);
    return frontWord ? "an" : "a";
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

export std::ostream &operator<<(std::ostream &out, const PrintableObject &obj) noexcept {
  if (obj.getCount() == 1) {
    out << obj.getSingularPrefix();
  } else {
    out << obj.getCount();
  }
  out << ' ';
  for (const auto &word : obj.getDescriptors()) {
    out << word.word << ' ';
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
  out << word << plural;
  return out;
}

export [[nodiscard]] Noun toName(ObjectInterface obj) noexcept {
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

export [[nodiscard]] Adjective getMatAdj(ObjectInterface obj) noexcept {
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

export [[nodiscard]] Noun toName(TerrainType terrain) {
  switch (terrain) {
  case TerrainType::Empty:
    return {{"empty spot"}};
  case TerrainType::Wall:
    return {{"wall"}};
  }
}

export [[nodiscard]] PrintableObject toPrintAbleObject(TerrainType terrain) noexcept {
  const Noun ObjName = toName(terrain);
  PrintableObject printer(ObjName);
  return printer;
}

export [[nodiscard]] PrintableObject toPrintAbleObject(ObjectInterface obj) noexcept {
  const Noun ObjName = toName(obj);
  const Adjective matDescriptor = getMatAdj(obj);
  PrintableObject printer(ObjName, obj.count());
  printer.addDescriptor(matDescriptor);
  if (obj.artifactStatus() != ArtifactId::Normal) {
    printer.setUseThe();
  }
  return printer;
}
export std::ostream &operator<<(std::ostream &str, const ObjectInterface &obj) noexcept {
  return str << toPrintAbleObject(obj);
}
export Symbol MonsterToSymbol(MonsterInterface monst) noexcept {
  Symbol sym = toDisplayChar(monst);
  sym.addModifier(toModifierChar(monst));
  sym.setFrontColor(toColorChar(monst));
  sym.setBackColor(Black);
  return sym;
}

export constexpr chtype ObjectTypeToCharacter(ObjectType otype) noexcept {
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

export constexpr Color ObjectMaterialToColor(Material otype) noexcept {
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

export Symbol ObjectToSymbol(ObjectInterface obj) noexcept {
  Symbol sym = ObjectTypeToCharacter(obj.type());
  Color c = ObjectMaterialToColor(obj.mat());
  sym.setFrontColor(c);
  return sym;
}

export Symbol TerrainTypeToSymbol(WorldFloorInterface floor, Position pos) noexcept {
  const auto tile = floor.getTile(pos);
  const auto c = tile.terrainType;
  switch (c) {
    using enum TerrainType;
  case Empty:
    return '.';
  case Wall:
    auto getType = [floor](Position pos) {
      return floor.inBounds(pos) && floor.getTile(pos).terrainType == Wall;
    };
    auto check = [&getType, pos](Dir dir) {
      if (!getType(pos + dir)) {
        return false;
      }
      auto [dx, dy] = dir;
      Dir oDir(dy, dx);
      // return true;
      return !(getType(pos + oDir) && getType(pos - oDir) && getType(pos + dir + oDir) && getType(pos + dir - oDir));
    };
    using enum SpecialChar::Directions;
    SpecialChar::Directions dir = None;
    if (check(Dir::up()))
      dir |= Up;
    if (check(Dir::down()))
      dir |= Down;
    if (check(Dir::left()))
      dir |= Left;
    if (check(Dir::right()))
      dir |= Right;
    if (dir == None && getType(pos.up())) {
      return ' ';
    }
    return SpecialChar::Walls[dir];
  }
}

export Symbol TileToSymbol(WorldFloorInterface floor, Position pos) noexcept {
  const auto tile = floor.getTile(pos);
  auto monstPtr = tile.monster;
  if (!monstPtr.isNull()) {
    return MonsterToSymbol(monstPtr);
  }
  if (!tile.objects.empty()) {
    return ObjectToSymbol(tile.objects.back());
  }
  return TerrainTypeToSymbol(floor, pos);
}

export std::string_view getName(MonsterInterface monster) noexcept {
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

export std::ostream &operator<<(std::ostream &out, MonsterInterface monster) {
  if (monster.isPlayer()) {
    out << "you";
  } else {
    out << "a " << getName(monster);
  }
  return out;
}
