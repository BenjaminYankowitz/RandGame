export module MonsterClassConfig;
import Common;
import GameTypes;

constexpr TimePeriod BaseSpeed{60};


export struct MonsterBrainInit{
  bool isPlayer =  false;
  bool hatesItemPickups = false;
};

export struct MonsterBrain {
  constexpr MonsterBrain(MonsterBrainInit init) noexcept : isPlayer_(init.isPlayer),hatesItemPickups_(init.hatesItemPickups){} //NOLINT(google-explicit-constructor)
  [[nodiscard]] constexpr bool isPlayer() const noexcept {return isPlayer_;}
  [[nodiscard]] constexpr bool hatesItemPickup() const noexcept {return hatesItemPickups_;}
  [[nodiscard]] constexpr bool caresEvent() const noexcept {return isPlayer() ||  hatesItemPickup();}
  constexpr void setIsPlayer(bool nV) noexcept {isPlayer_ = nV;}
  constexpr void setHatesItemPickups(bool nV) noexcept {hatesItemPickups_ = nV;}
  private:
  bool isPlayer_;
  bool hatesItemPickups_;
};

export constexpr MonsterBrain PlayerBrain(MonsterBrainInit{.isPlayer=true});

enum class MonsterCatagories :std::uint8_t {
  Nothing = 0,
  SlugFood = 1,
  Pest = SlugFood<<1,
  Mammal = Pest<<1,
  Humanoid = Mammal<<1,
};

[[nodiscard]] constexpr MonsterCatagories operator^(MonsterCatagories lhs, MonsterCatagories rhs) noexcept {
  return static_cast<MonsterCatagories>(std::to_underlying(lhs) ^ std::to_underlying(rhs));
}
[[nodiscard]] constexpr MonsterCatagories operator|(MonsterCatagories lhs, MonsterCatagories rhs) noexcept {
  return static_cast<MonsterCatagories>(std::to_underlying(lhs) | std::to_underlying(rhs));
}
[[nodiscard]] constexpr MonsterCatagories operator&(MonsterCatagories lhs, MonsterCatagories rhs) noexcept {
  return static_cast<MonsterCatagories>(std::to_underlying(lhs) & std::to_underlying(rhs));
}
[[nodiscard]] constexpr MonsterCatagories operator~(MonsterCatagories v) noexcept {
  return static_cast<MonsterCatagories>(~std::to_underlying(v));
}
constexpr MonsterCatagories &operator^=(MonsterCatagories &lhs, MonsterCatagories rhs) noexcept {
  return lhs = lhs ^ rhs;
}
constexpr MonsterCatagories &operator|=(MonsterCatagories &lhs, MonsterCatagories rhs) noexcept {
  return lhs = lhs | rhs;
}
constexpr MonsterCatagories &operator&=(MonsterCatagories &lhs, MonsterCatagories rhs) noexcept {
  return lhs = lhs & rhs;
}

export class MonsterClassInfo {
public:
  TimePeriod speed = BaseSpeed;
  Health maxHealth = 10;
  Dice::Group damage;
  MonsterBrain brain = MonsterBrainInit{};
  MonsterCatagories catagories;
  MonsterCatagories prey = MonsterCatagories::Nothing;
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
  constexpr MonsterClassInfo Human {.maxHealth = 10, .damage = "1d6"_dice, .catagories=MonsterCatagories::Mammal|MonsterCatagories::Humanoid,.mClass = MonsterClass::Human};
  constexpr MonsterClassInfo Cat   {.speed = BaseSpeed * 11 / 12, .maxHealth = 10, .damage = "2d6"_dice, .catagories=MonsterCatagories::Mammal,.mClass = MonsterClass::Cat};
  constexpr MonsterClassInfo SeaSlug   {.speed = BaseSpeed * 53, .maxHealth = 10, .damage = "10d2"_dice, .catagories=MonsterCatagories::Nothing,.prey=MonsterCatagories::SlugFood,.mClass = MonsterClass::SeaSlug};
  constexpr MonsterClassInfo GreedyWeasel{.speed = BaseSpeed * 11 / 12, .maxHealth = 10, .damage = "d4"_dice, .brain = MonsterBrainInit{.hatesItemPickups = true}, .catagories = MonsterCatagories::Pest, .mClass = MonsterClass::GreedyWeasel};
  constexpr MonsterClassInfo Bryozoan   {.speed = TimePeriod(0), .maxHealth = 10, .damage = "0"_dice, .catagories=MonsterCatagories::SlugFood, .mClass = MonsterClass::Bryozoan};
  return mkEnumToObject<MonsterClassInfo>({Human,Cat,SeaSlug,GreedyWeasel,Bryozoan});
}();


export [[nodiscard]] constexpr TimePeriod MonsterClassBaseSpeed(MonsterClass mClass) noexcept {
  return MonsterClassInfoArr[mClass].speed;
}


export [[nodiscard]] constexpr bool MonsterClassHunts(MonsterClass hunter, MonsterClass prey) noexcept {
  return (MonsterClassInfoArr[hunter].prey & MonsterClassInfoArr[prey].catagories) != MonsterCatagories::Nothing;
}