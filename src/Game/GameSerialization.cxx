module;
module Game;
import Common;
import SerializationLib;

using SerializationLib::deserialize;
using SerializationLib::fromStream;
using SerializationLib::serialize;
using SerializationLib::Tag;
using SerializationLib::toStream;

// --- Monster ---
std::size_t toStream(std::ostream &out, const Monster &input) {
  return input.serializeTo(out);
}

Monster fromStream(std::istream &in, std::size_t &numRead, Tag<Monster> /**/) {
  return Monster::deserializeFrom(in, numRead);
}

// --- WorldFloor ---
std::size_t toStream(std::ostream &out, const WorldFloor &input) {
  std::size_t written = 0;
  written += toStream(out, input.getObjectsArr());
  written += toStream(out, input.getMonsterArr());
  written += toStream(out, input.getTerrainTypeArr());
  written += toStream(out, input.getEventListenersArr());
  return written;
}

WorldFloor fromStream(std::istream &in, std::size_t &numRead, Tag<WorldFloor> /**/) {
  std::size_t totalRead = 0;
  std::size_t localRead;
  auto objects = fromStream(in, localRead, Tag<StaticPositionArr<ObjectContainer>>{});
  totalRead += localRead;
  auto monsters = fromStream(in, localRead, Tag<StaticPositionArr<Monster::ID>>{});
  totalRead += localRead;
  auto terrain = fromStream(in, localRead, Tag<StaticPositionArr<TerrainType>>{});
  totalRead += localRead;

  WorldFloor floor(objects.width(), objects.height());
  floor.getObjectsArr() = std::move(objects);
  floor.getMonsterArr() = std::move(monsters);
  floor.getTerrainTypeArr() = std::move(terrain);
  floor.getEventListenersArr() = fromStream(in, localRead, Tag<std::vector<Monster::ID>>{});
  totalRead += localRead;
  numRead = totalRead;
  return floor;
}

// --- Monster member implementations ---
std::size_t Monster::serializeTo(std::ostream &out) const noexcept {
  std::size_t written = 0;
  written += toStream(out, inventory_);
  written += serialize(out, speed_, loc_, maxHealth_, health_, exp_, snuggleDesire_, id_, next_, prev_);
  written += toStream(out, target_);
  written += serialize(out, damage_, brain_, mClass_, alive_);
  return written;
}

Monster Monster::deserializeFrom(std::istream &in, std::size_t &numRead) {
  std::size_t totalRead = 0;
  std::size_t localRead;

  auto inventory = fromStream(in, localRead, Tag<ObjectContainer>{});
  totalRead += localRead;

  TimePeriod speed(0);
  Location loc(0, 0, 0);
  Health maxHealth{};
  Health health{};
  int exp{};
  int snuggleDesire{};
  ID id;
  ID next;
  ID prev;
  totalRead += deserialize(in, speed, loc, maxHealth, health, exp, snuggleDesire, id, next, prev);

  auto target = fromStream(in, localRead, Tag<std::variant<NoTarget, ID, Location, EatTarget, HangTarget>>{});
  totalRead += localRead;

  Dice::Group damage(0);
  MonsterBrain brain(MonsterBrainInit{});
  MonsterClass mClass{};
  bool alive{};
  totalRead += deserialize(in, damage, brain, mClass, alive);

  MonsterBody body{speed, MustInit<Health>(maxHealth), damage, MustInit<MonsterClass>(mClass), alive};
  Monster m(body, loc, id, brain);
  m.health_ = health;
  m.exp_ = exp;
  m.snuggleDesire_ = snuggleDesire;
  m.next_ = next;
  m.prev_ = prev;
  m.target_ = target;
  m.inventory_ = std::move(inventory);

  numRead = totalRead;
  return m;
}

// --- GameState ---
std::size_t toStream(std::ostream &out, const GameState &input) {
  std::size_t written = 0;
  written += toStream(out, input.monsterMap_);
  written += toStream(out, input.monsterEvents_);
  written += toStream(out, input.floorData_);
  written += serialize(out, input.currentTime_, input.mIdGenerator_, input.player_);
  return written;
}

GameState fromStream(std::istream &in, std::size_t &numRead, Tag<GameState> /**/) {
  std::size_t totalRead = 0;
  std::size_t localRead;

  auto monsterMap = fromStream(in, localRead, Tag<std::unordered_map<Monster::ID, std::unique_ptr<Monster>>>{});
  totalRead += localRead;
  auto monsterEvents = fromStream(in, localRead, Tag<std::priority_queue<GameState::MonsterActionEvent, std::vector<GameState::MonsterActionEvent>, std::greater<>>>{});
  totalRead += localRead;
  auto floorData = fromStream(in, localRead, Tag<std::vector<WorldFloor>>{});
  totalRead += localRead;

  GameTime currentTime;
  Monster::ID::Generator mIdGenerator;
  Monster::ID player;
  totalRead += deserialize(in, currentTime, mIdGenerator, player);

  GameState state;
  state.monsterMap_ = std::move(monsterMap);
  state.monsterEvents_ = std::move(monsterEvents);
  state.floorData_ = std::move(floorData);
  state.currentTime_ = currentTime;
  state.mIdGenerator_ = mIdGenerator;
  state.player_ = player;

  numRead = totalRead;
  return state;
}
