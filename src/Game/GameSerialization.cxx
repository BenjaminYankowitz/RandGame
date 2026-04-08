module;
module Game;
import Common;
import SerializationLib;

using SerializationLib::fromStream;
using SerializationLib::Tag;
using SerializationLib::toStream;
using SerializationLib::serialize;
using SerializationLib::deserialize;
template<class T>
concept EmptyClass = std::is_empty_v<T>;

template <class Variant, std::size_t... Is>
Variant fromStreamVariantHelper(std::istream &in, std::size_t &numRead, std::size_t index, std::index_sequence<Is...> /*unused*/) {
  using reader_t = Variant (*)(std::istream &, std::size_t &);
  static constexpr std::array Readers = std::to_array<reader_t>({[](std::istream &in_, std::size_t &nr) -> Variant {
    return Variant(std::in_place_index<Is>, fromStream(in_, nr, Tag<std::variant_alternative_t<Is, Variant>>{}));
  }...});
  std::size_t localRead = 0;
  auto result = Readers[index](in, localRead);
  numRead += localRead;
  return result;
}

template <class T, class Container, EmptyClass Compare>
const Container &pqContainer(const std::priority_queue<T, Container, Compare> &pq) {
  struct Accessor : std::priority_queue<T, Container, Compare> {
    static const Container &get(const std::priority_queue<T, Container, Compare> &q) { return q.*&Accessor::c; }
  };
  return Accessor::get(pq);
}

// --- std::unique_ptr ---
template <class T, EmptyClass Dealoc>
std::size_t toStream(std::ostream &out, const std::unique_ptr<T,Dealoc> &input) {
  bool hasValue = input != nullptr;
  std::size_t written = toStream(out, hasValue);
  if (hasValue) {
    written += toStream(out, *input);
  }
  return written;
}

template <class T>
std::unique_ptr<T> fromStream(std::istream &in, std::size_t &numRead, Tag<std::unique_ptr<T>> /**/) {
  std::size_t totalRead = 0;
  std::size_t localRead;
  auto hasValue = fromStream(in, localRead, Tag<bool>{});
  totalRead += localRead;
  if (hasValue) {
    auto val = fromStream(in, localRead, Tag<T>{});
    totalRead += localRead;
    numRead = totalRead;
    return std::make_unique<T>(std::move(val));
  }
  numRead = totalRead;
  return nullptr;
}

// --- std::variant ---
template <class... Ts>
std::size_t toStream(std::ostream &out, const std::variant<Ts...> &input) {
  std::size_t written = toStream(out, input.index());
  std::visit([&](const auto &val) { written += toStream(out, val); }, input);
  return written;
}

template <class... Ts>
std::variant<Ts...> fromStream(std::istream &in, std::size_t &numRead, Tag<std::variant<Ts...>> /**/) {
  std::size_t localRead;
  auto index = fromStream(in, localRead, Tag<std::size_t>{});
  numRead = localRead;
  return fromStreamVariantHelper<std::variant<Ts...>>(in, numRead, index, std::index_sequence_for<Ts...>{});
}

// --- std::vector ---
template <class T>
std::size_t toStream(std::ostream &out, const std::vector<T> &input) {
  std::size_t written = toStream(out, input.size());
  for (const auto &elem : input) {
    written += toStream(out, elem);
  }
  return written;
}

template <class T>
std::vector<T> fromStream(std::istream &in, std::size_t &numRead, Tag<std::vector<T>> /**/) {
  std::size_t totalRead = 0;
  std::size_t localRead;
  auto size = fromStream(in, localRead, Tag<std::size_t>{});
  totalRead += localRead;
  std::vector<T> vec;
  vec.reserve(size);
  for (std::size_t i = 0; i < size; ++i) {
    vec.push_back(fromStream(in, localRead, Tag<T>{}));
    totalRead += localRead;
  }
  numRead = totalRead;
  return vec;
}

// --- std::unordered_map ---
template <class K, class V>
std::size_t toStream(std::ostream &out, const std::unordered_map<K, V> &input) {
  std::size_t written = toStream(out, input.size());
  for (const auto &[key, val] : input) {
    written += toStream(out, key);
    written += toStream(out, val);
  }
  return written;
}

template <class K, class V>
std::unordered_map<K, V> fromStream(std::istream &in, std::size_t &numRead, Tag<std::unordered_map<K, V>> /**/) {
  std::size_t totalRead = 0;
  std::size_t localRead;
  auto size = fromStream(in, localRead, Tag<std::size_t>{});
  totalRead += localRead;
  std::unordered_map<K, V> map;
  map.reserve(size);
  for (std::size_t i = 0; i < size; ++i) {
    auto key = fromStream(in, localRead, Tag<K>{});
    totalRead += localRead;
    auto val = fromStream(in, localRead, Tag<V>{});
    totalRead += localRead;
    map.emplace(std::move(key), std::move(val));
  }
  numRead = totalRead;
  return map;
}

// --- std::priority_queue ---
template <class T, class Container, EmptyClass Compare>
std::size_t toStream(std::ostream &out, const std::priority_queue<T, Container, Compare> &input) {
  return toStream(out, pqContainer(input));
}

template <class T, class Container, EmptyClass Compare>
std::priority_queue<T, Container, Compare> fromStream(std::istream &in, std::size_t &numRead, Tag<std::priority_queue<T, Container, Compare>> /**/) {
  auto container = fromStream(in, numRead, Tag<Container>{});
  return std::priority_queue<T, Container, Compare>(Compare{}, std::move(container));
}

// --- ObjectContainer ---
std::size_t toStream(std::ostream &out, const ObjectContainer &input) {
  return toStream(out, input.rawImpl());
}

ObjectContainer fromStream(std::istream &in, std::size_t &numRead, Tag<ObjectContainer> /**/) {
  ObjectContainer oc;
  oc.rawImpl() = fromStream(in, numRead, Tag<std::vector<std::unique_ptr<Object>>>{});
  return oc;
}

// --- StaticPositionArr ---
template <class T>
std::size_t toStream(std::ostream &out, const StaticPositionArr<T> &input) {
  std::size_t written = 0;
  written += toStream(out, input.width());
  written += toStream(out, input.height());
  for (const auto &elem : input) {
    written += toStream(out, elem);
  }
  return written;
}

template <class T>
StaticPositionArr<T> fromStream(std::istream &in, std::size_t &numRead, Tag<StaticPositionArr<T>> /**/) {
  std::size_t totalRead = 0;
  std::size_t localRead;
  auto width = fromStream(in, localRead, Tag<int>{});
  totalRead += localRead;
  auto height = fromStream(in, localRead, Tag<int>{});
  totalRead += localRead;
  StaticPositionArr<T> arr(width, height);
  for (auto &elem : arr) {
    elem = fromStream(in, localRead, Tag<T>{});
    totalRead += localRead;
  }
  numRead = totalRead;
  return arr;
}

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
  totalRead+=localRead;
  for (auto id : floor.getMonsterArr()) {
    if (!id.isNull())
      floor.addEventListener(id);
  }
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
