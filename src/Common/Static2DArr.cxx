export module Common:Static2DArr;
import :IteratorBase;
import std;
#ifdef NDEBUG
constexpr bool InDebug = false;
#else
constexpr bool InDebug = true;
#endif

// export namespace Cmn {
export template <class T>
class Static2DArr {
  public:
  using value_type = T;
  static_assert(std::is_same<std::remove_cvref_t<value_type>, value_type>::value, "Static2DArr must have a non-const, non-volatile, non reference value_type");
  using constvalue_T = const value_type;
  using size_type = std::size_t;
  using iterator = IteratorImpl<value_type, Static2DArr>;
  using const_iterator = IteratorImpl<const value_type, Static2DArr>;
  private:
  using arr_t = value_type[];//NOLINT(modernize-avoid-c-arrays)
  template<class U>
  struct PiteratorImpl{};
  template<>
  struct PiteratorImpl<Static2DArr>{using type = iterator;};
  template<>
  struct PiteratorImpl<const Static2DArr>{using type = const_iterator;};
  template<class U>
  using piterator = PiteratorImpl<std::remove_reference_t<U>>::type;
public:
  template<size_type size>
  constexpr Static2DArr(std::initializer_list<value_type[size]> list) noexcept : Static2DArr(list.size(),size){ // NOLINT(modernize-avoid-c-arrays)
    std::size_t nRow = 0;
    for(auto row : list){
      for(std::size_t nCol : std::ranges::views::iota(static_cast<size_type>(0),size)){
        (*this)[nRow,nCol] = row[nCol];
      }
      nRow++;
    }
  }
  constexpr Static2DArr() noexcept : Static2DArr(0,0){}
  constexpr Static2DArr(size_type rows, size_type cols) noexcept : rows_(rows), cols_(cols), data_(getAlloc(rows*cols)){} 
  constexpr Static2DArr(const Static2DArr &other) = delete;
  constexpr Static2DArr(Static2DArr &&other) noexcept = default;
  [[nodiscard]] constexpr bool isNull() const noexcept {return data_==nullptr;}
  [[nodiscard]] constexpr auto &operator[](this auto&& self, size_type row, size_type col) noexcept { 
    if constexpr (InDebug) {
      if (row >= self.rows_ || col >= self.cols_) {
        std::cerr << "Bad Static2DArr index\nrow: " << row << " col: " << col << '\n'
                  << "Dims are " << self.rows_ << " x " << self.cols_ << '\n';
        std::exit(1);
      }
    }
    return std::forward_like<decltype(self)>(self.data_[(row * self.cols_) + col]);
  }
  [[nodiscard]] constexpr auto begin(this auto&& self) noexcept { return piterator<decltype(self)>(self.data_.get()); }
  [[nodiscard]] constexpr auto end(this auto&& self) noexcept { return piterator<decltype(self)>(self.begin() + self.size()); }
  [[nodiscard]] constexpr size_type rows() const noexcept { return rows_; }
  [[nodiscard]] constexpr size_type cols() const noexcept { return cols_; }
  [[nodiscard]] constexpr size_type size() const noexcept { return cols_ * rows_; }
  [[nodiscard]] constexpr bool inBounds(size_type row, size_type col) const noexcept{
    return row < rows() && col < cols();
  } 
  constexpr void fill(const value_type& v) noexcept { 
    std::fill_n(data_.get(),size(),v);
  }
private:
  [[nodiscard]] constexpr static std::unique_ptr<arr_t> getAlloc(std::size_t size) noexcept{
    return std::make_unique<arr_t>(size);
  }
  size_type rows_;
  size_type cols_;
  std::unique_ptr<arr_t> data_; 
};
// }
