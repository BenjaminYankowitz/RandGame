export module GameTypes:GameTime;
import std;

export class TimePeriod {
public:
  friend class GameTime;
  constexpr explicit TimePeriod(std::size_t time) noexcept : impl(time) {}
  [[nodiscard]] constexpr bool operator==(const TimePeriod &other) const noexcept = default;
  [[nodiscard]] constexpr auto operator<=>(const TimePeriod &other) const noexcept = default;
  [[nodiscard]] constexpr TimePeriod operator+(TimePeriod other) const noexcept {
    auto cp = *this;
    cp += other;
    return cp;
  }
  constexpr TimePeriod &operator+=(TimePeriod other) noexcept {
    impl += other.impl;
    return *this;
  }
  [[nodiscard]] constexpr TimePeriod operator*(std::size_t other) const noexcept {
    auto cp = *this;
    cp *= other;
    return cp;
  }
  constexpr TimePeriod &operator*=(std::size_t other) noexcept {
    impl *= other;
    return *this;
  }
  constexpr TimePeriod &operator--() noexcept {
    --impl;
    return *this;
  }
  [[nodiscard]] constexpr bool future() const noexcept { return impl != 0; }
  constexpr TimePeriod &operator/=(std::size_t div) noexcept {
    impl /= div;
    return *this;
  }
  [[nodiscard]] constexpr TimePeriod operator/(std::size_t div) const noexcept {
    TimePeriod cp = *this;
    return cp /= div;
  }

  std::uint64_t impl;
};

export class GameTime {
public:
  constexpr GameTime() noexcept = default;
  [[nodiscard]] constexpr auto operator<=>(const GameTime &other) const noexcept = default;
  [[nodiscard]] constexpr GameTime operator+(TimePeriod timePassed) const noexcept {
    GameTime nTime(*this);
    return nTime += timePassed;
  }
  constexpr GameTime &operator+=(TimePeriod timePassed) noexcept {
    impl += timePassed.impl;
    return *this;
  }
  constexpr GameTime &operator++() noexcept {
    ++impl;
    return *this;
  }
  std::uint64_t impl = 0;
};


export std::ostream &operator<<(std::ostream &out, GameTime time) {
  return out << time.impl;
}
