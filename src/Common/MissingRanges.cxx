export module Common:MissingRanges;
import std;

export auto enumerate(std::ranges::viewable_range auto&& range){
  return std::views::zip(std::views::iota(static_cast<std::ranges::range_difference_t<decltype(range)>>(0)),range);
}