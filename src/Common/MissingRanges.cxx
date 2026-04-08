export module Common:MissingRanges;
import std;

template <class R1, class R2>
struct ConcatView {
    using difference_type = std::ptrdiff_t;
    using value_type = std::ranges::range_value_t<R1>;
  struct ConcatViewIter {
    using difference_type = std::ptrdiff_t;
    using value_type = std::ranges::range_value_t<R1>;
    struct ConcatViewIterSentinal {
    };
    struct OnRange1 {
      std::ranges::iterator_t<R1> iter;
      std::ranges::sentinel_t<R1> end;
      R2 r2;
    };
    struct OnRange2 {
      std::ranges::iterator_t<R2> iter;
      std::ranges::sentinel_t<R2> end;
    };
    constexpr void inc(OnRange1 iter) noexcept {
      ++iter.iter;
      if (iter.iter == iter.end)
        data = OnRange2{iter.r2.begin(), iter.r2.end()};
    }
    constexpr void inc(OnRange2 iter) noexcept {
      ++iter.iter;
    }
    constexpr ConcatViewIter& operator++() noexcept {
      data.visit([&](auto val) { inc(val); });
      return *this;
    }
    constexpr ConcatViewIter operator++(int) noexcept {
      auto old = *this;
      ++*this;
      return old;
    }
    [[nodiscard]] constexpr auto operator*() const noexcept {
      return data.visit([](auto val) { return *val.iter; });
    }
    [[nodiscard]] constexpr bool operator==(ConcatViewIterSentinal /*unused*/) const noexcept {
      return std::holds_alternative<OnRange2>(data) && std::get<OnRange2>(data).iter == std::get<OnRange2>(data).end;
    }
    std::variant<OnRange1, OnRange2> data;
  };

  [[nodiscard]] constexpr ConcatViewIter begin() const noexcept {
    auto begin = r1.begin();
    auto end = r2.end();
    if (begin == end)
      return ConcatViewIter(typename ConcatViewIter::OnRange2(r2.begin(), r2.end()));
    return ConcatViewIter(typename ConcatViewIter::OnRange1(begin, end, r2));
  }
  [[nodiscard]] constexpr ConcatViewIter::ConcatViewIterSentinal end() const noexcept {
    return {};
  }
  R1 r1;
  R2 r2;
};
export namespace Views {
[[nodiscard]] constexpr auto enumerate(std::ranges::viewable_range auto &&range) noexcept {
  return std::views::zip(std::views::iota(static_cast<std::ranges::range_difference_t<decltype(range)>>(0)), range);
}
[[nodiscard]] constexpr auto concat(std::ranges::viewable_range auto &&range1, std::ranges::viewable_range auto &&range2) noexcept {
  return ConcatView(std::forward<decltype(range1)>(range1), std::forward<decltype(range2)>(range2));
}
} // namespace Views