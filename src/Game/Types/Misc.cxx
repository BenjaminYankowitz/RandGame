module;
#include "../../Common/EnumBitOps.h"
export module GameTypes:Misc;
import :GameTime;
import Common;
import std;
export using Health = int;
export using MP = int;

export enum class TerrainType : std::uint8_t {
  StoneFloor,
  GrassFloor,
  Wall,
  UpStair,
  DownStair,
};

export enum class MonsterClass : std::uint8_t {
  Human,
  Cat,
  SeaSlug,
  GreedyWeasel,
  Bryozoan,
  Imp,
};

export enum class ObjectType : std::uint8_t {
  KingsCoin,
  Knife,
  Die,
  Armor,
  Corpse,
};

export enum class EquipType : std::uint8_t {
  Hand,
  Helm,
  Gloves,
  Ring,
  Body,
  Cloak,
  Shoes,
  CantEquip //Must stay as last value.
};

export [[nodiscard]] constexpr EquipType getEquipType(ObjectType obj){
  switch (obj) {
    using enum ObjectType;
    using enum EquipType;
  case Knife:
    return Hand;
  case Armor:
    return Body;
  default:
    return CantEquip;
  }
}

export enum class Material : std::uint8_t {
  Gold,
  Iron,
  Plastic,
  Wood,
  Flesh
};

export constexpr Material defaultMat(ObjectType type) {
  switch (type) {
    using enum Material;
    using enum ObjectType;
  case KingsCoin:
    return Gold;
  case Knife:
    return Iron;
  case Die:
    return Plastic;
  case Corpse:
    return Flesh;
  case Armor:
    return Iron;
  }
}

export enum class ArtifactId : std::uint8_t {
  Normal,
};

export enum class MoveMode : std::uint8_t {
  None = 0,
  Fight = 1,
  Move = Fight << 1,
  GetWith = Move << 1,
};
export {
  DEFINE_ENUM_BIT_OPS(MoveMode)
}

export class MonsterID {
  using idImpl = unsigned;

public:
  class Generator {
  public:
    constexpr MonsterID next() noexcept { return MonsterID(value_++); }

  private:
    MonsterID::idImpl value_ = MonsterID::NullV + 1;
  };
  [[nodiscard]] constexpr MonsterID() noexcept : id_(0) {}
  [[nodiscard]] constexpr std::size_t hash() const noexcept { return std::hash<idImpl>{}(id_); }
  [[nodiscard]] bool operator==(const MonsterID &o) const noexcept = default;
  [[nodiscard]] static constexpr MonsterID null() noexcept { return MonsterID{NullV}; }
  constexpr void clear() noexcept { id_ = NullV; }
  [[nodiscard]] constexpr explicit operator bool() const noexcept {
    return !isNull();
  }
  [[nodiscard]] constexpr bool isNull() const noexcept { return id_ == NullV; }

private:
  static constexpr idImpl NullV = 0;
  [[nodiscard]] constexpr explicit MonsterID(idImpl id) noexcept : id_(id) {}
  idImpl id_;
};

template <>
struct std::hash<MonsterID> {
  [[nodiscard]] constexpr std::size_t operator()(MonsterID s) const noexcept { return s.hash(); }
};