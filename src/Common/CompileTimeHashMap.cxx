export module Common:CompileTimeHashMap;
import std;

export namespace CompileTimeHashMap {
template <class KeyT, class ValueT, std::size_t Size>
class Pairing {
public:
  using KeyType = KeyT;
  using ValueType = ValueT;
  struct Pair {
    KeyType key;
    ValueType value;
  };
  consteval explicit Pairing(Pair (&&arr)[Size]) noexcept : data(std::to_array<Pair>(std::move(arr))) { /* NOLINT */ }
  static constexpr std::size_t size() noexcept { return Size; }
  std::array<Pair, Size> data;
};
template <class KeyT, class ValueT, std::size_t Size>
consteval Pairing<KeyT, ValueT, Size> to_Pairing(typename Pairing<KeyT, ValueT, Size>::Pair (&&arr)[Size]) noexcept { // NOLINT
  return Pairing<KeyT, ValueT, Size>(std::move(arr));
}
} // namespace CompileTimeHashMap

constexpr std::size_t Npos = static_cast<std::size_t>(-1);

using CompileTimeHashMap::Pairing;
template <typename T>
concept Hasher = true;

template <Pairing paring, Hasher hasher, decltype(paring)::KeyType nullKV>
consteval std::size_t getHashMapLen() {
  using PairingT = decltype(paring);
  using KeyType = PairingT::KeyType;
  static_assert(std::ranges::none_of(paring.data, [](PairingT::Pair x) { return x.key == nullKV; }), "Used Null value as a Key");
  bool done = false;
  std::size_t len = paring.size();
  std::vector<KeyType> used;
  while (!done) {
    done = true;
    len++;
    used.resize(len);
    std::ranges::fill(used, nullKV);
    for (auto i : paring.data) {
      const std::size_t index = hasher::operator()(i.key) % len;
      if (used[index] != nullKV) {
        if (used[index] == i.key) {
          return Npos;
        }
        done = false;
        break;
      }
      used[index] = i.key;
    }
  }
  return len;
}

template <Pairing paring, Hasher hasher, decltype(paring)::KeyType nullKV, decltype(paring)::ValueType nullVV>
consteval auto makeMap() {
  constexpr std::size_t MapSize = getHashMapLen<paring, hasher, nullKV>();
  static_assert(MapSize != Npos, "Used Duplicate Keys");
  std::array<typename decltype(paring)::Pair, MapSize> ret;
  ret.fill({nullKV, nullVV});
  std::ranges::for_each(paring.data, [&ret](auto i) {
    const std::size_t index = hasher::operator()(i.key) % MapSize;
    ret[index] = i;
  });
  return ret;
}

template <std::convertible_to<std::size_t> T>
class DefaultHasher {
public:
  constexpr static std::size_t operator()(T t) { return t; }
};

export namespace CompileTimeHashMap {
template <Pairing paring, Hasher hasher, decltype(paring)::KeyType nullKV, decltype(paring)::ValueType nullVV>
struct Map {
public:
  using ValueType = decltype(paring)::ValueType;
  using KeyType = decltype(paring)::KeyType;
  [[nodiscard]] constexpr ValueType get(KeyType inV) const noexcept {
    const auto &fd = Impl[hasher::operator()(inV) % Impl.size()];
    return fd.key != inV ? nullVV : fd.value;
  }
  [[nodiscard]] constexpr std::size_t size() const noexcept { return Impl.size(); }

private:
  static constexpr auto Impl = makeMap<paring, hasher, nullKV, nullVV>();
  //   static constexpr std::size_t size_ = map.size();
};

template <Pairing paring, decltype(paring)::KeyType nullKV, decltype(paring)::ValueType nullVV, Hasher hasher = DefaultHasher<typename decltype(paring)::KeyType>>
consteval auto to_Map() {
  return Map<paring, hasher, nullKV, nullVV>();
}
} // namespace CompileTimeHashMap