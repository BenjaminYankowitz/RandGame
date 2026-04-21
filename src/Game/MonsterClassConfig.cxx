module;
#include "../Common/EnumBitOps.h"
export module MonsterClassConfig;
import Common;
import GameTypes;

constexpr TimePeriod BaseSpeed{60};

export struct MonsterBrainInit {
  bool isPlayer = false;
  bool hatesItemPickups = false;
};

export struct MonsterBrainConfig {
  constexpr MonsterBrainConfig(MonsterBrainInit init) noexcept : isPlayer_(init.isPlayer), hatesItemPickups_(init.hatesItemPickups) {} // NOLINT(google-explicit-constructor)
  [[nodiscard]] constexpr bool isPlayer() const noexcept { return isPlayer_; }
  [[nodiscard]] constexpr bool hatesItemPickup() const noexcept { return hatesItemPickups_; }
  [[nodiscard]] constexpr bool caresEvent() const noexcept { return isPlayer() || hatesItemPickup(); }
  constexpr void setIsPlayer(bool nV) noexcept { isPlayer_ = nV; }
  constexpr void setHatesItemPickups(bool nV) noexcept { hatesItemPickups_ = nV; }

private:
  bool isPlayer_;
  bool hatesItemPickups_;
};

export constexpr MonsterBrainConfig PlayerBrain(MonsterBrainInit{.isPlayer = true});

enum class MonsterCategories : std::uint8_t {
  Nothing = 0,
  SlugFood = 1,
  Pest = SlugFood << 1,
  Mammal = Pest << 1,
  Humanoid = Mammal << 1,
  Demon = Humanoid << 1,
};

DEFINE_ENUM_BIT_OPS(MonsterCategories)

export class BodyPlan {
public:
  consteval BodyPlan(std::initializer_list<std::pair<EquipType, std::int8_t>> list) {
    slots_.fill(0);
    for (auto [type, count] : list) {
      const auto index = std::to_underlying(type);
      if (slots_[index] != 0 || count < 0)
        std::unreachable();
      slots_[index] = count;
    }
    for (auto i : std::views::iota(static_cast<std::size_t>(1),slots_.size())) {
      slots_[i] += slots_[i - 1];
    }
  }
  [[nodiscard]] constexpr std::pair<std::int8_t, std::int8_t> gSlots(EquipType type) const noexcept {
    std::uint8_t index = std::to_underlying(type);
    std::int8_t prev = 0;
    if (index != 0)
      prev = slots_[index - 1];
    return {prev, static_cast<std::int8_t>(slots_[index] - prev)};
  };
  [[nodiscard]] constexpr std::int8_t gNSlots(EquipType type) const noexcept {
    return gSlots(type).second;
  };
  [[nodiscard]] constexpr std::int8_t totalSlots() const noexcept {
    return slots_.back();
  }

private:
  std::array<std::int8_t, std::to_underlying(EquipType::CantEquip)+1> slots_{};
};
using enum EquipType;
constexpr BodyPlan HumaniodBody = {{Hand, 2}, {Helm, 1}, {Gloves, 1}, {Ring, 2}, {Body, 1}, {Cloak, 1}, {Shoes, 1}};

export class MonsterClassInfo {
public:
  TimePeriod speed = BaseSpeed;
  BodyPlan bodyPlan = HumaniodBody;
  Health baseHealth = 10;
  MP baseMP = 0;
  Dice::Group naturalWeapon;
  MonsterBrainConfig brain = MonsterBrainInit{};
  int maxThrowingDistance = 10;
  MonsterCategories catagories;
  MonsterCategories prey = MonsterCategories::Nothing;
  MonsterClass mClass;
};

template <>
class GetEnumValue<MonsterClassInfo> {
public:
  static constexpr MonsterClass get(const MonsterClassInfo &mInfo) noexcept {
    return mInfo.mClass;
  }
};

export constexpr auto MonsterClassInfoArr = []() {
  using namespace Dice::Literals;
  constexpr MonsterClassInfo Human{.baseMP = 10, .naturalWeapon = "1d6"_dice, .catagories = MonsterCategories::Mammal | MonsterCategories::Humanoid, .prey = MonsterCategories::Pest, .mClass = MonsterClass::Human};
  constexpr MonsterClassInfo Cat{.speed = BaseSpeed * 11 / 12, .naturalWeapon = "2d6"_dice, .catagories = MonsterCategories::Mammal, .mClass = MonsterClass::Cat};
  constexpr MonsterClassInfo SeaSlug{.speed = BaseSpeed * 53, .naturalWeapon = "10d2"_dice, .catagories = MonsterCategories::Nothing, .prey = MonsterCategories::SlugFood, .mClass = MonsterClass::SeaSlug};
  constexpr MonsterClassInfo GreedyWeasel{.speed = BaseSpeed * 11 / 12, .naturalWeapon = "d4"_dice, .brain = MonsterBrainInit{.hatesItemPickups = true}, .catagories = MonsterCategories::Pest, .mClass = MonsterClass::GreedyWeasel};
  constexpr MonsterClassInfo Bryozoan{.speed = TimePeriod(0), .naturalWeapon = "0"_dice, .catagories = MonsterCategories::SlugFood, .mClass = MonsterClass::Bryozoan};
  constexpr MonsterClassInfo Imp{.naturalWeapon = "1d3+1"_dice, .catagories = MonsterCategories::Demon | MonsterCategories::Pest, .prey = MonsterCategories::Humanoid, .mClass = MonsterClass::Imp};
  return mkEnumToObject<MonsterClassInfo>({Human, Cat, SeaSlug, GreedyWeasel, Bryozoan, Imp});
}();

export [[nodiscard]] constexpr TimePeriod MonsterClassBaseSpeed(MonsterClass mClass) noexcept {
  return MonsterClassInfoArr[mClass].speed;
}

export [[nodiscard]] constexpr bool MonsterClassHunts(MonsterClass hunter, MonsterClass prey) noexcept {
  return hasOverlap(MonsterClassInfoArr[hunter].prey, MonsterClassInfoArr[prey].catagories);
}