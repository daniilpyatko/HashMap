#include <stdexcept>

#include <vector>

using namespace std;

template <class KeyType, class ValueType, class Hash = std::hash<KeyType> >
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

  HashMap(std::initializer_list<pair<const KeyType, ValueType> > init,
      Hash hasher_ = Hash()) : hasher_(hasher_) {
    all_.resize(capacity_);
    auto b = init.begin();
    auto s = init.end();
    while (b != s) {
      insert(*b++);
    }
  }

  HashMap& operator=(const HashMap& h1) {
    if (this != &h1) {
      hasher_ = h1.hasher_;
      all_.resize(h1.all_.size());
      for (size_t i = 0; i < all_.size(); ++i) {
        for (size_t j = 0; j < h1.all_[i].size(); ++j) {
          all_[i].push_back(h1.all_[i][j]);
        }
      }
    }
    return *this;
  }

  Hash hash_function() const { return hasher_; }

  void insert(pair<const KeyType, ValueType> x) {
    bool found = false;
    size_t ind = hasher_(x.first) % capacity_;
    for (size_t i = 0; i < all_[ind].size(); ++i) {
      if (all_[ind][i].first == x.first) {
        found = true;
      }
    }
    if (!found) {
      all_[ind].push_back(x);
      cnt_elements_++;
      if (cnt_elements_ > capacity_ * kExpand) {
        rebuild();
      }
    }
  }

  void erase(KeyType x) {
    size_t ind = hasher_(x) % capacity_;
    bool found = false;
    vector<pair<const KeyType, ValueType> > tmpall_;
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
    if (cnt_elements_ < capacity_ / kShrink) {
      rebuild();
    }
  }

  ValueType& operator[](KeyType x) {
    iterator cur = find(x);
    if (cur != end()) {
      return cur->second;
    } else {
      insert({x, ValueType()});
      iterator tmp = find(x);
      return tmp->second;
    }
  }

  const ValueType& at(KeyType x) const {
    const_iterator cur = find(x);
    if (cur != end()) {
      return cur->second;
    } else {
      throw out_of_range("Element is not in the HashMap");
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

  // ForwardIterator for HashMap
  class iterator {
   public:
    iterator() {}
    iterator(pair<int, int> _pos, HashMap* phash_map) : pos(_pos), phash_map_(phash_map) {}
    bool operator==(const iterator& other) { return pos == other.pos; }
    bool operator!=(const iterator& other) { return !(pos == other.pos); }
    iterator operator++(int foo) {
      iterator tmp = iterator(pos, h);
      pos = phash_map_->findNext(pos);
      return tmp;
    }
    iterator operator++() {
      pos = phash_map_->findNext(pos);
      return *this;
    }
    pair<const KeyType, ValueType>& operator*() {
      return phash_map_->all_[pos.first][pos.second];
    }
    pair<const KeyType, ValueType>* operator->() {
      return &(phash_map_->all_[pos.first][pos.second]);
    }

   private:
    pair<int, int> pos_;
    HashMap* phash_map_;
  };
  iterator begin() { return iterator(findNext(kBeforeBeginPos), this); }
  iterator end() { return iterator(kAfterEndPos, this); }

//  Same as iterator but doesn't allow changes
  class const_iterator {
   public:
    const_iterator() {}
    const_iterator(pair<int, int> _pos, const HashMap* phash_map) : pos(_pos), phash_map_(phash_map) {}
    bool operator==(const const_iterator& other) { return pos == other.pos; }
    bool operator!=(const const_iterator& other) { return !(pos == other.pos); }
    const_iterator operator++(int foo) {
      const_iterator tmp = const_iterator(pos, phash_map_);
      pos = phash_map_->findNext(pos);
      return tmp;
    }
    const_iterator operator++() {
      pos = phash_map_->findNext(pos);
      return *this;
    }
    const pair<const KeyType, ValueType>& operator*() {
      return phash_map_->all_[pos.first][pos.second];
    }
    const pair<const KeyType, ValueType>* operator->() {
      return &(phash_map_->all_[pos.first][pos.second]);
    }

   private:
    pair<int, int> pos;
    const HashMap* phash_map_;
  };
  const_iterator begin() const { return const_iterator(findNext(kBeforeBeginPos), this); }
  const_iterator end() const { return const_iterator(kAfterEndPos, this); }
  iterator find(KeyType x) {
    size_t ind = hasher_(x) % capacity_;
    for (size_t i = 0; i < all_[ind].size(); ++i) {
      if (x == all_[ind][i].first) {
        return iterator({static_cast<int>(ind), i}, this);
      }
    }
    return end();
  }

  const_iterator find(KeyType x) const {
    size_t ind = hasher_(x) % capacity_;
    for (size_t i = 0; i < all_[ind].size(); ++i) {
      if (x == all_[ind][i].first) {
        return const_iterator({static_cast<int>(ind), i}, this);
      }
    }
    return end();
  }

 private:
  // indicates the position before the first element
  static constexpr pair<int,int> kBeforeBeginPos = {-1, 0};
  // indicates the position after the last element
  static constexpr pair<int,int> kAfterEndPos = {-2, 0};
  size_t cnt_elements_ = 0;
  size_t capacity_ = kMinCapacity;
  vector<vector<pair<const KeyType, ValueType> > > all_;
  Hash hasher_;

  pair<int, int> findNext(pair < int, int > pos) const {
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
//  Two conditions trigger the call of this function.
//  Expand condition: cnt_elements_ > capacity_ * kExpand.
//  Shrink condition: cnt_elements_ < capacity_ / kShrink.
  void rebuild() {
    int newcapacity = max(static_cast<size_t>(kMinCapacity), cnt_elements_ * kExpand);
    capacity_ = newcapacity;
    vector<vector<pair<const KeyType, ValueType> > > nall(capacity_);
    for (size_t i = 0; i < all_.size(); ++i) {
      for (size_t j = 0; j < all_[i].size(); ++j) {
        nall[hasher_(all_[i][j].first) % capacity_].push_back(all_[i][j]);
      }
    }
    all_.clear();
    all_.resize(capacity_);
    for (size_t i = 0; i < capacity_; ++i) {
      for (size_t j = 0; j < nall[i].size(); ++j) {
        all_[i].push_back(nall[i][j]);
      }
    }
  }
};
