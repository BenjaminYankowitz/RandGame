#include <gtest/gtest.h>
import Common;
//NOLINTBEGIN(readability-function-cognitive-complexity)

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
constexpr bool initializeConstexpr() {
  SmallVector<T> vec;
  if (vec.size() != 0) return false;
  if (!vec.empty()) return false;
  return true;
}
static_assert(initializeConstexpr<int>());

template <class T>
void push1_back() {
  {
    SmallVector<T> vec;
      vec.push_back(T{23});
      EXPECT_EQ(vec[0], 23);
      EXPECT_EQ(vec.size(), 1u);
      EXPECT_FALSE(vec.empty());
      EXPECT_TRUE(checkActiveVal<T>(1));
  }
  EXPECT_TRUE(checkActiveVal<T>(0));
}

template <class T>
void push2_back_manualReserve() {
  {
    SmallVector<T> vec;
      vec.push_back(T{23});
      vec.reserve(2);
      EXPECT_EQ(vec[0], 23);
      EXPECT_EQ(vec.size(), 1u);
      EXPECT_TRUE(checkActiveVal<T>(1));
      vec.push_back(T{52});
      EXPECT_EQ(vec[0], 23);
      EXPECT_EQ(vec[1], 52);
      EXPECT_EQ(vec.size(), 2u);
      EXPECT_FALSE(vec.empty());
      EXPECT_TRUE(checkActiveVal<T>(2));
  }
  EXPECT_TRUE(checkActiveVal<T>(0));
}

template <class T>
void push_many_back() {
  constexpr int Number = 2;
  constexpr int Base = 3;
  {
    SmallVector<T> vec;
    for(auto i : std::ranges::views::iota(0,Number)){
      vec.push_back(T{i+Base});
      EXPECT_EQ(vec[i], (i+Base));
      EXPECT_EQ(vec.size(), std::size_t(i+1));
      EXPECT_FALSE(vec.empty());
      EXPECT_TRUE(checkActiveVal<T>(i+1));
    }
    if constexpr (std::same_as<int,T>)
      EXPECT_TRUE(std::ranges::equal(vec,std::ranges::views::iota(Base,Base+Number)));
  }
  EXPECT_TRUE(checkActiveVal<T>(0));
}

template <class T>
void reservePreventsReAlloc() {
  {
    SmallVector<T> vec;
    vec.reserve(5);
    EXPECT_GE(vec.capacity(), 5u);
    EXPECT_TRUE(checkActiveVal<T>(0));
    vec.push_back(T{23});
    auto ptr = &vec[0];
    vec.push_back(T{91});
    vec.push_back(T{3});
    vec.push_back(T{28});
    vec.push_back(T{29});
    EXPECT_EQ(ptr, &vec[0]);
    EXPECT_EQ(vec.size(), 5u);
    EXPECT_TRUE(checkActiveVal<T>(5));
    EXPECT_EQ(vec[0], 23);
    EXPECT_EQ(vec[1], 91);
    EXPECT_EQ(vec[2], 3);
    EXPECT_EQ(vec[3], 28);
    EXPECT_EQ(vec[4], 29);
  }
  EXPECT_TRUE(checkActiveVal<T>(0));
}

template <class T>
void emplace_back_test() {
  {
    SmallVector<T> vec;
    auto& ref1 = vec.emplace_back(42);
    EXPECT_EQ(ref1, 42);
    EXPECT_EQ(vec.size(), 1u);
    EXPECT_TRUE(checkActiveVal<T>(1));
    auto& ref2 = vec.emplace_back(99);
    EXPECT_EQ(ref2, 99);
    EXPECT_EQ(vec[0], 42);
    EXPECT_EQ(vec.size(), 2u);
    EXPECT_TRUE(checkActiveVal<T>(2));
  }
  EXPECT_TRUE(checkActiveVal<T>(0));
}

template <class T>
void pop_back_test() {
  {
    SmallVector<T> vec;
    vec.push_back(T{10});
    vec.push_back(T{20});
    vec.push_back(T{30});
    EXPECT_TRUE(checkActiveVal<T>(3));
    vec.pop_back();
    EXPECT_EQ(vec.size(), 2u);
    EXPECT_EQ(vec[0], 10);
    EXPECT_EQ(vec[1], 20);
    EXPECT_TRUE(checkActiveVal<T>(2));
    vec.pop_back();
    EXPECT_EQ(vec.size(), 1u);
    EXPECT_EQ(vec[0], 10);
    EXPECT_TRUE(checkActiveVal<T>(1));
  }
  EXPECT_TRUE(checkActiveVal<T>(0));
}

template <class T>
void clear_test() {
  {
    SmallVector<T> vec;
    vec.push_back(T{1});
    vec.push_back(T{2});
    vec.push_back(T{3});
    EXPECT_TRUE(checkActiveVal<T>(3));
    vec.clear();
    EXPECT_EQ(vec.size(), 0u);
    EXPECT_TRUE(vec.empty());
    EXPECT_TRUE(checkActiveVal<T>(0));
  }
  EXPECT_TRUE(checkActiveVal<T>(0));
}

template <class T>
void resize_test() {
  {
    SmallVector<T> vec;
    vec.resize(3);
    EXPECT_EQ(vec.size(), 3u);
    EXPECT_EQ(vec[0], 0);
    EXPECT_EQ(vec[1], 0);
    EXPECT_EQ(vec[2], 0);
    EXPECT_TRUE(checkActiveVal<T>(3));
    vec.resize(1);
    EXPECT_EQ(vec.size(), 1u);
    EXPECT_EQ(vec[0], 0);
    EXPECT_TRUE(checkActiveVal<T>(1));
    auto oldSize = vec.size();
    vec.resize(oldSize);
    EXPECT_EQ(vec.size(), oldSize);
    EXPECT_TRUE(checkActiveVal<T>(1));
  }
  EXPECT_TRUE(checkActiveVal<T>(0));
}

template <class T>
void front_back_test() {
  {
    SmallVector<T> vec;
    vec.push_back(T{10});
    vec.push_back(T{20});
    vec.push_back(T{30});
    EXPECT_EQ(vec.front(), 10);
    EXPECT_EQ(vec.back(), 30);
    EXPECT_TRUE(checkActiveVal<T>(3));
  }
  EXPECT_TRUE(checkActiveVal<T>(0));
}

template <class T>
void data_test() {
  {
    SmallVector<T> vec;
    EXPECT_EQ(vec.data(), nullptr);
    vec.push_back(T{42});
    EXPECT_NE(vec.data(), nullptr);
    EXPECT_EQ(*vec.data(), 42);
    EXPECT_TRUE(checkActiveVal<T>(1));
  }
  EXPECT_TRUE(checkActiveVal<T>(0));
}

template <class T>
void iterator_test() {
  {
    SmallVector<T> vec;
    EXPECT_EQ(vec.begin(), vec.end());
    EXPECT_EQ(vec.cbegin(), vec.cend());
    vec.push_back(T{1});
    vec.push_back(T{2});
    vec.push_back(T{3});
    EXPECT_EQ(std::distance(vec.begin(), vec.end()), 3);
    int expected = 1;
    for (const auto& elem : vec) {
      EXPECT_EQ(elem, expected);
      expected++;
    }
    EXPECT_TRUE(checkActiveVal<T>(3));
  }
  EXPECT_TRUE(checkActiveVal<T>(0));
}

template <class T>
void swap_test() {
  {
    SmallVector<T> a;
    a.push_back(T{1});
    a.push_back(T{2});
    SmallVector<T> b;
    b.push_back(T{10});
    b.push_back(T{20});
    b.push_back(T{30});
    EXPECT_TRUE(checkActiveVal<T>(5));
    a.swap(b);
    EXPECT_EQ(a.size(), 3u);
    EXPECT_EQ(a[0], 10);
    EXPECT_EQ(a[1], 20);
    EXPECT_EQ(a[2], 30);
    EXPECT_EQ(b.size(), 2u);
    EXPECT_EQ(b[0], 1);
    EXPECT_EQ(b[1], 2);
    EXPECT_TRUE(checkActiveVal<T>(5));
  }
  EXPECT_TRUE(checkActiveVal<T>(0));
}

template <class T>
void move_constructor_test() {
  {
    SmallVector<T> original;
    original.push_back(T{5});
    original.push_back(T{10});
    EXPECT_TRUE(checkActiveVal<T>(2));
    SmallVector<T> moved(std::move(original));
    EXPECT_EQ(moved.size(), 2u);
    EXPECT_EQ(moved[0], 5);
    EXPECT_EQ(moved[1], 10);
    EXPECT_TRUE(original.empty());
    EXPECT_TRUE(checkActiveVal<T>(2));
  }
  EXPECT_TRUE(checkActiveVal<T>(0));
}

template <class T>
void move_assignment_test() {
  {
    SmallVector<T> src;
    src.push_back(T{7});
    src.push_back(T{14});
    SmallVector<T> dst;
    dst.push_back(T{99});
    EXPECT_TRUE(checkActiveVal<T>(3));
    dst = std::move(src);
    EXPECT_EQ(dst.size(), 2u);
    EXPECT_EQ(dst[0], 7);
    EXPECT_EQ(dst[1], 14);
    EXPECT_TRUE(src.empty());
    EXPECT_TRUE(checkActiveVal<T>(2));
  }
  EXPECT_TRUE(checkActiveVal<T>(0));
}

template <class T>
void initializer_list_constructor_test() {
  {
    SmallVector<T> vec{T{1}, T{2}, T{3}};
    EXPECT_EQ(vec.size(), 3u);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
    EXPECT_EQ(vec[2], 3);
    EXPECT_TRUE(checkActiveVal<T>(3));
  }
  EXPECT_TRUE(checkActiveVal<T>(0));
}

template <class T>
void size_constructor_test() {
  {
    SmallVector<T> vec(5);
    EXPECT_EQ(vec.size(), 5u);
    for (std::size_t i = 0; i < 5; i++) {
      EXPECT_EQ(vec[i], 0);
    }
    EXPECT_TRUE(checkActiveVal<T>(5));
  }
  EXPECT_TRUE(checkActiveVal<T>(0));
}

template <class T>
void shrink_to_fit_test() {
  {
    SmallVector<T> vec;
    vec.reserve(100);
    EXPECT_GE(vec.capacity(), 100u);
    vec.push_back(T{1});
    vec.push_back(T{2});
    vec.push_back(T{3});
    vec.shrink_to_fit();
    EXPECT_EQ(vec.capacity(), 3u);
    EXPECT_EQ(vec.size(), 3u);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
    EXPECT_EQ(vec[2], 3);
    EXPECT_TRUE(checkActiveVal<T>(3));
  }
  EXPECT_TRUE(checkActiveVal<T>(0));
}

template <class T>
void reserve_exact_test() {
  {
    SmallVector<T> vec;
    vec.reserveExact(10);
    EXPECT_EQ(vec.capacity(), 10u);
    EXPECT_EQ(vec.size(), 0u);
    EXPECT_TRUE(checkActiveVal<T>(0));
  }
  EXPECT_TRUE(checkActiveVal<T>(0));
}

template <class T>
void reallocation_test() {
  {
    SmallVector<T> vec;
    constexpr int Count = 20;
    for (int i = 0; i < Count; i++) {
      vec.push_back(T{i + 1});
      EXPECT_EQ(vec.size(), std::size_t(i + 1));
      EXPECT_TRUE(checkActiveVal<T>(i + 1));
    }
    for (int i = 0; i < Count; i++) {
      EXPECT_EQ(vec[i], i + 1);
    }
    EXPECT_TRUE(checkActiveVal<T>(Count));
  }
  EXPECT_TRUE(checkActiveVal<T>(0));
}

TEST(SmallVectorTests, InitializeInt) {
  SmallVector<int> vec;
  EXPECT_EQ(vec.size(), 0u);
  EXPECT_TRUE(vec.empty());
}
TEST(SmallVectorTests, InitializeAdvanced) {
  SmallVector<AdvancedStruct> vec;
  EXPECT_EQ(vec.size(), 0u);
  EXPECT_TRUE(vec.empty());
}
TEST(SmallVectorTests, Push1BackInt) { push1_back<int>(); }
TEST(SmallVectorTests, Push1BackAdvanced) { push1_back<AdvancedStruct>(); }
TEST(SmallVectorTests, Push2BackManualReserveInt) { push2_back_manualReserve<int>(); }
TEST(SmallVectorTests, Push2BackManualReserveAdvanced) { push2_back_manualReserve<AdvancedStruct>(); }
TEST(SmallVectorTests, PushManyBackInt) { push_many_back<int>(); }
TEST(SmallVectorTests, PushManyBackAdvanced) { push_many_back<AdvancedStruct>(); }
TEST(SmallVectorTests, ReservePreventsReAllocInt) { reservePreventsReAlloc<int>(); }
TEST(SmallVectorTests, ReservePreventsReAllocAdvanced) { reservePreventsReAlloc<AdvancedStruct>(); }
TEST(SmallVectorTests, EmplaceBackInt) { emplace_back_test<int>(); }
TEST(SmallVectorTests, EmplaceBackAdvanced) { emplace_back_test<AdvancedStruct>(); }
TEST(SmallVectorTests, PopBackInt) { pop_back_test<int>(); }
TEST(SmallVectorTests, PopBackAdvanced) { pop_back_test<AdvancedStruct>(); }
TEST(SmallVectorTests, ClearInt) { clear_test<int>(); }
TEST(SmallVectorTests, ClearAdvanced) { clear_test<AdvancedStruct>(); }
TEST(SmallVectorTests, ResizeInt) { resize_test<int>(); }
TEST(SmallVectorTests, ResizeAdvanced) { resize_test<AdvancedStruct>(); }
TEST(SmallVectorTests, FrontBackInt) { front_back_test<int>(); }
TEST(SmallVectorTests, FrontBackAdvanced) { front_back_test<AdvancedStruct>(); }
TEST(SmallVectorTests, DataInt) { data_test<int>(); }
TEST(SmallVectorTests, DataAdvanced) { data_test<AdvancedStruct>(); }
TEST(SmallVectorTests, IteratorInt) { iterator_test<int>(); }
TEST(SmallVectorTests, IteratorAdvanced) { iterator_test<AdvancedStruct>(); }
TEST(SmallVectorTests, SwapInt) { swap_test<int>(); }
TEST(SmallVectorTests, SwapAdvanced) { swap_test<AdvancedStruct>(); }
TEST(SmallVectorTests, MoveConstructorInt) { move_constructor_test<int>(); }
TEST(SmallVectorTests, MoveConstructorAdvanced) { move_constructor_test<AdvancedStruct>(); }
TEST(SmallVectorTests, MoveAssignmentInt) { move_assignment_test<int>(); }
TEST(SmallVectorTests, MoveAssignmentAdvanced) { move_assignment_test<AdvancedStruct>(); }
TEST(SmallVectorTests, InitializerListInt) { initializer_list_constructor_test<int>(); }
TEST(SmallVectorTests, InitializerListAdvanced) { initializer_list_constructor_test<AdvancedStruct>(); }
TEST(SmallVectorTests, SizeConstructorInt) { size_constructor_test<int>(); }
TEST(SmallVectorTests, SizeConstructorAdvanced) { size_constructor_test<AdvancedStruct>(); }
TEST(SmallVectorTests, ShrinkToFitInt) { shrink_to_fit_test<int>(); }
TEST(SmallVectorTests, ShrinkToFitAdvanced) { shrink_to_fit_test<AdvancedStruct>(); }
TEST(SmallVectorTests, ReserveExactInt) { reserve_exact_test<int>(); }
TEST(SmallVectorTests, ReserveExactAdvanced) { reserve_exact_test<AdvancedStruct>(); }
TEST(SmallVectorTests, ReallocationInt) { reallocation_test<int>(); }
TEST(SmallVectorTests, ReallocationAdvanced) { reallocation_test<AdvancedStruct>(); }

//NOLINTEND(readability-function-cognitive-complexity)
