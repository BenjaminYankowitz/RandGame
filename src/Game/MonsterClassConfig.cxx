export module MonsterClassConfig;
import Common;
import GameTypes;

constexpr TimePeriod BaseSpeed{60};


struct MonsterBrainInit{
  MustInit<bool> isPlayer =  false;
  MustInit<bool> hatesItemPickups = false;
};

export struct MonsterBrain {
  constexpr MonsterBrain(MonsterBrainInit init) noexcept : isPlayer_(init.isPlayer),hatesItemPickups_(init.hatesItemPickups){} //NOLINT(google-explicit-constructor)
  [[nodiscard]] constexpr bool isPlayer() const noexcept {return isPlayer_;}
  [[nodiscard]] constexpr bool hatesItemPickup() const noexcept {return hatesItemPickups_;}
  [[nodiscard]] constexpr bool caresItemPickup() const noexcept {return isPlayer() || hatesItemPickup();}
  [[nodiscard]] constexpr bool caresMonsterAttack() const noexcept {return isPlayer();}
  [[nodiscard]] constexpr bool caresEvent() const noexcept {return caresItemPickup() ||  caresMonsterAttack();}
  constexpr void setIsPlayer(bool nV) noexcept {isPlayer_ = nV;}
  constexpr void setHatesItemPickups(bool nV) noexcept {hatesItemPickups_ = nV;}
  private:
  bool isPlayer_;
  bool hatesItemPickups_;
};

export constexpr MonsterBrain PlayerBrain(MonsterBrainInit{.isPlayer=true});

export class MonsterClassInfo {
public:
  TimePeriod speed = BaseSpeed;
  Health maxHealth;
  Dice::Group damage;
  MonsterBrain brain = MonsterBrainInit{};
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
  constexpr MonsterClassInfo Human {.maxHealth = 10, .damage = "1d6"_dice, .mClass = MonsterClass::Human};
  constexpr MonsterClassInfo Cat   {.speed = BaseSpeed * 11 / 12, .maxHealth = 10, .damage = "2d6"_dice, .mClass = MonsterClass::Cat};
  constexpr MonsterClassInfo SeaSlug   {.speed = BaseSpeed * 53, .maxHealth = 10, .damage = "10d2"_dice, .mClass = MonsterClass::SeaSlug};
  constexpr MonsterClassInfo GreedyWeasel   {.speed = BaseSpeed * 11 / 12, .maxHealth = 10, .damage = "d4"_dice, .brain=MonsterBrainInit{.hatesItemPickups=true}, .mClass = MonsterClass::GreedyWeasel};
  constexpr MonsterClassInfo Bryozoan   {.speed = TimePeriod(0), .maxHealth = 10, .damage = "0"_dice, .mClass = MonsterClass::Bryozoan};
  return mkEnumToObject<MonsterClassInfo>({Human,Cat,SeaSlug,GreedyWeasel,Bryozoan});
}();


export [[nodiscard]] constexpr TimePeriod MonsterClassBaseSpeed(MonsterClass mClass) noexcept {
  return MonsterClassInfoArr[mClass].speed;
}
