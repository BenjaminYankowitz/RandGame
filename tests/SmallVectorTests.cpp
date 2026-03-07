import Common;
//NOLINTBEGIN(readability-function-cognitive-complexity)

using TestResult = int;
constexpr TestResult FailedTest = 1;
constexpr TestResult PassedTest = 0;

#define REQUIRE(expresion) \
  do {                     \
    if (!(expresion)) {      \
      std::cout << #expresion << '\n'; \
      return false;        \
    }                      \
  } while (false)

#define RUNTEST(test) \
  do {                     \
    if (!(test<int>() && test<AdvancedStruct>())) {      \
      return FailedTest;        \
    }                      \
  } while (false)

struct AdvancedStruct {
  AdvancedStruct() noexcept : AdvancedStruct(0) {};
  explicit AdvancedStruct(int n) noexcept{
    val_ = n;
    total_++;
  }
  AdvancedStruct(const AdvancedStruct&o) noexcept {
    val_ = o.val_;
    total_++;
  }
  AdvancedStruct(AdvancedStruct&&o)  noexcept {
    val_ = o.val_;
    total_++;
  }
  ~AdvancedStruct() {
    total_--;
  }
  static std::size_t total() {
    return total_;
  }
  bool operator==(int n) const {
    return val_ == n;
  }
  bool operator==(const AdvancedStruct& ) const = default;

private:
  int val_;
  int ccnt_ = cnt++;
  static std::size_t total_;
  static int cnt;
};
[[nodiscard]] bool operator==(int n, const AdvancedStruct& s){
  return s==n;
}
std::size_t AdvancedStruct::total_ = 0;
int AdvancedStruct::cnt = 0;

template<class T>
[[nodiscard]] constexpr bool checkActiveVal(std::size_t n){
  if constexpr (std::same_as<T,AdvancedStruct>){
    return AdvancedStruct::total()==n;
  } else {
    return AdvancedStruct::total()==0;
  }
}

template <class T>
constexpr bool initialize() {
  SmallVector<T> vec;
  REQUIRE(vec.size() == 0);
  REQUIRE(vec.empty());
  return true;
}
static_assert(initialize<int>());

template <class T>
bool push1_back() {
  {
    SmallVector<T> vec;
      vec.push_back(T{23});
      REQUIRE(vec[0]==23);
      REQUIRE(vec.size()==1);
      REQUIRE(!vec.empty());
      REQUIRE(checkActiveVal<T>(1));
  }
  return checkActiveVal<T>(0);
}
template <class T>
bool push2_back_manualReserve() {
  {
    SmallVector<T> vec;
      vec.push_back(T{23});
      vec.reserve(2);
      REQUIRE(vec[0]==23);
      REQUIRE(vec.size()==1);
      REQUIRE(checkActiveVal<T>(1));
      vec.push_back(T{52});
      REQUIRE(vec[0]==23);
      REQUIRE(vec[1]==52);
      REQUIRE(vec.size()==2);
      REQUIRE(!vec.empty());
      REQUIRE(checkActiveVal<T>(2));
  }
  return checkActiveVal<T>(0);
}

template <class T>
bool push_many_back() {
  constexpr int Number = 2;
  constexpr int Base = 3;
  {
    SmallVector<T> vec;
    for(auto i : std::ranges::views::iota(0,Number)){
      vec.push_back(T{i+Base});
      REQUIRE(vec[i]==(i+Base));
      REQUIRE(vec.size()==std::size_t(i+1));
      REQUIRE(!vec.empty());
      REQUIRE(checkActiveVal<T>(i+1));
    }
    if constexpr (std::same_as<int,T>)
      REQUIRE(std::ranges::equal(vec,std::ranges::views::iota(Base,Base+Number)));
  }
  return checkActiveVal<T>(0);
}

template <class T>
bool reservePreventsReAlloc() {
  {
    SmallVector<T> vec;
    vec.reserve(5);
    REQUIRE(vec.capacity() >= 5);
    REQUIRE(checkActiveVal<T>(0));
    vec.push_back(T{23});
    auto ptr = &vec[0];
    vec.push_back(T{91});
    vec.push_back(T{3});
    vec.push_back(T{28});
    vec.push_back(T{29});
    REQUIRE(ptr==&vec[0]);
    REQUIRE(vec.size() == 5);
    REQUIRE(checkActiveVal<T>(5));
    REQUIRE(vec[0] == 23);
    REQUIRE(vec[1] == 91);
    REQUIRE(vec[2] == 3);
    REQUIRE(vec[3] == 28);
    REQUIRE(vec[4] == 29);
  }
  return checkActiveVal<T>(0);
}
// SmallVector()
// constexpr explicit SmallVector(size_type n)
// template <class InputIterator>
// constexpr SmallVector(InputIterator first, InputIterator last)
// template <class R>
// constexpr SmallVector(std::from_range_t /*unused*/, R &&rg)
// constexpr SmallVector(SmallVector &&x) noexcept
// constexpr ~SmallVector()
// constexpr SmallVector(std::initializer_list<value_type> il)
// constexpr SmallVector &operator=(SmallVector &&x)
// [[nodiscard]] constexpr iterator begin()
// [[nodiscard]] constexpr const_iterator begin()
// [[nodiscard]] constexpr iterator end()
// [[nodiscard]] constexpr const_iterator end()
// [[nodiscard]] constexpr const_iterator cbegin()
// [[nodiscard]] constexpr const_iterator cend()
// [[nodiscard]] constexpr size_type size()
// [[nodiscard]] constexpr static size_type max_size()
// [[nodiscard]] constexpr size_type capacity()
// [[nodiscard]] constexpr bool empty()
// constexpr void reserve(size_type n)
// constexpr void shrink_to_fit()
// [[nodiscard]] constexpr auto& operator[](this auto&& self, size_type n)
// [[nodiscard]] constexpr auto& front(this auto&& self)
// [[nodiscard]] constexpr auto& back(this auto&& self)
// [[nodiscard]] constexpr auto data(this auto&& self)

// constexpr void push_back(const value_type &x)
// constexpr void push_back(value_type &&x)
// template <class... Args>
// constexpr value_type& emplace_back(Args &&...args)
// constexpr void pop_back()
// constexpr void clear()
// constexpr void resize(size_type sz)
// constexpr void swap(SmallVector & other)

int main() {
  try{
  RUNTEST(initialize);
  RUNTEST(push1_back);
  RUNTEST(push2_back_manualReserve);
  RUNTEST(push_many_back);
  RUNTEST(reservePreventsReAlloc);
  } catch(const std::exception& e){
    std::cout << e.what() << '\n';
    return FailedTest;
  }
  return PassedTest;
}

//NOLINTEND(readability-function-cognitive-complexity)