export module SerializationLib;
import std;

template <class T>
concept TriviallyCopyable = std::is_trivially_copyable_v<T>;
export namespace SerializationLib {
template<class T>
class Tag{};
template<TriviallyCopyable T>
std::size_t toStream(std::ostream& out, T input){
  static constexpr std::size_t InputSize = sizeof(input);
  auto buffer = std::bit_cast<std::array<char,InputSize>>(input);
  out.write(buffer.data(), InputSize);
  return InputSize;
}

template<TriviallyCopyable T>
T fromStream(std::istream& in,std::size_t& numRead, Tag<T> /**/){
  static constexpr std::size_t OutPutSize = sizeof(T);
  numRead = OutPutSize;
  std::array<char,OutPutSize> buffer;
  in.read(buffer.data(), OutPutSize);
  return std::bit_cast<T>(buffer);
}

template<class T>
T fromStream(std::istream& in, Tag<T> tag){
  std::size_t _;
  return fromStream(in,_,tag);
}


template<class T>
concept Serializeable = requires(std::ostream & out, std::istream& in, std::size_t nRead, const T& input){
  {toStream(out, input)} -> std::same_as<std::size_t>;
  {fromStream(in,nRead,Tag<T>{})} -> std::same_as<T>;
};

std::size_t serialize(std::ostream & out, const Serializeable auto&... inputs) {
  return (toStream(out,inputs)+...);
}

std::size_t deserialize(std::istream & in, Serializeable auto&... inputs) {
  auto readAndGetSize = [&in](auto &toFill){
    std::size_t sz;
    toFill = fromStream(in,sz,Tag<std::remove_reference_t<decltype(toFill)>>{});
    return sz;
  };
  return (readAndGetSize(inputs)+...);
}

} // namespace SerializationLib