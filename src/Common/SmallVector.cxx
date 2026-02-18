export module Common:SmallVector;
import :IteratorBase;
import std;

std::unique_ptr<int> z;
 
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
  explicit SmallVector(size_type n) noexcept{
    resize(n);
  };
  template <class InputIterator>
  SmallVector(InputIterator first, InputIterator last){
    auto dist = std::distance(first,last);
    reserve(dist);
    for(auto i = first; i!=last; i++){
      unchecked_emplace_back(i);
    }
  }
  template <class R>
  SmallVector(std::from_range_t /*unused*/, R &&rg) : SmallVector(std::ranges::begin(rg),std::ranges::end(rg)){}
  SmallVector(SmallVector &&x) noexcept{
    data_ = std::move(x.data_);
  }
  ~SmallVector(){
    freeRegion();
  }
  SmallVector(std::initializer_list<value_type> il) noexcept(std::is_nothrow_copy_constructible_v<value_type>) : SmallVector(il.begin(),il.end()){}
  SmallVector &operator=(SmallVector &&x) noexcept{
    data_ = std::move(x.data_);
    return *this;
  };
  [[nodiscard]] iterator begin() noexcept{return iterator(empty() ? nullptr : &getRef(0));}
  [[nodiscard]] const_iterator begin() const noexcept {return const_iterator(empty() ? nullptr : &getRef(0));};
  [[nodiscard]] iterator end() noexcept{return iterator(empty() ? nullptr : &getRef(size()));}
  [[nodiscard]] const_iterator end() const noexcept{return const_iterator(empty() ? nullptr : &getRef(size()));};
  [[nodiscard]] const_iterator cbegin() const noexcept{return begin();}
  [[nodiscard]] const_iterator cend() const noexcept{return end();}
  [[nodiscard]] size_type size() const noexcept{return empty() ? 0 : getSizeRef();}
  [[nodiscard]] static constexpr size_type max_size() noexcept{return (std::numeric_limits<size_type>::max()-ExtraBytes)/TBytes;}
  [[nodiscard]] size_type capacity() const noexcept {return empty() ? 0 : getCapacityRef();}
  [[nodiscard]] bool empty() const noexcept {return data_==nullptr;};
  void reserve(size_type n) noexcept{if(n<capacity()) setCapacity(n);};
  void shrink_to_fit() noexcept{setCapacity(size());};
  [[nodiscard]] auto& operator[](this auto&& self, size_type n){return std::forward_like<decltype(self)>(self.getRef(n));}
  [[nodiscard]] auto& front(this auto&& self){return self[0];}
  [[nodiscard]] auto& back(this auto&& self){return self[self.size()-1];}
  [[nodiscard]] auto data(this auto&& self) noexcept {return &self[0];};
  
  void push_back(const value_type &x) noexcept(std::is_nothrow_copy_constructible_v<value_type>){
    emplace_back(x);
  };
  void push_back(value_type &&x) noexcept(std::is_nothrow_move_constructible_v<value_type>){
    emplace_back(std::move(x));
  };
  template <class... Args>
  value_type& emplace_back(Args &&...args) noexcept(std::is_nothrow_constructible_v<value_type, Args...>){
    if(size()==capacity()){
      setCapacityAtLeast(capacity()+1);
    }
    unchecked_emplace_back(std::forward<Args>(args)...);
    return back();
  }
  void pop_back() noexcept {
    back().~value_type();
    --getSizeRef();
    if(size() == 0){
      data_ = nullptr;
    }
  }
  void clear() noexcept {
    freeRegion();
    data_ = nullptr;
  }
  void resize(size_type sz) noexcept{
    if(sz==0){
      clear();
      return;
    }
    if(sz<size()){
      freeRegion(sz);
    } else if(sz > size()){
      setCapacityAtLeast(sz);
      for(size_type i = size(); i < sz; i++){
        std::construct_at(getPtr(i));
      }
    }
    getSizeRef() = sz;
  }
  void swap(SmallVector & other) noexcept{
    std::swap(data_,other.data_);
  }
  private:
  void freeRegion(size_type begin = 0){
    if constexpr (std::is_trivially_destructible_v<value_type>){
      return;
    }
    for(std::size_t i = begin; i < size(); i++){
      getRef(i).~value_type();
    }
  }
  static constexpr size_type SizeTypeSize = sizeof(size_type);
  static constexpr size_type ExtraBytes = 2*SizeTypeSize;
  static constexpr value_type* getPtrFree(const std::unique_ptr<ArrT>& data, size_type n) noexcept{
    return reinterpret_cast<value_type*>(data.get()+ExtraBytes)+n;
  }
  [[nodiscard]] value_type* getPtrFree(size_type n) const noexcept{
    return &getSizeRef(n);
  }
  [[nodiscard]] value_type* getPtr(size_type n) const noexcept {
    return getPtrFree(data_,n);
  }
  [[nodiscard]] value_type& getRef(size_type n) const noexcept {
    return *getPtrFree(data_,n);
  }
  [[nodiscard]] size_type& getSizeRef() const noexcept{
    return *reinterpret_cast<size_type*>(data_.get());
  }
  [[nodiscard]] size_type& getCapacityRef() const noexcept{
    return *reinterpret_cast<size_type*>(data_.get()+SizeTypeSize);
  }
  void setCapacityAtLeast(size_type n){
    setCapacity(std::max(n,capacity()*3/2));
  }
  void setCapacity(size_type n){
    if(n==0){
      data_ = nullptr;
      return;
    }
    const size_type nDataSize = ExtraBytes + n * TBytes;
    if(data_==nullptr){
      data_ = std::make_unique<ArrT>(nDataSize);
      getSizeRef() = 0;
      getCapacityRef() = n;
      return;
    }
    if(data_!=nullptr) {
      auto nData = std::make_unique<ArrT>(nDataSize);
      if constexpr (std::is_trivially_move_constructible_v<value_type>){
        std::memcpy(nData.get(),data_.get(),ExtraBytes+size()*TBytes);
        return;
      }
      std::memcpy(nData.get(), data_.get(), ExtraBytes);
      value_type *oV = getPtr(0);
      value_type *nV = getPtrFree(nData, 0);
      for (std::size_t i = 0; i < size(); i++) {
        *(nV++) = std::move(*(oV++));
      }
      data_ = std::move(nData);
    }
  }
  template <class... Args>
  void unchecked_emplace_back(Args&&... args) {
    std::construct_at(getPtr(getSizeRef()++),std::forward<Args>(args)...);
  }
  std::unique_ptr<ArrT> data_;
};