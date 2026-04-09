export module SerializationLib;
import std;

template <class T>
concept TriviallyCopyable = std::is_trivially_copyable_v<T>;
template <class T>
concept EmptyClass = std::is_empty_v<T>;

export namespace SerializationLib {
template <class T>
class Tag {};
}
using SerializationLib::Tag;

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


export namespace SerializationLib {
template <TriviallyCopyable T>
std::size_t toStream(std::ostream &out, T input) {
  if constexpr (EmptyClass<T>) {
    return 0;
  }
  static constexpr std::size_t InputSize = sizeof(input);
  auto buffer = std::bit_cast<std::array<char, InputSize>>(input);
  out.write(buffer.data(), InputSize);
  return InputSize;
}

template <TriviallyCopyable T>
T fromStream(std::istream &in, std::size_t &numRead, Tag<T> /**/) {
  if constexpr (EmptyClass<T>) {
    numRead = 0;
    return T{};
  }
  static constexpr std::size_t OutPutSize = sizeof(T);
  numRead = OutPutSize;
  std::array<char, OutPutSize> buffer;
  in.read(buffer.data(), OutPutSize);
  return std::bit_cast<T>(buffer);
}

template <class T>
T fromStream(std::istream &in, Tag<T> tag) {
  std::size_t _;
  return fromStream(in, _, tag);
}

template <class T>
concept Serializeable = requires(std::ostream &out, std::istream &in, std::size_t nRead, const T &input) {
  { toStream(out, input) } -> std::same_as<std::size_t>;
  { fromStream(in, nRead, Tag<T>{}) } -> std::same_as<T>;
};

std::size_t serialize(std::ostream &out, const Serializeable auto &...inputs) {
  return (toStream(out, inputs) + ...);
}

std::size_t deserialize(std::istream &in, Serializeable auto &...inputs) {
  auto readAndGetSize = [&in](auto &toFill) {
    std::size_t sz;
    toFill = fromStream(in, sz, Tag<std::remove_reference_t<decltype(toFill)>>{});
    return sz;
  };
  return (readAndGetSize(inputs) + ...);
}

// --- std::unique_ptr ---
template <class T, EmptyClass Dealoc>
std::size_t toStream(std::ostream &out, const std::unique_ptr<T, Dealoc> &input) {
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

} // namespace SerializationLib