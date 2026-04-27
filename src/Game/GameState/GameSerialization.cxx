module GameState;
import Common;
import SerializationLib;

using SerializationLib::deserialize;
using SerializationLib::fromStream;
using SerializationLib::serialize;
using SerializationLib::Tag;
using SerializationLib::toStream; // NOLINT(misc-unused-using-decls) //This is nessesary for serialize to work.

// --- Monster ---
void toStream(std::ostream &out, const Monster &input) {
  input.serializeTo(out);
}

Monster fromStream(std::istream &in, Tag<Monster> /**/) {
  return Monster::deserializeFrom(in);
}

// --- WorldFloor ---
void toStream(std::ostream &out, const WorldFloor &input) {
  serialize(out, input.getObjectsArr(), input.getMonsterArr(), input.getTerrainTypeArr(), input.getEventListenersArr());
}

WorldFloor fromStream(std::istream &in, Tag<WorldFloor> /**/) {
  auto objects = fromStream(in, Tag<StaticPositionArr<ObjectContainer>>{});
  WorldFloor floor(objects.width(), objects.height());
  floor.getObjectsArr() = std::move(objects);
  floor.getMonsterArr() = fromStream(in, Tag<StaticPositionArr<Monster::ID>>{});
  floor.getTerrainTypeArr() = fromStream(in, Tag<StaticPositionArr<TerrainType>>{});
  floor.getEventListenersArr() = fromStream(in, Tag<std::vector<Monster::ID>>{});
  return floor;
}

// --- Monster member implementations ---
void Monster::serializeTo(std::ostream &out) const noexcept {
  serialize(out, body_,loc_,id_,brain_,inventory_,exp_,next_,prev_);
  //inventory_, body_, brain_, loc_, exp_, id_, next_, prev_
  for (std::int8_t i = 0; i < body_.plan.totalSlots(); ++i) {
    toStream(out, equipment_[i]);
  }
}

Monster Monster::deserializeFrom(std::istream &in) {
  auto body = fromStream(in, Tag<MonsterBody>{});
  auto loc = fromStream(in, Tag<Location>{});
  auto id = fromStream(in, Tag<ID>{});
  auto brain = fromStream(in, Tag<MonsterBrain>{});
  Monster m(body,loc,id,brain);
  deserialize(in, m.inventory_, m.exp_, m.next_, m.prev_);
  for (std::int8_t i = 0; i < m.body_.plan.totalSlots(); ++i) {
    m.equipment_[i] = fromStream(in, Tag<std::unique_ptr<Object>>{});
  }
  return m;
}

// --- GameState ---
void toStream(std::ostream &out, const GameState &input) {
  serialize(out, input.monsterMap_, input.monsterEvents_, input.floorData_, input.currentTime_, input.mIdGenerator_, input.player_);
}

GameState fromStream(std::istream &in, Tag<GameState> /**/) {
  GameState state;
  deserialize(in,state.monsterMap_,state.monsterEvents_,state.floorData_,state.currentTime_,state.mIdGenerator_,state.player_);
  return state;
}
