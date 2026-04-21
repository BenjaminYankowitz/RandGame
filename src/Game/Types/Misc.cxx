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

export enum class ObjectTypeImpl : std::uint8_t { //NOLINT(readability-enum-initial-value)
  KingsCoin,
  Knife,
  Die,
  Armor,
  Corpse = 1 << (std::numeric_limits<std::uint8_t>::digits - 1),
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

[[nodiscard]] constexpr bool isCorpse(ObjectTypeImpl obj) {
  return (std::to_underlying(obj) & std::to_underlying(ObjectTypeImpl::Corpse)) != 0;
}

export struct ObjectType {
  using enum ObjectTypeImpl;
  constexpr ObjectType(ObjectTypeImpl val) noexcept : type_(val) {} // NOLINT(google-explicit-constructor)
  constexpr operator ObjectTypeImpl() const noexcept {              // NOLINT(google-explicit-constructor)
    return isCorpse(type_) ? Corpse : type_;
  }
  [[nodiscard]] constexpr bool operator==(const ObjectType &other) const = default;
  [[nodiscard]] constexpr bool operator==(ObjectTypeImpl other) const noexcept { return (operator ObjectTypeImpl()) == other; };

private:
  friend constexpr MonsterClass corpseOfWhat(ObjectType obj);
  ObjectTypeImpl type_;
};

export [[nodiscard]] constexpr EquipType getEquipType(ObjectType obj){
  switch (obj) {
    using enum ObjectTypeImpl;
    using enum EquipType;
  case Knife:
    return Hand;
  case Armor:
    return Body;
  default:
    return CantEquip;
  }
}

export [[nodiscard]] constexpr bool isCorpse(ObjectType obj) {
  return obj == ObjectType::Corpse;
}

export [[nodiscard]] constexpr ObjectType corpseOf(MonsterClass monst) {
  return static_cast<ObjectTypeImpl>(std::to_underlying(monst) | std::to_underlying(ObjectType::Corpse));
}

export [[nodiscard]] constexpr MonsterClass corpseOfWhat(ObjectType obj) {
  if (!isCorpse(obj)) {
    Logging::log << "Tried to get corpse of non corpse object\n";
  }
  return static_cast<MonsterClass>(std::to_underlying(obj.type_) & ~std::to_underlying(ObjectType::Corpse));
}

export enum class Material : std::uint8_t {
  Gold,
  Iron,
  Plastic,
  Wood,
  Flesh
};

export constexpr Material defaultMat(ObjectType type) {
  switch (ObjectTypeImpl(type)) {
    using enum Material;
    using enum ObjectTypeImpl;
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