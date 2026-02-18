export module SerializationLib;
import std;

template <class T>
concept TriviallyCopyable = std::is_trivially_copyable_v<T>;
export namespace SerializationLib {


std::size_t toStream(std::ostream& out, TriviallyCopyable auto input){
  static constexpr std::size_t InputSize = sizeof(input);
  auto buffer = std::bit_cast<std::array<char,InputSize>>(input);
  out.write(buffer.data(), InputSize);
  return InputSize;
}

template<TriviallyCopyable T>
T fromStream(std::istream& in,std::size_t& numRead){
  static constexpr std::size_t OutPutSize = sizeof(T);
  numRead = OutPutSize;
  std::array<char,OutPutSize> buffer;
  in.read(buffer.data(), OutPutSize);
  return std::bit_cast<T>(buffer);
}

template<class T>
T fromStream(std::istream& in){
  std::size_t _;
  return fromStream<T>(in,_);
}


template<class T>
concept Serializeable = requires(std::ostream & out, std::istream& in, std::size_t nRead, const T& input){
  {toStream(out, input)} -> std::same_as<std::size_t>;
  {fromStream<T>(in,nRead)} -> std::same_as<T>;
};

std::size_t serialize(std::ostream & out, const Serializeable auto&... inputs) {
  return (toStream(out,inputs)+...);
}

std::size_t deserialize(std::istream & in, const Serializeable auto&... inputs) {
  auto readAndGetSize = [&in](auto &toFill){
    std::size_t sz;
    toFill = fromStream<decltype(toFill)>(in,sz);
    return sz;
  };
  return (readAndGetSize(in,inputs)+...);
}

} // namespace SerializationLib