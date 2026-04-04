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

export struct MonsterBrain {
  constexpr MonsterBrain(MonsterBrainInit init) noexcept : isPlayer_(init.isPlayer), hatesItemPickups_(init.hatesItemPickups) {} // NOLINT(google-explicit-constructor)
  [[nodiscard]] constexpr bool isPlayer() const noexcept { return isPlayer_; }
  [[nodiscard]] constexpr bool hatesItemPickup() const noexcept { return hatesItemPickups_; }
  [[nodiscard]] constexpr bool caresEvent() const noexcept { return isPlayer() || hatesItemPickup(); }
  constexpr void setIsPlayer(bool nV) noexcept { isPlayer_ = nV; }
  constexpr void setHatesItemPickups(bool nV) noexcept { hatesItemPickups_ = nV; }

private:
  bool isPlayer_;
  bool hatesItemPickups_;
};

export constexpr MonsterBrain PlayerBrain(MonsterBrainInit{.isPlayer = true});

enum class MonsterCategories : std::uint8_t {
  Nothing = 0,
  SlugFood = 1,
  Pest = SlugFood << 1,
  Mammal = Pest << 1,
  Humanoid = Mammal << 1,
  Demon = Humanoid << 1,
};

DEFINE_ENUM_BIT_OPS(MonsterCategories)

export class MonsterClassInfo {
public:
  TimePeriod speed = BaseSpeed;
  Health baseHealth = 10;
  Dice::Group damage;
  MonsterBrain brain = MonsterBrainInit{};
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
  constexpr MonsterClassInfo Human{.baseHealth = 10, .damage = "1d6"_dice, .catagories = MonsterCategories::Mammal | MonsterCategories::Humanoid, .prey = MonsterCategories::Pest, .mClass = MonsterClass::Human};
  constexpr MonsterClassInfo Cat{.speed = BaseSpeed * 11 / 12, .baseHealth = 10, .damage = "2d6"_dice, .catagories = MonsterCategories::Mammal, .mClass = MonsterClass::Cat};
  constexpr MonsterClassInfo SeaSlug{.speed = BaseSpeed * 53, .baseHealth = 10, .damage = "10d2"_dice, .catagories = MonsterCategories::Nothing, .prey = MonsterCategories::SlugFood, .mClass = MonsterClass::SeaSlug};
  constexpr MonsterClassInfo GreedyWeasel{.speed = BaseSpeed * 11 / 12, .baseHealth = 10, .damage = "d4"_dice, .brain = MonsterBrainInit{.hatesItemPickups = true}, .catagories = MonsterCategories::Pest, .mClass = MonsterClass::GreedyWeasel};
  constexpr MonsterClassInfo Bryozoan{.speed = TimePeriod(0), .baseHealth = 10, .damage = "0"_dice, .catagories = MonsterCategories::SlugFood, .mClass = MonsterClass::Bryozoan};
  constexpr MonsterClassInfo Imp{.baseHealth = 5, .damage = "1d3+1"_dice, .catagories = MonsterCategories::Demon | MonsterCategories::Pest, .prey = MonsterCategories::Humanoid, .mClass = MonsterClass::Imp};
  return mkEnumToObject<MonsterClassInfo>({Human, Cat, SeaSlug, GreedyWeasel, Bryozoan, Imp});
}();

export [[nodiscard]] constexpr TimePeriod MonsterClassBaseSpeed(MonsterClass mClass) noexcept {
  return MonsterClassInfoArr[mClass].speed;
}

export [[nodiscard]] constexpr bool MonsterClassHunts(MonsterClass hunter, MonsterClass prey) noexcept {
  return (MonsterClassInfoArr[hunter].prey & MonsterClassInfoArr[prey].catagories) != MonsterCategories::Nothing;
}