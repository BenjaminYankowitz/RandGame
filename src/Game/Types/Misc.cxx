export module GameTypes:Misc;
import :GameTime;
import Common;
import std;
export using Health = int;

export enum class TerrainType : std::uint8_t {
  Empty,
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
};

export enum class ObjectTypeImpl : std::uint8_t {
  KingsCoin,
  Knife,
  Die,
  Corpse = 1<<(std::numeric_limits<std::uint8_t>::digits-1),
};

[[nodiscard]] constexpr bool isCorpse(ObjectTypeImpl obj){
  return (std::to_underlying(obj) & std::to_underlying(ObjectTypeImpl::Corpse)) != 0;
}

export struct ObjectType {
  using enum ObjectTypeImpl;
  constexpr ObjectType(ObjectTypeImpl val) noexcept : type_(val){} //NOLINT(google-explicit-constructor)
  constexpr operator ObjectTypeImpl() const noexcept { //NOLINT(google-explicit-constructor)
    return isCorpse(type_) ? Corpse : type_;
  }
  [[nodiscard]] constexpr bool operator==(const ObjectType& other) const = default;
  [[nodiscard]] constexpr bool operator==(ObjectTypeImpl other) const noexcept {return (operator ObjectTypeImpl()) == other;};
  private:
  friend constexpr MonsterClass corpseOfWhat(ObjectType obj);
  ObjectTypeImpl type_;
};

export [[nodiscard]]  constexpr bool isCorpse(ObjectType obj){
  return obj == ObjectType::Corpse;
}

export [[nodiscard]] constexpr ObjectType corpseOf(MonsterClass monst){
  return static_cast<ObjectTypeImpl>(std::to_underlying(monst)|std::to_underlying(ObjectType::Corpse));
}

export [[nodiscard]] constexpr MonsterClass corpseOfWhat(ObjectType obj){
  if(!isCorpse(obj)){
    Logging::log << "Tried to get corpse of non corpse object\n";
  }
  return static_cast<MonsterClass>(std::to_underlying(obj.type_)&~std::to_underlying(ObjectType::Corpse));
}

export enum class Material : std::uint8_t {
  Gold,
  Iron,
  Plastic,
  Wood,
  Flesh
};

export constexpr Material defaultMat(ObjectType type){
  switch (type) {
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
  }
}

export enum class ArtifactId : std::uint8_t {
  Normal,
};

export class MoveMode {
  using ImplT = std::uint8_t;

public:
  [[nodiscard]] static constexpr MoveMode fight() noexcept { return MoveMode(Fight); };
  [[nodiscard]] static constexpr MoveMode move() noexcept { return MoveMode(Move); };
  [[nodiscard]] constexpr bool isMove() const noexcept { return (impl_ & Move) != 0; };
  [[nodiscard]] constexpr bool isFight() const noexcept { return (impl_ & Fight) != 0; };
  constexpr MoveMode &operator^=(MoveMode o) noexcept {
    impl_ ^= o.impl_;
    return *this;
  }
  constexpr MoveMode &operator|=(MoveMode o) noexcept {
    impl_ |= o.impl_;
    return *this;
  }
  constexpr MoveMode &operator&=(MoveMode o) noexcept {
    impl_ &= o.impl_;
    return *this;
  }
  constexpr MoveMode operator^(MoveMode o) const noexcept {
    auto cp = *this;
    return cp ^= o;
  }
  constexpr MoveMode operator|(MoveMode o) const noexcept {
    auto cp = *this;
    return cp |= o;
  }
  constexpr MoveMode operator&(MoveMode o) const noexcept {
    auto cp = *this;
    return cp &= o;
  }
  constexpr MoveMode operator~() const noexcept { return MoveMode(~impl_); }

private:
  constexpr explicit MoveMode(ImplT v) noexcept : impl_(v) {}
  ImplT impl_;
  constexpr static ImplT Fight = 1;
  constexpr static ImplT Move = Fight << 1;
};

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