export module Common:IteratorBase;
import std;

export template <class RefValueT, class Parent, class RetValueT = RefValueT&, RetValueT(*F)(RefValueT&) = [](RefValueT& v) -> RetValueT {return static_cast<RetValueT>(v);}>
class IteratorImpl {
  template <class ORefValueT, class OParent, class ORetValueT,ORetValueT(*OF)(ORefValueT&)>
  friend class IteratorImpl;
public:
  using difference_type = std::ptrdiff_t;
  using value_type = std::remove_reference_t<RetValueT>;
  [[nodiscard]] constexpr RetValueT operator[](difference_type i) const noexcept { return F(impl_[i]); }
  [[nodiscard]] constexpr RetValueT operator*() const noexcept { return operator[](0); }
  [[nodiscard]] constexpr value_type *operator->() const noexcept { 
    static_assert(std::is_reference_v<RetValueT>);
    return &operator*();
  }
  IteratorImpl() = default;
  constexpr IteratorImpl(const IteratorImpl&) = default;
  template <class ORefValueT, class ORetValueT,ORetValueT(*OF)(ORefValueT&)>
  constexpr IteratorImpl(IteratorImpl<ORefValueT,Parent,ORetValueT,OF> other) noexcept : impl_(other.impl_){}; //NOLINT(google-explicit-constructor)
  constexpr IteratorImpl &operator++() noexcept { return operator+=(1); }
  constexpr IteratorImpl operator++(int) noexcept {
    auto cp = *this;
    operator++();
    return cp;
  }
  constexpr IteratorImpl &operator+=(difference_type i) noexcept {
    impl_ = std::next(impl_,i);
    return *this;
  }
  [[nodiscard]] constexpr IteratorImpl operator+(difference_type i) const noexcept {
    auto cp = *this;
    return cp += i;
  }
  constexpr IteratorImpl &operator--() noexcept { return operator-=(1); }
  constexpr IteratorImpl operator--(int) noexcept {
    auto cp = *this;
    operator--();
    return cp;
  }
  constexpr IteratorImpl &operator-=(difference_type i) noexcept {
    impl_ = std::prev(impl_,i);
    return *this;
  }
  [[nodiscard]] constexpr IteratorImpl operator-(difference_type i) const noexcept {
    auto cp = *this;
    return cp -= i;
  }
  [[nodiscard]] constexpr difference_type operator-(IteratorImpl other) const noexcept {
    return std::distance(other.impl_,impl_);
  }
  [[nodiscard]] constexpr std::strong_ordering operator<=>(const IteratorImpl& other) const noexcept = default;
  [[nodiscard]] constexpr bool operator==(const IteratorImpl& other) const noexcept = default;

private:
  friend Parent;
  constexpr explicit IteratorImpl(RefValueT *in) noexcept : impl_(in) {}
  RefValueT *impl_;
};