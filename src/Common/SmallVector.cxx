export module Common:SmallVector;
import :IteratorBase;
import std;

export template<class T>
class SmallVector {
  private:
  using ArrT = std::byte[]; //NOLINT(modernize-avoid-c-arrays);
  static constexpr std::size_t TBytes = sizeof(T);
public:
  using value_type = T;
  using size_type = std::size_t;
  using iterator = IteratorImpl<value_type, SmallVector>;
  using const_iterator = IteratorImpl<const value_type, SmallVector>;
  static_assert(alignof(T)<=16, "Does not support greater than 16 byte alignment");
  SmallVector() = default;
  constexpr explicit SmallVector(size_type n) noexcept{
    resize(n);
  };
  template <class InputIterator>
  constexpr SmallVector(InputIterator first, InputIterator last){
    auto dist = std::distance(first,last);
    reserve(dist);
    for(auto i = first; i!=last; i++){
      unchecked_emplace_back(i);
    }
  }
  template <class R>
  constexpr SmallVector(std::from_range_t /*unused*/, R &&rg) : SmallVector(std::ranges::begin(rg),std::ranges::end(rg)){}
  constexpr SmallVector(SmallVector &&x) noexcept{
    data_ = std::move(x.data_);
  }
  constexpr ~SmallVector() noexcept{
    destroyRegion();
  }
  constexpr SmallVector(std::initializer_list<value_type> il) noexcept(std::is_nothrow_copy_constructible_v<value_type>) : SmallVector(il.begin(),il.end()){}
  constexpr SmallVector &operator=(SmallVector &&x) noexcept{
    data_ = std::move(x.data_);
    return *this;
  };
  [[nodiscard]] constexpr iterator begin() noexcept{
    if(noPtr()){
      return iterator(nullptr);
    }
    return iterator(getPtr(0));
  }
  [[nodiscard]] constexpr const_iterator begin() const noexcept {
    if(noPtr()){
      return const_iterator(nullptr);
    }
    return const_iterator(getPtr(0));
  }
  [[nodiscard]] constexpr iterator end() noexcept{
    if(noPtr()){
      return iterator(nullptr);
    }
    return iterator(getPtr(size()));
  }
  [[nodiscard]] constexpr const_iterator end() const noexcept{
    if(noPtr()){
      return const_iterator(nullptr);
    }
    return const_iterator(getPtr(size()));
  }
  [[nodiscard]] constexpr const_iterator cbegin() const noexcept{return begin();}
  [[nodiscard]] constexpr const_iterator cend() const noexcept{return end();}
  [[nodiscard]] constexpr size_type size() const noexcept{
    if(noPtr()){
      return 0;
    }
    return getSizeRef();
  }
  [[nodiscard]] constexpr static size_type max_size() noexcept{return (std::numeric_limits<size_type>::max()-ExtraBytes)/TBytes;}
  [[nodiscard]] constexpr size_type capacity() const noexcept {
    if(noPtr()){
      return 0;
    }
    return getCapacityRef();
  }
  [[nodiscard]] constexpr bool empty() const noexcept {return size()==0;};
  constexpr void reserve(size_type n) noexcept{if(n>capacity()) setCapacityAtLeast(n);};
  constexpr void reserveExact(size_type n) noexcept{if(n>capacity()) setCapacity(n);};
  constexpr void shrink_to_fit() noexcept{setCapacity(size());};
  [[nodiscard]] constexpr auto& operator[](this auto&& self, size_type n){return std::forward_like<decltype(self)>(*self.getPtr(n));}
  [[nodiscard]] constexpr auto& front(this auto&& self){return self[0];}
  [[nodiscard]] constexpr auto& back(this auto&& self){return self[self.size()-1];}
  [[nodiscard]] constexpr auto data(this auto&& self) noexcept {
    if (self.noPtr()) {
      return static_cast<value_type*>(nullptr);
    }
    return self.getPtr(0);
  };
  constexpr void push_back(const value_type &x) noexcept(std::is_nothrow_copy_constructible_v<value_type>){
    emplace_back(x);
  };
  constexpr void push_back(value_type &&x) noexcept(std::is_nothrow_move_constructible_v<value_type>){
    emplace_back(std::move(x));
  };
  template <class... Args>
  constexpr value_type& emplace_back(Args &&...args) noexcept(std::is_nothrow_constructible_v<value_type, Args...>){
    if(size()==capacity()){
      setCapacityAtLeast(capacity()+1);
    }
    unchecked_emplace_back(std::forward<Args>(args)...);
    return back();
  }
  constexpr void pop_back() noexcept {
    back().~value_type();
    --getSizeRef();
  }
  constexpr void clear() noexcept {
    destroyRegion();
  }
  constexpr void resize(size_type sz) noexcept{
    if(sz<size()){
      destroyRegion(sz);
    } else if(sz > size()){
      setCapacityAtLeast(sz);
      for(size_type i = size(); i < sz; i++){
        std::construct_at(getPtr(i));
      }
    }
    getSizeRef() = sz;
  }
  constexpr void swap(SmallVector & other) noexcept{
    std::swap(data_,other.data_);
  }
  private:
  [[nodiscard]] constexpr bool noPtr() const noexcept{
    return data_==nullptr;
  }
  constexpr void destroyRegion(size_type begin = 0) noexcept{
    if constexpr (std::is_trivially_destructible_v<value_type>){
      return;
    }
    for(std::size_t i = begin; i < size(); i++){
      getPtr(i)->~value_type();
    }
  }
  static constexpr size_type SizeTypeSize = sizeof(size_type);
  static constexpr size_type ExtraBytes = 2*SizeTypeSize;
  [[nodiscard]] static constexpr value_type* getPtrFree(const std::unique_ptr<ArrT>& data, size_type n) noexcept{
    return reinterpret_cast<value_type*>(data.get()+ExtraBytes)+n;
  }
  [[nodiscard]] constexpr value_type* getPtr(size_type n) const noexcept {
    return getPtrFree(data_,n);
  }
  [[nodiscard]] constexpr size_type& getSizeRef() const noexcept{
    return *reinterpret_cast<size_type*>(data_.get());
  }
  [[nodiscard]] constexpr size_type& getCapacityRef() const noexcept{
    return *reinterpret_cast<size_type*>(data_.get()+SizeTypeSize);
  }
  constexpr void setCapacityAtLeast(size_type n){
    setCapacity(std::max(n,capacity()*3/2));
  }
  constexpr void setCapacity(size_type n){
    if(n==0){
      data_ = nullptr;
      return;
    }
    const size_type nDataSize = ExtraBytes + n * TBytes;
    if(noPtr()){
      data_ = std::make_unique<ArrT>(nDataSize);
      getSizeRef() = 0;
      getCapacityRef() = n;
      return;
    }
    auto nData = std::make_unique<ArrT>(nDataSize);
    if constexpr (std::is_trivially_copyable_v<value_type>) {
      std::memcpy(nData.get(), data_.get(), ExtraBytes + size() * TBytes);
    } else {
      std::memcpy(nData.get(), data_.get(), SizeTypeSize);
      value_type *oV = getPtr(0);
      value_type *nV = getPtrFree(nData, 0);
      for (std::size_t i = 0; i < size(); i++) {
        std::construct_at(nV++,std::move(*(oV++)));
      }
    }
    destroyRegion();
    data_ = std::move(nData);
    getCapacityRef() = n;
  }
  template <class... Args>
  constexpr void unchecked_emplace_back(Args&&... args) noexcept(std::is_nothrow_constructible_v<value_type, Args...>){
    if(noPtr()){
      std::cerr << "emplaceback\n";
    }
    std::construct_at(getPtr(getSizeRef()++),std::forward<Args>(args)...);
  }
  std::unique_ptr<ArrT> data_;
};