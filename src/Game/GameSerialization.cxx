module;
module Game;
import Common;
import SerializationLib;

using SerializationLib::deserialize;
using SerializationLib::fromStream;
using SerializationLib::serialize;
using SerializationLib::Tag;
using SerializationLib::toStream; // NOLINT(misc-unused-using-decls) //This is nessesary for serialize to work.

// --- Monster ---
std::size_t toStream(std::ostream &out, const Monster &input) {
  return input.serializeTo(out);
}

Monster fromStream(std::istream &in, std::size_t &numRead, Tag<Monster> /**/) {
  return Monster::deserializeFrom(in, numRead);
}

// --- WorldFloor ---
std::size_t toStream(std::ostream &out, const WorldFloor &input) {
  return serialize(out, input.getObjectsArr(), input.getMonsterArr(), input.getTerrainTypeArr(), input.getEventListenersArr());
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
  return serialize(out, inventory_, body_, brain_, loc_, exp_, id_, next_, prev_);
}

Monster Monster::deserializeFrom(std::istream &in, std::size_t &numRead) {
  std::size_t totalRead = 0;
  std::size_t localRead;

  auto inventory = fromStream(in, localRead, Tag<ObjectContainer>{});
  totalRead += localRead;

  auto body = fromStream(in, localRead, Tag<Monster::MonsterBody>{});
  totalRead += localRead;

  auto brain = fromStream(in, localRead, Tag<Monster::MonsterBrain>{});
  totalRead += localRead;
  Location loc = fromStream(in, localRead, Tag<Location>{});
  totalRead += localRead;
  int exp;
  Monster::ID id;
  Monster::ID next;
  Monster::ID prev;
  totalRead += deserialize(in, exp, id, next, prev);
  Monster m(body, loc, id, brain);
  m.inventory_ = std::move(inventory);
  m.next_ = next;
  m.prev_ = prev;
  m.exp_ = exp;
  numRead = totalRead;
  return m;
}

// --- GameState ---
std::size_t toStream(std::ostream &out, const GameState &input) {
  return serialize(out, input.monsterMap_, input.monsterEvents_, input.floorData_, input.currentTime_, input.mIdGenerator_, input.player_);
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
