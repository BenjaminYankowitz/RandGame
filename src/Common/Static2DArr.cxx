export module Common:Static2DArr;
import :IteratorBase;
import :LogStream;
import :Debug;
import std;
import SerializationLib;

export template <class T, class sizeT = std::size_t>
class Static2DArr {
public:
  using value_type = T;
  static_assert(std::is_same_v<std::remove_cvref_t<value_type>, value_type>, "Static2DArr must have a non-const, non-volatile, non reference value_type");
  using constvalue_T = const value_type;
  using size_type = sizeT;
  static_assert(std::integral<size_type>);
  using iterator = value_type *;
  using const_iterator = constvalue_T *;

private:
  using arr_t = value_type[];
  template <bool b>
  struct PiteratorImpl {};
  template <>
  struct PiteratorImpl<false> {
    using type = iterator;
  };
  template <>
  struct PiteratorImpl<true> {
    using type = const_iterator;
  };
  template <class U>
  using piterator = PiteratorImpl<std::is_const_v<U>>::type;

public:
  template <size_type size>
  constexpr Static2DArr(std::initializer_list<value_type[size]> list) noexcept : Static2DArr(list.size(), size) {
    constexpr static auto CZero = std::views::iota(static_cast<size_type>(0));
    for (auto [row, nRow] : std::views::zip(list, CZero)) {
      for (auto [val, nCol] : std::views::zip(row, CZero)) {
        (*this)[nRow, nCol] = val;
      }
    }
  }
  constexpr Static2DArr() noexcept : Static2DArr(0, 0) {}
  constexpr Static2DArr(size_type rows, size_type cols) noexcept : rows_(rows), cols_(cols), data_(getAlloc(rows * cols)) {
    if constexpr (InDebug) {
      if (rows < 0 || cols < 0) {
        Logging::log << "Tried to make Static2DArr with dims: " << rows << " by " << cols << '\n';
        rows_ = std::max<size_type>(0, rows);
        cols_ = std::max<size_type>(0, cols);
        data_ = nullptr;
      }
    }
  }
  constexpr Static2DArr(const Static2DArr &other) = delete;
  constexpr Static2DArr &operator=(const Static2DArr &other) = delete;
  constexpr Static2DArr(Static2DArr &&other) noexcept = default;
  constexpr Static2DArr &operator=(Static2DArr &&other) noexcept = default;
  constexpr ~Static2DArr() noexcept = default;
  [[nodiscard]] constexpr bool isNull() const noexcept { return data_ == nullptr; }
  [[nodiscard]] constexpr auto &operator[](this auto &&self, size_type row, size_type col) noexcept {
    if constexpr (InDebug) {
      if (!self.Static2DArr::inBounds(row, col)) {
        Logging::log << "Bad Static2DArr index\nrow: " << row << " col: " << col << '\n'
                     << "Dims are " << self.rows_ << " x " << self.cols_ << '\n';
        return std::forward_like<decltype(self)>(self.data_[0]);
      }
    }
    return std::forward_like<decltype(self)>(self.data_[(row * self.cols_) + col]);
  }
  [[nodiscard]] constexpr auto begin(this auto &&self) noexcept { return Static2DArr::piterator<decltype(self)>(self.data_.get()); }
  [[nodiscard]] constexpr auto end(this auto &&self) noexcept { return Static2DArr::piterator<decltype(self)>(self.begin() + self.size()); }
  [[nodiscard]] constexpr size_type rows() const noexcept { return rows_; }
  [[nodiscard]] constexpr size_type cols() const noexcept { return cols_; }
  [[nodiscard]] constexpr size_type size() const noexcept { return cols_ * rows_; }
  [[nodiscard]] constexpr bool inBounds(size_type row, size_type col) const noexcept {
    return row >= 0 && row < rows() && col >= 0 && col < cols();
  }
  constexpr void fill(const value_type &v) noexcept {
    std::fill_n(data_.get(), size(), v);
  }
  constexpr auto indexIter() const noexcept {
    return std::views::transform(std::views::iota(static_cast<size_type>(0), rows_ * cols_), [cols_ = cols_](size_type i) { return std::make_pair(i / cols_, i % cols_); });
  }
  [[nodiscard]] constexpr size_type flatIndex(size_type row, size_type col) const noexcept {
    return (row * cols_) + col;
  }

private:
  [[nodiscard]] constexpr static std::unique_ptr<arr_t> getAlloc(size_type size) noexcept {
    if (size <= 0) {
      return nullptr;
    }
    return std::make_unique<arr_t>(size);
  }
  size_type rows_;
  size_type cols_;
  std::unique_ptr<arr_t> data_;
};

using SerializationLib::fromStream;
using SerializationLib::Tag;
using SerializationLib::toStream;

export template <class T, class sizeT>
void toStream(std::ostream &out, const Static2DArr<T, sizeT> &input) {
  toStream(out, input.rows());
  toStream(out, input.cols());
  for (const auto &elem : input) {
    toStream(out, elem);
  }
}

export template <class T, class sizeT>
Static2DArr<T, sizeT> fromStream(std::istream &in, Tag<Static2DArr<T, sizeT>> /**/) {
  auto rows = fromStream(in, Tag<sizeT>{});
  auto cols = fromStream(in, Tag<sizeT>{});
  Static2DArr<T, sizeT> arr(rows, cols);
  for (auto &elem : arr) {
    elem = fromStream(in, Tag<T>{});
  }
  return arr;
}
