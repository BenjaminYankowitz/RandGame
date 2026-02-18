export module GameTypes:GameTime;
import std;

export class TimePeriod {
public:
  friend class GameTime;
  constexpr explicit TimePeriod(std::size_t time) noexcept : impl_(time) {}
  [[nodiscard]] constexpr auto operator<=>(const TimePeriod &other) const noexcept { return impl_ <=> other.impl_; }
  [[nodiscard]] constexpr TimePeriod operator+(TimePeriod other) const noexcept {
    auto cp = *this;
    cp += other;
    return cp;
  }
  constexpr TimePeriod &operator+=(TimePeriod other) noexcept {
    impl_ += other.impl_;
    return *this;
  }
  [[nodiscard]] constexpr TimePeriod operator*(std::size_t other) const noexcept {
    auto cp = *this;
    cp *= other;
    return cp;
  }
  constexpr TimePeriod &operator*=(std::size_t other) noexcept {
    impl_ *= other;
    return *this;
  }
  constexpr TimePeriod &operator--() noexcept {
    --impl_;
    return *this;
  }
  [[nodiscard]] constexpr bool future() const noexcept { return impl_ != 0; }
  constexpr TimePeriod operator/=(std::size_t div) noexcept {
    impl_ /= div;
    return *this;
  }
  [[nodiscard]] constexpr TimePeriod operator/(std::size_t div) const noexcept {
    TimePeriod cp = *this;
    return cp /= div;
  }

private:
  std::size_t impl_;
};

export class GameTime {
public:
  constexpr GameTime() noexcept = default;
  [[nodiscard]] constexpr auto operator<=>(const GameTime &other) const noexcept = default;
  constexpr GameTime operator+(TimePeriod timePassed) noexcept {
    GameTime nTime(*this);
    return nTime += timePassed;
  }
  constexpr GameTime &operator+=(TimePeriod timePassed) noexcept {
    impl += timePassed.impl_;
    return *this;
  }
  constexpr GameTime &operator++() noexcept {
    ++impl;
    return *this;
  }
  std::size_t impl = 0;
};