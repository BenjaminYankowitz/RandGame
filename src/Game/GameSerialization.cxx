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
  written += serialize(out, body_);
  written += serialize(out, loc_, exp_, snuggleDesire_, id_, next_, prev_);
  written += toStream(out, target_);
  written += serialize(out, brain_);
  return written;
}

Monster Monster::deserializeFrom(std::istream &in, std::size_t &numRead) {
  std::size_t totalRead = 0;
  std::size_t localRead;

  auto inventory = fromStream(in, localRead, Tag<ObjectContainer>{});
  totalRead += localRead;

  auto body = fromStream(in, localRead, Tag<Monster::MonsterBody>{});
  totalRead += localRead;

  Location loc(0, 0, 0);
  int exp{};
  int snuggleDesire{};
  ID id;
  ID next;
  ID prev;
  totalRead += deserialize(in, loc, exp, snuggleDesire, id, next, prev);

  auto target = fromStream(in, localRead, Tag<std::variant<NoTarget, ID, Location, EatTarget, HangTarget>>{});
  totalRead += localRead;

  MonsterBrain brain(MonsterBrainInit{});
  totalRead += deserialize(in, brain);

  MonsterBodyInit init{body.speed_, MustInit<Health>(body.maxHealth_), body.damage_, MustInit<MonsterClass>(body.mClass_), body.alive_};
  Monster m(init, loc, id, brain);
  m.body_ = body;
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
