export module Common:IteratorBase;
import std;

template <class Input, class Output>
[[nodiscard]] constexpr Output defaultConvert(Input v) { return static_cast<Output>(v); }

template <class Input, class Output> 
using ConvertFType = decltype(defaultConvert<Input,Output>);

export template <class ParentIterator, class RetValueT = typename std::iterator_traits<ParentIterator>::reference, ConvertFType<typename std::iterator_traits<ParentIterator>::reference,RetValueT>* F = defaultConvert<typename std::iterator_traits<ParentIterator>::reference,RetValueT>>
class IteratorWrapper {
  template <class ORefValueT, class OParent, class ORetValueT, ConvertFType<ORefValueT,ORetValueT> *OF>
  friend class IteratorImpl;

public:
  using difference_type = std::ptrdiff_t;
  using value_type = std::remove_reference_t<RetValueT>;
  [[nodiscard]] constexpr RetValueT operator[](difference_type i) const noexcept { return F(impl_[i]); }
  [[nodiscard]] constexpr RetValueT operator*() const noexcept { return F(*impl_); }
  [[nodiscard]] constexpr value_type *operator->() const noexcept {
    static_assert(std::is_reference_v<RetValueT>);
    return &operator*();
  }
  IteratorWrapper() = default;
  constexpr IteratorWrapper(ParentIterator parent) noexcept : impl_(parent) {}; // NOLINT(google-explicit-constructor)
  constexpr IteratorWrapper &operator++() noexcept { return operator+=(1); }
  constexpr IteratorWrapper operator++(int) noexcept {
    auto cp = *this;
    operator++();
    return cp;
  }
  constexpr IteratorWrapper &operator+=(difference_type i) noexcept {
    impl_ = std::next(impl_, i);
    return *this;
  }
  [[nodiscard]] constexpr IteratorWrapper operator+(difference_type i) const noexcept {
    auto cp = *this;
    return cp += i;
  }
  constexpr IteratorWrapper &operator--() noexcept { return operator-=(1); }
  constexpr IteratorWrapper operator--(int) noexcept {
    auto cp = *this;
    operator--();
    return cp;
  }
  constexpr IteratorWrapper &operator-=(difference_type i) noexcept {
    impl_ = std::prev(impl_, i);
    return *this;
  }
  [[nodiscard]] constexpr IteratorWrapper operator-(difference_type i) const noexcept {
    auto cp = *this;
    return cp -= i;
  }
  [[nodiscard]] constexpr difference_type operator-(IteratorWrapper other) const noexcept {
    return std::distance(other.impl_, impl_);
  }
  [[nodiscard]] constexpr std::strong_ordering operator<=>(const IteratorWrapper &other) const noexcept = default;
  [[nodiscard]] constexpr bool operator==(const IteratorWrapper &other) const noexcept = default;

private:
  ParentIterator impl_;
};

export template<auto F,class ParentIterator>
auto mkIteratorWrapper(ParentIterator iter){
  return IteratorWrapper<ParentIterator,decltype(F(iter)),F>(iter);
}