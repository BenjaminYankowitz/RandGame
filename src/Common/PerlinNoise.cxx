export module PerlinNoise;
import Common;
namespace PerlinNoise {
double Smoothstep(double x, double a, double b) {
  return a + (x * x * (3 - (2 * x)) * (b - a));
}

template <std::size_t N>
class MathVector {
  static_assert(N > 0);

public:
  constexpr MathVector() = default;
  constexpr explicit MathVector(const std::array<double, N> &vector) noexcept : impl_(vector) {}
  constexpr explicit MathVector(const std::array<std::size_t, N> &vector) noexcept {
    for (std::size_t i = 0; i < vector.size(); i++) {
      impl_[i] = vector[i];
    }
  }
  [[nodiscard]] constexpr double operator*(const MathVector &other) const noexcept {
    return std::inner_product(impl_.begin(), impl_.end(), other.impl_.begin(), 0.0);
  }
  [[nodiscard]] constexpr MathVector operator-(const MathVector &other) const noexcept {
    std::array<double, N> ret{};
    std::ranges::transform(std::views::zip(impl_, other.impl_), ret.begin(), [](auto a) {
      return std::get<0>(a) - std::get<1>(a);
    });
    return MathVector(ret);
  }
  [[nodiscard]] constexpr std::array<std::size_t, N> corner(std::size_t n) const noexcept {
    std::array<std::size_t, N> ret{};
    const auto toCorner = [n, impl_ = impl_](std::size_t i) { return ((n & (1 << i)) == 0u) ? std::floor(impl_[i]) : std::ceil(impl_[i]); };
    std::ranges::transform(std::ranges::iota_view(0ul, N), ret.begin(), toCorner);
    return ret;
  }
  [[nodiscard]] constexpr static std::size_t cornerNum() noexcept {
    return 1ul << N;
  }
  std::array<double, N> impl_;
};
template <std::size_t N>
std::ostream &operator<<(std::ostream &out, const MathVector<N> &vec) {
  out << vec.impl_[0];
  std::for_each(std::ranges::drop_view(vec, 1), [&out](auto n) { out << ',' << n; });
  return out;
}

export class Generator {
public:
  Generator(int xDim, int yDim) noexcept : arr_(yDim, xDim) {
    static constexpr double Tau = 2 * std::numbers::pi;
    std::uniform_real_distribution<> zeroOne(0, 1);
    std::uniform_real_distribution<> zeroTau(0, Tau);
    auto mkCircle = [&zeroOne, &zeroTau]() {
      const double rMag = Rnd::get(zeroOne);
      const double mag = rMag * rMag;
      const double angle = Rnd::get(zeroTau);
      return MathVector<2>(std::array<double, 2>{mag * std::cos(angle), mag * std::sin(angle)});
    };
    std::ranges::generate(arr_, mkCircle);
  }
  [[nodiscard]] constexpr double getHeight(double x, double y) {
    const double fX = x - std::floor(x);
    const double fY = y - std::floor(y);
    const MathVector<2> vec(std::array<double, 2>{x, y});
    std::array<double, vec.cornerNum()> values{};
    const auto func = [vec, &arr = arr_](int i) {
      const auto cornerArr = vec.corner(i);
      const MathVector<2> corner(cornerArr);
      return (vec - corner) * (arr[cornerArr[1], cornerArr[0]]);
    };
    std::ranges::transform(std::ranges::iota_view(0ul, values.size()), values.begin(), func);
    double ret = Smoothstep(fY, Smoothstep(fX, values[0], values[1]), Smoothstep(fX, values[2], values[3]));
    return ret;
  }

private:
  Static2DArr<MathVector<2>, int> arr_;
};
} // namespace PerlinNoise
