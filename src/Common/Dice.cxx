export module Common:Dice;
import :Random;
import :Misc;
import std;

namespace {
struct MonoState {};
} // namespace
constexpr std::int64_t stringViewToNumber(std::string_view str) {
  std::int64_t out = 0;
  bool negative = str[0] == '-';
  std::from_chars_result result = std::from_chars(str.begin() + ((str[0] == '+' || str[0] == '-') ? 1 : 0), str.end(), out);
  if (result.ec != std::errc()) {
    throw ERRCException(result.ec);
  }
  return negative ? -out : out;
}

[[nodiscard]] constexpr std::int64_t sign(int n) noexcept {
  if (n < 0)
    return -1;
  if (n > 0)
    return 1;
  return 0;
}

export namespace Dice {
class SingleTypeGroup {
public:
  consteval explicit SingleTypeGroup(std::string_view writtenExplanation) {
    const std::int64_t dLoc = writtenExplanation.find_first_of("dD");
    const std::int64_t start = writtenExplanation.find_first_not_of(' ');
    if (dLoc == start) {
      number_ = 1;
    } else {
      number_ = stringViewToNumber(writtenExplanation.substr(start, dLoc - start));
    }
    if (dLoc == std::string_view::npos) {
      faces_ = 1;
    } else {
      faces_ = stringViewToNumber(writtenExplanation.substr(dLoc + 1));
    }
  }
  constexpr SingleTypeGroup(std::uint16_t faces, std::int16_t number) : faces_(faces), number_(number) { //NOLINT(bugprone-easily-swappable-parameters)
    if (faces == 0) {
      throw std::invalid_argument{"Cannot have a zero sided die"};
    }
  }
  constexpr SingleTypeGroup() noexcept : faces_(1), number_(0) {}
  [[nodiscard]] std::int64_t operator()() const noexcept {
    std::uniform_int_distribution<std::int64_t> dist(1, faces_);
    auto view = std::views::repeat(MonoState{}, std::abs(number_));
    return std::transform_reduce(view.begin(), view.end(), 0ul, std::plus<>(), [&dist](MonoState) {
             return Rnd::get(dist);
           }) *
           sign(number_);
  }
  [[nodiscard]] constexpr std::int64_t min() const noexcept { return number_; }
  [[nodiscard]] constexpr std::int64_t max() const noexcept { return static_cast<std::int64_t>(number_) * faces_; }
  [[nodiscard]] constexpr std::uint16_t getFaces() const noexcept { return faces_; }
  [[nodiscard]] constexpr std::int16_t getNumber() const noexcept { return number_; }

private:
  std::uint16_t faces_;
  std::int16_t number_;
};

class Group {
public:
  constexpr static std::size_t MaxTypes = 2;
  consteval explicit Group(std::string_view writtenExplanation) : constant_(0) {
    const std::size_t strLen = writtenExplanation.size();
    std::size_t beginSection = 0;
    std::size_t endSection = std::min(writtenExplanation.find_first_of("+-"), strLen);
    std::size_t typeNum = 0;
    while (beginSection < strLen) {
      const SingleTypeGroup g(writtenExplanation.substr(beginSection, endSection - beginSection));
      if (g.getFaces() == 1) {
        constant_ += g.getNumber();
      } else {
        dice_[typeNum++] = g;
      }
      if (endSection == std::string_view::npos) {
        break;
      }
      beginSection = endSection;
      endSection = std::min(writtenExplanation.find_first_of("+-", endSection + 1), strLen);
    }
  }
  constexpr explicit Group(std::int16_t constant, SingleTypeGroup s1 = {}, SingleTypeGroup s2 = {}) noexcept : constant_(constant), dice_({s1, s2}) {}
  [[nodiscard]] std::int64_t operator()() const noexcept {
    return std::transform_reduce(dice_.begin(), dice_.end(), constant_, std::plus<>(), [](SingleTypeGroup die) {
      return die();
    });
  }
  [[nodiscard]] constexpr std::int64_t min() const noexcept {
    return std::transform_reduce(dice_.begin(), dice_.end(), static_cast<std::int64_t>(constant_), std::plus<>(), [](const SingleTypeGroup &die) {
      return die.min();
    });
  }
  [[nodiscard]] constexpr std::int64_t max() const noexcept {
    return std::transform_reduce(dice_.begin(), dice_.end(), static_cast<std::int64_t>(constant_), std::plus<>(), [](const SingleTypeGroup &die) {
      return die.max();
    });
  }

  // private:
  std::int16_t constant_;
  std::array<SingleTypeGroup, MaxTypes> dice_;
};

namespace Literals {
consteval Group operator""_dice(const char *str, std::size_t len) noexcept {
  return Group(std::string_view(str, len));
}
consteval SingleTypeGroup operator""_diceST(const char *str, std::size_t len) noexcept {
  return SingleTypeGroup(std::string_view(str, len));
}
static_assert("1d3+1"_dice.constant_ == 1);
} // namespace Literals

}; // namespace Dice
