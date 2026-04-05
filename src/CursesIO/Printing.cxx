export module Printing;
import Common;
import CursesLowLevel;
import GameInterface;
using namespace std::literals;
export namespace IOExceptions {
using IOModuleException = CursesLowLevel::IOExceptions::IOModuleException;
}
using namespace CursesLowLevel;

constexpr std::array Vowels = {'a', 'e', 'i', 'o', 'u', 'y'};

class Word {
public:
  std::string_view word;
  bool usesAn = std::ranges::contains(Vowels, word[0]);
};

class Noun : public Word {
public:
  std::string_view weirdPlural{}; // NOLINT(readability-redundant-member-init)
};

[[nodiscard]] constexpr std::string_view getPluralSuffix(Noun noun) noexcept {
  return noun.word.back() == 's' ? "es" : "s";
}

class Adjective : public Word {};

[[nodiscard]] Adjective
getMatAdj(ObjectInterface obj) noexcept {
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
  case Material::Flesh:
    return {"flesh"};
  }
}

[[nodiscard]] constexpr std::size_t getCount(TerrainType /*terrain*/) noexcept {
  return 1;
}

[[nodiscard]] constexpr Noun getNoun(TerrainType terrain) noexcept {
  switch (terrain) {
  case TerrainType::Empty:
    return {{"empty spot"}};
  case TerrainType::Wall:
    return {{"wall"}};
  case TerrainType::UpStair:
    return {{"staircase up"}};
  case TerrainType::DownStair:
    return {{"staircase down"}};
  }
}

[[nodiscard]] constexpr auto getAdjectives(TerrainType /*terrain*/) noexcept {
  return std::views::empty<Adjective>;
}

[[nodiscard]] constexpr std::string_view specialArticle(TerrainType /*monster*/) noexcept {
  return {};
}

[[nodiscard]] constexpr std::size_t getCount(MonsterInterface /*monster*/) noexcept {
  return 1;
}

[[nodiscard]] constexpr Noun getClassNoun(MonsterClass mClass) noexcept {
  switch (mClass) {
    using enum MonsterClass;
  case Human:
    return {{"human"}};
  case Cat:
    return {{"cat"}};
  case SeaSlug:
    return {{"sea slug"}};
  case GreedyWeasel:
    return {{"greedy weasel"}};
  case Bryozoan:
    return {{"bryozoan"}, "bryozoa"};
  case Imp:
    return {{"imp"}};
  }
}

[[nodiscard]] constexpr Noun getNoun(MonsterInterface monster) noexcept {
  return getClassNoun(monster.getClass());
}

[[nodiscard]] constexpr auto getAdjectives(MonsterInterface /*monster*/) noexcept {
  return std::views::empty<Adjective>;
}

[[nodiscard]] constexpr std::string_view specialArticle(MonsterInterface /*monster*/) noexcept {
  return {};
}

[[nodiscard]] constexpr std::size_t getCount(ObjectInterface obj) noexcept {
  return obj.count();
}

[[nodiscard]] constexpr Noun getNoun(ObjectInterface obj) noexcept {
  switch (obj.type()) {
    using enum ObjectTypeImpl;
  case KingsCoin:
    return {{"coin"}};
  case Knife:
    return {{"knife"}, "knives"};
  case Die:
    return {{"die"}, "dice"};
  case Corpse:
    return {{"corpse"}};
  }
}

[[nodiscard]] constexpr bool printDefaultMat(ObjectType type) noexcept {
  switch (type) {
    using enum ObjectTypeImpl;
  case KingsCoin:
  case Knife:
    return true;
  case Die:
  case Corpse:
    return false;
  }
}

[[nodiscard]] constexpr auto getAdjectives(ObjectInterface obj) noexcept {
  std::vector<Adjective> ret;
  if (printDefaultMat(obj.type()) || defaultMat(obj.type()) != obj.mat()) {
    ret.push_back(getMatAdj(obj));
  }
  if (isCorpse(obj.type())) {
    ret.emplace_back(getClassNoun(corpseOfWhat(obj.type())));
  }
  return ret;
}

[[nodiscard]] constexpr std::string_view specialArticle(ObjectInterface obj) noexcept {
  return obj.artifactStatus() == ArtifactId::Normal ? std::string_view{} : "the";
}

template <class T>
void printThing(std::ostream &out, T thing) {
  auto count = getCount(thing);
  auto adjectives = getAdjectives(thing);
  auto noun = getNoun(thing);
  if (auto article = specialArticle(thing); !article.empty()) {
    out << article;
  } else {
    if (count == 1) {
      bool useAn = adjectives.empty() ? noun.usesAn : adjectives.begin()->usesAn;
      if (useAn) {
        out << "an";
      } else {
        out << 'a';
      }
    } else {
      out << count;
    }
  }
  out << ' ';
  for (auto adjective : adjectives) {
    out << adjective.word << ' ';
  }

  if (count != 1 && !noun.weirdPlural.empty()) {
    out << noun.weirdPlural;
    return;
  }
  out << noun.word;
  if (count != 1) {
    out << ((noun.word.back() == 's') ? "es" : "s");
  }
}

export std::ostream &operator<<(std::ostream &out, const TerrainType type) noexcept {
  printThing(out, type);
  return out;
}

export std::ostream &operator<<(std::ostream &out, const ObjectInterface obj) noexcept {
  printThing(out, obj);
  return out;
}

export std::ostream &operator<<(std::ostream &out, MonsterInterface monster) {
  if (monster.isPlayer()) {
    out << "you";
  } else {
    printThing(out, monster);
  }
  return out;
}

export [[nodiscard]] attr_t toModifierChar(const MonsterInterface &monst) noexcept {
  if (monst.isPlayer()) {
    return Modifier::Standout;
  }
  return Modifier::Normal;
}

export [[nodiscard]] Color toColorChar(MonsterClass mClass) noexcept {
  switch (mClass) {
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
  case Imp:
    return Red;
  }
}

export [[nodiscard]] Color toColorChar(const MonsterInterface &monst) noexcept {
  return toColorChar(monst.getClass());
}

export [[nodiscard]] chtype toDisplayChar(const MonsterInterface &monst) noexcept {
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
  case Imp:
    return 'i';
  }
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
    using enum ObjectTypeImpl;
  case KingsCoin:
    return '$';
  case Knife:
    return ')';
  case Die:
    return '(';
  case ObjectTypeImpl::Corpse:
    return '%';
  }
}

export constexpr Color ObjectToColor(ObjectInterface obj) noexcept {
  switch (obj.mat()) {
    using enum Material;
  case Gold:
    return Yellow;
  case Iron:
    return White;
  case Plastic:
    return BrightWhite;
  case Wood:
    return Brown;
  case Material::Flesh:
    const auto type = obj.type();
    if (type == ObjectType::Corpse) {
      return toColorChar(corpseOfWhat(obj.type()));
    }
    return Red;
  }
}

export Symbol ObjectToSymbol(ObjectInterface obj) noexcept {
  Symbol sym = ObjectTypeToCharacter(obj.type());
  Color c = ObjectToColor(obj);
  sym.setFrontColor(c);
  return sym;
}

export Symbol TerrainTypeInterfaceToSymbol(TerrainTypeInterface c) noexcept {
  switch (c) {
    using enum TerrainTypeInterface;
  case Unknown:
    return ' ';
  case Empty:
    return '.';
  case UpStair:
    return '<';
  case DownStair:
    return '>';
  case CWall:
    return L'▯';
  case HWall:
    return L'─';
  case VWall:
    return L'│';
  case UTWall:
    return L'┬';
  case DTWall:
    return L'┴';
  case LTWall:
    return L'├';
  case RTWall:
    return L'┤';
  case TWall:
    return L'┼';
  case ULCornerWall:
    return L'┌';
  case URCornerWall:
    return L'┐';
  case DLCornerWall:
    return L'└';
  case DRCornerWall:
    return L'┘';
  case SWall:
    return ' ';
  }
}

export Symbol TerrainTypeToSymbol(WorldFloorInterface floor, Position pos) noexcept {
  return TerrainTypeInterfaceToSymbol(floor.getTile(pos).terrainType);
}

export Symbol MemoryTerrainToSymbol(TerrainTypeInterface terrain) noexcept {
  Symbol sym = TerrainTypeInterfaceToSymbol(terrain);
  sym.setFrontColor(Grey);
  return sym;
}

export Symbol TileToSymbol(WorldTileInterface tile) noexcept {
  if (!tile.monster.isNull()) {
    return MonsterToSymbol(tile.monster);
  }
  if (!tile.objects.empty()) {
    return ObjectToSymbol(tile.objects.back());
  }
  return TerrainTypeInterfaceToSymbol(tile.terrainType);
}

export Symbol TileToSymbol(WorldFloorInterface floor, Position pos) noexcept {
  return TileToSymbol(floor.getTile(pos));
}