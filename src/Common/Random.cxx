export module Common:Random;
import std;

template <class T>
concept Distribution = requires(T dist, std::mt19937 state) {
  { dist(state) } -> std::same_as<typename T::result_type>;
};

[[nodiscard]] std::seed_seq getSeed() noexcept {
  using intType = std::uint_fast32_t;
  static constexpr intType DefaultSeed = 659949982;
  intType seeda = DefaultSeed;
  try {
    std::random_device rd;
    seeda = rd();
  } catch (...) {
  }
  intType seedb = std::time(nullptr);
  return {seeda, seedb};
}

static inline std::mt19937 rndState = []() {
  auto seedSeq = getSeed();
  std::mt19937 ret(seedSeq);
  return ret;
}();

export namespace Rnd {
auto get(Distribution auto &dist) {
  return dist(rndState);
}
template <std::integral T>
auto uniform_int(T min, T max) {
  std::uniform_int_distribution<T> dist(min, max);
  return get(dist);
}
auto rnd(std::integral auto n) {
  return uniform_int<decltype(n)>(0, n - 1);
}
template <std::floating_point T, bool EndOpen = false>
auto uniform_real(T min, T max) {
  std::uniform_real_distribution<T> dist(min, max);
  T res = get(dist);
  if constexpr (EndOpen) {
    if (res == max) {
      res = std::nextafter(max, min);
    }
  }
  return res;
}
template <std::floating_point T = double, bool EndOpen = false>
T uniform_01() {
  return uniform_real<T, EndOpen>(0, 1);
}
bool flip(double prob = 0.5) {
  std::bernoulli_distribution dist(prob);
  return get(dist);
}
void shuffle(auto &arr) {
  std::ranges::shuffle(arr, rndState);
}
} // namespace Rnd
