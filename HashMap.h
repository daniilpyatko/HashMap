#include <stdexcept>

#include <vector>


//  HashMap that uses separate chaining for collision resolution.
//  The rescaling technique keeps the size of the structure close to the number of elements inside
//  and thus lets the naive traversal of all elements to work in O(elements_inside).
//  The exact conditions for rescaling are near the checkRebuild method.
template <class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
 public:
  static constexpr size_t kMinCapacity = 4;
  static constexpr size_t kExpand = 2;
  static constexpr size_t kShrink = 4;

  HashMap(Hash hasher = Hash()) : hasher_(hasher) { all_.resize(capacity_); }

  template <typename iterator>
  HashMap(iterator b, iterator s, Hash hasher = Hash())
      : hasher_(hasher) {
    all_.resize(capacity_);
    while (b != s) {
      insert(*b++);
    }
  }

  HashMap(std::initializer_list<std::pair<const KeyType, ValueType> > init,
      Hash hasher_ = Hash()) : hasher_(hasher_) {
    all_.resize(capacity_);
    auto b = init.begin();
    auto s = init.end();
    while (b != s) {
      insert(*b++);
    }
  }

  HashMap& operator=(const HashMap& other) {
    if (this != &other) {
      hasher_ = other.hasher_;
      all_.resize(other.all_.size());
      for (size_t i = 0; i < all_.size(); ++i) {
        for (size_t j = 0; j < other.all_[i].size(); ++j) {
          all_[i].push_back(other.all_[i][j]);
        }
      }
    }
    return *this;
  }

  Hash hash_function() const { return hasher_; }

  std::pair<int, int> insert(std::pair<const KeyType, ValueType> x, std::pair<size_t, bool> hsh = {0, false}) {
    checkRebuild();
    bool found = false;
    size_t ind = 0;
    if (hsh.second) {
      ind = hsh.first % capacity_;
    } else {
      ind = hasher_(x.first) % capacity_;
    }
    size_t ipos = 0;
    for (size_t i = 0; i < all_[ind].size(); ++i) {
      if (all_[ind][i].first == x.first) {
        found = true;
        ipos = i;
      }
    }
    if (found) {
      return {static_cast<int>(ind), ipos};
    }

    all_[ind].push_back(x);
    cnt_elements_++;
    return {static_cast<int>(ind), static_cast<int>(all_[ind].size() - 1)};
  }

  void erase(KeyType x) {
    checkRebuild();
    size_t ind = hasher_(x) % capacity_;
    bool found = false;
    std::vector<std::pair<const KeyType, ValueType> > tmpall_;
    for (size_t i = 0; i < all_[ind].size(); ++i) {
      if (all_[ind][i].first == x) {
        found = true;
      } else {
        tmpall_.push_back(all_[ind][i]);
      }
    }
    if (!found) {
      return;
    }

    all_[ind].clear();
    for (size_t i = 0; i < tmpall_.size(); ++i) {
      all_[ind].push_back(tmpall_[i]);
    }
    cnt_elements_--;
  }

  ValueType& operator[](KeyType x) {
    size_t hsh = hasher_(x);
    iterator cur = find(x, {hsh, true});
    if (cur != end()) {
      return cur->second;
    } else {
      auto pos = iterator(insert({x, ValueType()}, {hsh, true}), this);
      return pos->second;
    }
  }

  const ValueType& at(KeyType x) const {
    size_t hsh = hasher_(x);
    const_iterator cur = find(x, {hsh, true});
    if (cur != end()) {
      return cur->second;
    } else {
      throw std::out_of_range("Element is not in the HashMap");
    }
  }

  void clear() {
    all_.clear();
    capacity_ = kMinCapacity;
    cnt_elements_ = 0;
    all_.resize(capacity_);
  }

  int size() const { return cnt_elements_; }

  bool empty() const { return cnt_elements_ == 0; }


//  ForwardIterator for HashMap.
//  Stores the pair of numbers (chain's position in the structure, position in the chain) for the element it is pointing at.
//  There are also two special states kBeforeBeginPos and kAfterEndPos.
  class iterator {
   public:
    iterator() {}
    iterator(std::pair<int, int> pos, HashMap* phash_map) : pos_(pos), phash_map_(phash_map) {}
    bool operator==(const iterator& other) { return pos_ == other.pos_; }
    bool operator!=(const iterator& other) { return !(pos_ == other.pos_); }
    iterator operator++(int foo) {
      iterator tmp = iterator(pos_, phash_map_);
      pos_ = phash_map_->findNext(pos_);
      return tmp;
    }
    iterator operator++() {
      pos_ = phash_map_->findNext(pos_);
      return *this;
    }
    std::pair<const KeyType, ValueType>& operator*() {
      return phash_map_->all_[pos_.first][pos_.second];
    }
    std::pair<const KeyType, ValueType>* operator->() {
      return &(phash_map_->all_[pos_.first][pos_.second]);
    }

   private:
    std::pair<int, int> pos_;
    HashMap* phash_map_;
  };
  iterator begin() { return iterator(findNext(kBeforeBeginPos), this); }
  iterator end() { return iterator(kAfterEndPos, this); }


//  Same as iterator but doesn't allow changes
  class const_iterator {
   public:
    const_iterator() {}
    const_iterator(std::pair<int, int> pos, const HashMap* phash_map) : pos_(pos), phash_map_(phash_map) {}
    bool operator==(const const_iterator& other) { return pos_ == other.pos_; }
    bool operator!=(const const_iterator& other) { return !(pos_ == other.pos_); }
    const_iterator operator++(int foo) {
      const_iterator tmp = const_iterator(pos_, phash_map_);
      pos_ = phash_map_->findNext(pos_);
      return tmp;
    }
    const_iterator operator++() {
      pos_ = phash_map_->findNext(pos_);
      return *this;
    }
    const std::pair<const KeyType, ValueType>& operator*() {
      return phash_map_->all_[pos_.first][pos_.second];
    }
    const std::pair<const KeyType, ValueType>* operator->() {
      return &(phash_map_->all_[pos_.first][pos_.second]);
    }

   private:
    std::pair<int, int> pos_;
    const HashMap* phash_map_;
  };
  const_iterator begin() const { return const_iterator(findNext(kBeforeBeginPos), this); }
  const_iterator end() const { return const_iterator(kAfterEndPos, this); }
  iterator find(KeyType x, std::pair<size_t, bool> hsh = {0, false}) {
    size_t ind = 0;
    if (hsh.second) {
      ind = hsh.first % capacity_;
    } else {
      ind = hasher_(x) % capacity_;
    }
    for (size_t i = 0; i < all_[ind].size(); ++i) {
      if (x == all_[ind][i].first) {
        return iterator({static_cast<int>(ind), i}, this);
      }
    }
    return end();
  }

  const_iterator find(KeyType x, std::pair<size_t, bool> hsh = {0, false}) const {
    size_t ind = 0;
    if (hsh.second) {
      ind = hsh.first % capacity_;
    } else {
      ind = hasher_(x) % capacity_;
    }
    for (size_t i = 0; i < all_[ind].size(); ++i) {
      if (x == all_[ind][i].first) {
        return const_iterator({static_cast<int>(ind), i}, this);
      }
    }
    return end();
  }


 private:
//  indicates the position before the first element
  const std::pair<int,int> kBeforeBeginPos = {-1, 0};
//  indicates the position after the last element
  const std::pair<int,int> kAfterEndPos = {-2, 0};
  size_t cnt_elements_ = 0;
  size_t capacity_ = kMinCapacity;
  std::vector<std::vector<std::pair<const KeyType, ValueType> > > all_;
  Hash hasher_;

  std::pair<int, int> findNext(std::pair < int, int > pos) const {
    if (pos == kBeforeBeginPos) {
      for (size_t i = 0; i < all_.size(); ++i) {
        if (all_[i].size() != 0) {
          return {i, 0};
        }
      }
      return kAfterEndPos;
    }

    if(pos == kAfterEndPos){
      return kAfterEndPos;
    }

    if (pos.second + 1 < static_cast<int>(all_[pos.first].size())) {
      pos.second++;
      return pos;
    } else {
      for (size_t i = pos.first + 1; i < capacity_; ++i) {
        if (all_[i].size() != 0) {
          return {i, 0};
        }
      }
      return kAfterEndPos;
    }
  }

  void checkRebuild(){
    if (cnt_elements_ < capacity_ / kShrink) {
      rebuild();
    } else if (cnt_elements_ > capacity_ * kExpand) {
      rebuild();
    }
  }

  void rebuild() {
    int newcapacity = std::max(static_cast<size_t>(kMinCapacity), cnt_elements_ * kExpand);
    capacity_ = newcapacity;
    std::vector<std::vector<std::pair<const KeyType, ValueType> > > nall(capacity_);
    for (size_t i = 0; i < all_.size(); ++i) {
      for (size_t j = 0; j < all_[i].size(); ++j) {
        nall[hasher_(all_[i][j].first) % capacity_].push_back(all_[i][j]);
      }
    }
    all_.swap(nall);
  }
};
