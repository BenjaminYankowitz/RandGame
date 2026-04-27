export module SerializationLib;
import std;

template <class T>
concept TriviallyCopyable = std::is_trivially_copyable_v<T>;
template <class T>
concept EmptyClass = std::is_empty_v<T>;

export namespace SerializationLib {
template <class T>
class Tag {};

class DeserializationError : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};
} // namespace SerializationLib
using SerializationLib::DeserializationError;
using SerializationLib::Tag;

template <class Variant, std::size_t... Is>
Variant fromStreamVariantHelper(std::istream &in, std::size_t index, std::index_sequence<Is...> /*unused*/) {
  using reader_t = Variant (*)(std::istream &);
  static constexpr std::array Readers = std::to_array<reader_t>({[](std::istream &in_) -> Variant {
    return Variant(std::in_place_index<Is>, fromStream(in_, Tag<std::variant_alternative_t<Is, Variant>>{}));
  }...});
  return Readers[index](in);
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
void toStream(std::ostream &out, T input) {
  if constexpr (EmptyClass<T>) {
    return;
  } else {
    static constexpr std::size_t InputSize = sizeof(input);
    auto buffer = std::bit_cast<std::array<char, InputSize>>(input);
    out.write(buffer.data(), InputSize);
  }
}

template <TriviallyCopyable T>
T fromStream(std::istream &in, Tag<T> /**/) {
  if constexpr (EmptyClass<T>) {
    return T{};
  } else {
    static constexpr std::size_t OutPutSize = sizeof(T);
    std::array<char, OutPutSize> buffer;
    in.read(buffer.data(), OutPutSize);
    if (!in || in.gcount() != static_cast<std::streamsize>(OutPutSize)) {
      throw DeserializationError("SerializationLib: short read / stream failure");
    }
    return std::bit_cast<T>(buffer);
  }
}

template <class T>
concept Serializeable = requires(std::ostream &out, std::istream &in, const T &input) {
  { toStream(out, input) } -> std::same_as<void>;
  { fromStream(in, Tag<T>{}) } -> std::same_as<T>;
};

void serialize(std::ostream &out, const Serializeable auto &...inputs) {
  (toStream(out, inputs), ...);
}

void deserialize(std::istream &in, Serializeable auto &...inputs) {
  ((inputs = fromStream(in, Tag<std::remove_reference_t<decltype(inputs)>>{})), ...);
}

// --- std::unique_ptr ---
template <class T, EmptyClass Dealoc>
void toStream(std::ostream &out, const std::unique_ptr<T, Dealoc> &input) {
  bool hasValue = input != nullptr;
  toStream(out, hasValue);
  if (hasValue) {
    toStream(out, *input);
  }
}

template <class T>
std::unique_ptr<T> fromStream(std::istream &in, Tag<std::unique_ptr<T>> /**/) {
  auto hasValue = fromStream(in, Tag<bool>{});
  if (hasValue) {
    return std::make_unique<T>(fromStream(in, Tag<T>{}));
  }
  return nullptr;
}

// --- std::variant ---
template <class... Ts>
void toStream(std::ostream &out, const std::variant<Ts...> &input) {
  toStream(out, input.index());
  std::visit([&](const auto &val) { toStream(out, val); }, input);
}

template <class... Ts>
std::variant<Ts...> fromStream(std::istream &in, Tag<std::variant<Ts...>> /**/) {
  auto index = fromStream(in, Tag<std::size_t>{});
  if (index >= sizeof...(Ts)) {
    throw DeserializationError("SerializationLib: variant index out of range");
  }
  return fromStreamVariantHelper<std::variant<Ts...>>(in, index, std::index_sequence_for<Ts...>{});
}

// --- std::vector ---
template <class T>
void toStream(std::ostream &out, const std::vector<T> &input) {
  toStream(out, input.size());
  for (const auto &elem : input) {
    toStream(out, elem);
  }
}

template <class T>
std::vector<T> fromStream(std::istream &in, Tag<std::vector<T>> /**/) {
  auto size = fromStream(in, Tag<std::size_t>{});
  std::vector<T> vec;
  vec.reserve(size);
  for (std::size_t i = 0; i < size; ++i) {
    vec.push_back(fromStream(in, Tag<T>{}));
  }
  return vec;
}

// --- std::unordered_map ---
template <class K, class V>
void toStream(std::ostream &out, const std::unordered_map<K, V> &input) {
  toStream(out, input.size());
  for (const auto &[key, val] : input) {
    toStream(out, key);
    toStream(out, val);
  }
}

template <class K, class V>
std::unordered_map<K, V> fromStream(std::istream &in, Tag<std::unordered_map<K, V>> /**/) {
  auto size = fromStream(in, Tag<std::size_t>{});
  std::unordered_map<K, V> map;
  map.reserve(size);
  for (std::size_t i = 0; i < size; ++i) {
    auto key = fromStream(in, Tag<K>{});
    auto val = fromStream(in, Tag<V>{});
    map.emplace(std::move(key), std::move(val));
  }
  return map;
}

// --- std::priority_queue ---
template <class T, class Container, EmptyClass Compare>
void toStream(std::ostream &out, const std::priority_queue<T, Container, Compare> &input) {
  toStream(out, pqContainer(input));
}

template <class T, class Container, EmptyClass Compare>
std::priority_queue<T, Container, Compare> fromStream(std::istream &in, Tag<std::priority_queue<T, Container, Compare>> /**/) {
  auto container = fromStream(in, Tag<Container>{});
  return std::priority_queue<T, Container, Compare>(Compare{}, std::move(container));
}

} // namespace SerializationLib
