export module Common:Static2DArr;
import :IteratorBase;
import :LogStream;
import std;
#ifdef NDEBUG
constexpr bool InDebug = false;
#else
constexpr bool InDebug = true;
#endif

export template <class T, class sizeT = std::size_t>
class Static2DArr {
  public:
  using value_type = T;
  static_assert(std::is_same_v<std::remove_cvref_t<value_type>, value_type>, "Static2DArr must have a non-const, non-volatile, non reference value_type");
  using constvalue_T = const value_type;
  using size_type = sizeT;
  static_assert(std::integral<size_type>);
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
    constexpr static auto CZero = std::views::iota(static_cast<size_type>(0));
    for(auto [row,nRow] : std::views::zip(list,CZero)){
      for(auto [val,nCol] : std::views::zip(row,CZero)){
        (*this)[nRow,nCol] = val;
      }
    }
  }
  constexpr Static2DArr() noexcept : Static2DArr(0,0){}
  constexpr Static2DArr(size_type rows, size_type cols) noexcept : rows_(rows), cols_(cols), data_(getAlloc(rows*cols)){
    if constexpr (InDebug) {
      if (rows < 0 || cols < 0) {
        Logging::log << "Tried to make Static2DArr with dims: " << rows << " by " <<  cols << '\n';
        rows_ = std::max(0,rows);
        cols_ = std::max(0,cols);
      }
    }
  }
  constexpr Static2DArr(const Static2DArr &other) = delete;
  constexpr Static2DArr(Static2DArr &&other) noexcept = default;
  [[nodiscard]] constexpr bool isNull() const noexcept {return data_==nullptr;}
  [[nodiscard]] constexpr auto &operator[](this auto&& self, size_type row, size_type col) noexcept { 
    if constexpr (InDebug) {
      if (!self.inBounds(row,col)) {
        Logging::log << "Bad Static2DArr index\nrow: " << row << " col: " << col << '\n'
                  << "Dims are " << self.rows_ << " x " << self.cols_ << '\n';
        return self[0,0];
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
    return row >= 0 && row < rows() && col >=0 && col < cols();
  } 
  constexpr void fill(const value_type& v) noexcept { 
    std::fill_n(data_.get(),size(),v);
  }
  constexpr auto indexIter() const noexcept{
    return std::views::transform(std::views::iota(static_cast<size_type>(0),rows_*cols_),[cols_=cols_](size_type i){return std::make_pair(i/cols_,i%cols_);});
  }
private:
  [[nodiscard]] constexpr static std::unique_ptr<arr_t> getAlloc(size_type size) noexcept{
    if(size<0){
      return nullptr;
    }
    return std::make_unique<arr_t>(size);
  }
  size_type rows_;
  size_type cols_;
  std::unique_ptr<arr_t> data_; 
};
