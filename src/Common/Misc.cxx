export module Common:Misc;
import :IteratorBase;
import std;

export template <std::integral INT>
class DisjointSet {
  public:
  constexpr explicit DisjointSet(INT maxSize) noexcept : parent_(maxSize) {
    std::iota(parent_.begin(), parent_.end(), 0);
  }
  [[nodiscard]] constexpr INT find_set(INT v) noexcept {
    while(v!=parent_[v]){
      v = parent_[v] = parent_[parent_[v]];
    }
    return v;
  }
  [[nodiscard]] constexpr bool union_set(INT a, INT b) noexcept {
    a = find_set(a);
    b = find_set(b);
    parent_[b] = a;
    return a!=b;
  }
  private:
  std::vector<INT> parent_;
};

export template <class IteratorType>
class Iterable {
public:
  constexpr Iterable(IteratorType begin, IteratorType end) noexcept : begin_(begin), end_(end) {}
  [[nodiscard]] constexpr IteratorType begin() const noexcept { return begin_; }
  [[nodiscard]] constexpr IteratorType end() const noexcept { return end_; }

private:
  IteratorType begin_;
  IteratorType end_;
};

export template <class T>
class OptionalReference {
private:
public:
  using iterator = IteratorImpl<T, OptionalReference>;
  using const_iterator = IteratorImpl<const T, OptionalReference>;
  constexpr explicit OptionalReference(T *ptr) noexcept : ptr_(ptr) {}
  constexpr explicit OptionalReference(T& value) noexcept : ptr_(&value) {}
  constexpr OptionalReference() = default;
  [[nodiscard]] constexpr iterator begin() noexcept { return iterator(ptr_); }
  [[nodiscard]] constexpr const_iterator begin() const noexcept { return const_iterator(ptr_); }
  [[nodiscard]] constexpr iterator end() noexcept { return endIter(); }
  [[nodiscard]] constexpr const_iterator end() const noexcept { return endIter(); }
  [[nodiscard]] constexpr T &operator*() noexcept { return *ptr_; }
  [[nodiscard]] constexpr const T &operator*() const noexcept { return *ptr_; }
  [[nodiscard]] constexpr T *operator->() noexcept { return ptr_; }
  [[nodiscard]] constexpr const T *operator->() const noexcept { return ptr_; }
  [[nodiscard]] constexpr T value_or(T value) const noexcept { return has_value() ? *ptr_ : value; }
  [[nodiscard]] constexpr explicit operator bool() const noexcept { return has_value(); }
  [[nodiscard]] constexpr bool has_value() const noexcept { return ptr_; }
  constexpr void doIfValue(auto &&f) {
    if (has_value()) {
      f(*ptr_);
    }
  }
  constexpr void doIfNoValue(auto&& f){
    if(!has_value()){
      f();
    }
  }
  constexpr auto doIf(auto&& noValue, auto&& hasValue){
    if(has_value()){
      return hasValue(*ptr_);
    }
    return noValue();
    
  }

private:
  [[nodiscard]] constexpr iterator endIter() const noexcept{
    return iterator(has_value() ? ptr_ + 1 : nullptr);
  }
  T *ptr_ = nullptr;
};

export template <class T>
class MustInit {
public: // NOLINTBEGIN(google-explicit-constructor)
  constexpr MustInit(T v) noexcept : v_(v) {}
  [[nodiscard]] constexpr operator T &() noexcept { return v_; }
  [[nodiscard]] constexpr operator const T &() const noexcept { return v_; }
  // NOLINTEND(google-explicit-constructor)
private:
  T v_;
};

export template <class T>
struct GetEnumValue {
public:
  static constexpr void get(const T & /* Unused */) noexcept {
    static_assert(false,"You must specify how to get the enum value from your object");
  }
};

template <class T>
concept EnumType = std::is_enum_v<T>;
template <class ObjectT, std::size_t size>
class EnumToObject {
private:
  std::array<ObjectT, size> impl_;
  using EnumT = decltype(GetEnumValue<ObjectT>::get(impl_[0]));

public:
  [[nodiscard]] consteval explicit EnumToObject(const std::array<ObjectT, size> &arr) noexcept : impl_(arr) {}
  [[nodiscard]] constexpr ObjectT &operator[](EnumT e) noexcept {
    return impl_[std::to_underlying(e)];
  }
  [[nodiscard]] constexpr const ObjectT &operator[](EnumT e) const noexcept {
    return impl_[std::to_underlying(e)];
  }
};


export template <class ObjectT, std::size_t size, std::size_t... Is>
constexpr auto mkEnumToObjectimpl(std::array<ObjectT,size> arr, std::index_sequence<Is...> /*template thing*/) {
  std::array<std::size_t, size> indexMapping;
  std::ranges::for_each(std::ranges::iota_view(0ul,size),[&](std::size_t i){
    indexMapping[std::to_underlying(GetEnumValue<ObjectT>::get(arr[i]))] = i;
  });
  return EnumToObject<ObjectT, size>(std::array<ObjectT, size>{(arr[indexMapping[Is]])...});
}

export template <class ObjectT, std::size_t size>
[[nodiscard]] constexpr auto mkEnumToObject(ObjectT (&&arr)[size]) noexcept { // NOLINT(modernize-avoid-c-arrays)
  return mkEnumToObjectimpl(std::to_array(std::move(arr)), std::make_index_sequence<size>());
}

export [[nodiscard]] constexpr std::string_view ERRCTOString(std::errc errorCode) noexcept {
  switch (errorCode) {
    using enum std::errc;
  case invalid_argument:
    return "errc: invalid argument";
  case result_out_of_range:
    return "errc: result out of range";
  default:
    return "errc: unimplemented code";
  }
}

export class ERRCException final : public std::exception {
public:
  explicit ERRCException(std::errc errorCode) noexcept : errorCode_(errorCode) {}
  [[nodiscard]] const char *what() const noexcept final {
    return ERRCTOString(errorCode_).data();
  }
  [[nodiscard]] std::errc getErrorCode() const noexcept {
    return errorCode_;
  }

private:
  std::errc errorCode_;
};