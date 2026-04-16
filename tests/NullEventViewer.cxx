export module NullEventViewer;
import GameState;
import GameInterface;
import Common;

export class NullEventViewer final : public EventViewer {
  void itemPickup(const Monster & /*grabber*/, const Object & /*grabbed*/) noexcept override {}
  void monsterHitMonster(const Monster::HitReturn & /*hitinfo*/, const Monster & /*attacker*/, const Monster & /*attacked*/) noexcept override {}
  void monsterHitWall(const Monster & /*attacker*/, Location /*loc*/) noexcept override {}
  void monsterAte(const Monster & /*eater*/, const Object & /*eaten*/) noexcept override {}
  void debug(std::string_view /*message*/) noexcept override {}
};

export class NullEventViewerInterface final : public EventViewerInterface {
  void itemPickup(MonsterInterface /*grabber*/, ObjectInterface /*grabbed*/) override {}
  void debug(std::string_view /*message*/) override {}
  void monsterHitMonster(HitInfo /*hitinfo*/, MonsterInterface /*attacker*/, MonsterInterface /*attacked*/) override {}
  void monsterHitWall(MonsterInterface /*attacker*/, TerrainTypeInterface /*attacked*/) override {}
  void exception(const std::exception & /*e*/) noexcept override {}
  void monsterAte(MonsterInterface /*eater*/, ObjectInterface /*eaten*/) override {}
};

export std::unique_ptr<EventViewerInterface> makeNullViewer() {
  return std::make_unique<NullEventViewerInterface>();
}
