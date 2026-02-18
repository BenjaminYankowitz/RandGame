export module Common:Random;
import std;

template<class T>
concept Distribution = requires(T dist, std::mt19937 state){
  {dist(state)} -> std::same_as<typename T::result_type>;
};

[[nodiscard]] std::seed_seq getSeed() noexcept {
  using intType = std::uint_fast32_t;
  static constexpr intType DefaultSeed = 659949982;
  intType seeda = DefaultSeed;
  try {
    std::random_device rd;
    seeda = rd();
  } catch (...) {}
  intType seedb = std::time(nullptr);
  return {seeda,seedb};
}

static inline std::mt19937 rndState = [](){
  auto seedSeq = getSeed();
  std::mt19937 ret(seedSeq);
  return ret;
}();

export namespace Rnd {
  auto get(Distribution auto& dist){
    return dist(rndState);
  }
  auto callable(Distribution auto& dist){
    return [&dist](){return dist(rndState);};
  }
  void shuffle(auto& arr){
    std::ranges::shuffle(arr,rndState);
  }
} // namespace Rnd