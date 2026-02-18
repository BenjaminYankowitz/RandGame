export module GameTypes:Misc;
import :GameTime;
import std;
export using Health = int;

export enum class TerrainType : std::uint8_t {
  Empty,
  Wall
};

export enum class MonsterClass : std::uint8_t {
  Human,
  Cat,
  SeaSlug,
  GreedyWeasel,
  Bryozoan
};

export enum class ObjectType : std::uint8_t {
  KingsCoin,
  Knife,
  Die
};

export enum class Material : std::uint8_t {
  Gold,
  Iron,
  Plastic,
  Wood
};

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