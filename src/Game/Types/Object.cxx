export module GameTypes:Object;
import :Misc;
import Common;

export struct ObjectBluePrint {
  ObjectType type;
  Material mat = defaultMat(type);
  ArtifactId artifactStatus = ArtifactId::Normal;
  int count = 1;
};

export class Object {
public:
  constexpr Object(const ObjectBluePrint &obj) noexcept : count_(obj.count), type_(obj.type), mat_(obj.mat), artifactStatus_(obj.artifactStatus) {} // NOLINT(google-explicit-constructor)
  [[nodiscard]] constexpr bool isCombinable() const noexcept {
    return artifactStatus_ == ArtifactId::Normal;
  }
  [[nodiscard]] constexpr bool canCombine(const Object &other) const noexcept {
    return other.type_ == type_ && other.mat_ == mat_ && isCombinable() && other.isCombinable();
  }
  constexpr void combine(std::unique_ptr<Object> other) noexcept {
    count_ += other->count_;
  }
  [[nodiscard]] constexpr const int &count() const noexcept { return count_; }
  [[nodiscard]] constexpr int &count() noexcept { return count_; }
  [[nodiscard]] constexpr ObjectType type() const noexcept { return type_; }
  [[nodiscard]] constexpr Material mat() const noexcept { return mat_; }
  [[nodiscard]] constexpr ArtifactId artifactStatus() const noexcept { return artifactStatus_; }
  [[nodiscard]] constexpr std::unique_ptr<Object> split(int n) noexcept {
    count_ -= n;
    auto obj = std::make_unique<Object>(ObjectBluePrint{type_, mat_, artifactStatus_, n});
    return obj;
  }

private:
  int count_;
  ObjectType type_;
  Material mat_;
  ArtifactId artifactStatus_ = ArtifactId::Normal;
};

template <class T>
[[nodiscard]] constexpr auto &deref(T &p) noexcept { return std::forward_like<T &>(*p); };

export class ObjectContainer {
public:
  using iterator = IteratorWrapper<std::unique_ptr<Object> *, Object &, deref>;
  using const_iterator = IteratorWrapper<const std::unique_ptr<Object> *, const Object &, deref>;
  ObjectContainer() = default;
  ObjectContainer(ObjectContainer &) = delete;
  ObjectContainer(ObjectContainer &&) = default;
  ObjectContainer &operator=(ObjectContainer &&) = default;
  constexpr void addObject(std::unique_ptr<Object> obj) noexcept {
    auto v = std::ranges::find_if(*this, [&obj = *obj](const Object &oObj) { return obj.canCombine(oObj); });
    if (v != end()) {
      v->combine(std::move(obj));
    } else {
      impl_.push_back(std::move(obj));
    }
  }
  constexpr void addObject(const ObjectBluePrint &obj) noexcept {
    addObject(std::make_unique<Object>(obj));
  }
  [[nodiscard]] constexpr std::size_t size() const noexcept {
    return impl_.size();
  }
  [[nodiscard]] constexpr bool empty() const noexcept {
    return impl_.empty();
  }
  [[nodiscard]] constexpr Object &operator[](std::size_t i) {
    return *impl_[i];
  }
  [[nodiscard]] constexpr const Object &operator[](std::size_t i) const {
    return *impl_[i];
  }
  [[nodiscard]] constexpr iterator begin() noexcept {
    return {impl_.data()};
  }
  [[nodiscard]] constexpr const_iterator begin() const noexcept {
    return {impl_.data()};
  }
  [[nodiscard]] constexpr iterator end() noexcept {
    return {impl_.data() + impl_.size()};
  }
  [[nodiscard]] constexpr const_iterator end() const noexcept {
    return {impl_.data() + impl_.size()};
  }
  [[nodiscard]] constexpr std::unique_ptr<Object> remove(std::size_t i) noexcept {
    std::unique_ptr<Object> ptr = std::move(impl_[i]);
    if (i + 1 != impl_.size()) {
      impl_[i] = std::move(impl_.back());
    }
    impl_.pop_back();
    return ptr;
  }
  [[nodiscard]] constexpr Object &front() noexcept {
    return operator[](0);
  }
  [[nodiscard]] constexpr const Object &front() const noexcept {
    return operator[](0);
  }
  [[nodiscard]] constexpr Object &back() noexcept {
    return operator[](size() - 1);
  }
  [[nodiscard]] constexpr const Object &back() const noexcept {
    return operator[](size() - 1);
  }

private:
  std::vector<std::unique_ptr<Object>> impl_;
};

static_assert([]() {
  // ObjectContainer q;
  return true;
}());