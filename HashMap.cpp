#include <algorithm>

#include <vector>

#include <utility>


using namespace std;

template < class KeyType, class ValueType, class Hash = std::hash < KeyType >>
  class HashMap {
    private:
      const size_t min_capacity = 4;
    size_t cnt_elements = 0;
    size_t capacity = min_capacity;
    vector < vector < pair <
      const KeyType, ValueType > > > all;
    Hash hasher;

    public:
      HashMap(Hash nhasher = Hash()): hasher(nhasher) {
        all.resize(capacity);
      }

    template < typename iterator >
      HashMap(iterator b, iterator s, Hash nhasher = Hash()): hasher(nhasher) {
        all.resize(capacity);
        while (b != s) {
          insert(*b);
          b++;
        }
      }

    HashMap(std::initializer_list < pair <
      const KeyType, ValueType >> init, Hash nhasher = Hash()):
      hasher(nhasher) {
      all.resize(capacity);
      auto b = init.begin();
      auto s = init.end();
      while (b != s) {
        insert(*b);
        b++;
      }
    }

    HashMap & operator = (HashMap & h1) {
      if (this != & h1) {
        hasher = h1.hasher;
        all.resize(h1.all.size());
        for (size_t i = 0; i < all.size(); ++i) {
          for (size_t j = 0; j < h1.all[i].size(); ++j) {
            all[i].push_back(h1.all[i][j]);
          }
        }
      }
      return *this;
    }

    Hash hash_function() const {
      return hasher;
    }

    class iterator {
      private:
        pair < int, int > pos;
      HashMap * h;

      public:
        iterator() {}
      iterator(pair < int, int > _pos, HashMap * _h): pos(_pos), h(_h) {}
      bool operator == (const iterator & other) {
        return pos == other.pos;
      }
      bool operator != (const iterator & other) {
        return !(pos == other.pos);
      }
      iterator operator++(int asd) {
        iterator tmp = iterator(pos, h);
        pos = h->findnext(pos.first, pos.second);
        return tmp;
      }
      iterator operator++() {
        pos = h->findnext(pos.first, pos.second);
        return *this;
      }
      pair <
        const KeyType, ValueType > & operator * () {
          return h ->all[pos.first][pos.second];
        }
      pair <
        const KeyType, ValueType > * operator -> () {
          return &(h ->all[pos.first][pos.second]);
        }
    };
    iterator begin() {
      return iterator(findnext(0, -1), this);
    }
    iterator end() {
      return iterator({
        capacity,
        -1
      }, this);
    }

    class const_iterator {
      private:
        pair < int, int > pos;
      const HashMap * h;

      public:
        const_iterator() {}
      const_iterator(pair < int, int > _pos,
        const HashMap * _h): pos(_pos), h(_h) {}
      bool operator == (const const_iterator & other) {
        return pos == other.pos;
      }
      bool operator != (const const_iterator & other) {
        return !(pos == other.pos);
      }
      const_iterator operator++(int asd) {
        const_iterator tmp = const_iterator(pos, h);
        pos = h->findnext(pos.first, pos.second);
        return tmp;
      }
      const_iterator operator++() {
        pos = h->findnext(pos.first, pos.second);
        return *this;
      }
      const pair <
        const KeyType, ValueType > & operator * () {
          return h->all[pos.first][pos.second];
        }
      const pair <
        const KeyType, ValueType > * operator-> () {
          return &(h->all[pos.first][pos.second]);
        }
    };
    const_iterator begin() const {
      return const_iterator(findnext(0, -1), this);
    }
    const_iterator end() const {
      return const_iterator({
        capacity,
        -1
      }, this);
    }

    pair < int, int > findnext(int pos1, int pos2) const {
      // case of begin
      if (pos1 == 0 && pos2 == -1) {
        for (size_t i = 0; i < all.size(); ++i) {
          if (all[i].size() != 0) {
            return {i, 0};
          }
        }
      }
      if (pos2 + 1 < static_cast < int >(all[pos1].size())) {
        pos2++;
        return {pos1, pos2};
      } else {
        for (size_t i = pos1 + 1; i < capacity; ++i) {
          if (all[i].size() != 0) {
            return {i, 0};
          }
        }
        return {capacity, -1};
      }
    }

    void rebuild() {
      // newcapacity is floor(cnt_elements * 0.5)
      int newcapacity = ((cnt_elements * 2 + 4 - 1) / 4) * 4;
      capacity = newcapacity;
      vector < vector < pair <
        const KeyType, ValueType > > > nall(capacity);
      for (size_t i = 0; i < all.size(); ++i) {
        for (size_t j = 0; j < all[i].size(); ++j) {
          nall[hasher(all[i][j].first) % capacity].push_back(all[i][j]);
        }
      }
      all.clear();
      all.resize(capacity);
      for (size_t i = 0; i < capacity; ++i) {
        for (size_t j = 0; j < nall[i].size(); ++j) {
          all[i].push_back(nall[i][j]);
        }
      }
    }

    void insert(pair <
      const KeyType, ValueType > x) {
      bool fnd = 0;
      size_t ind = hasher(x.first) % capacity;
      for (size_t i = 0; i < all[ind].size(); ++i) {
        if (all[ind][i].first == x.first) {
          fnd = 1;
        }
      }
      if (!fnd) {
        all[ind].push_back(x);
        cnt_elements++;
        if (3 * cnt_elements > 4 * capacity) {
          rebuild();
        }
      }
    }

    void erase(KeyType x) {
      size_t ind = hasher(x) % capacity;
      bool fnd = 0;
      vector < pair < const KeyType, ValueType > > tmpall;
      for (size_t i = 0; i < all[ind].size(); ++i) {
        if (all[ind][i].first == x) {
          fnd = 1;
        } else {
          tmpall.push_back(all[ind][i]);
        }
      }
      if (!fnd) {
        return;
      }
      all[ind].clear();
      for (size_t i = 0; i < tmpall.size(); ++i) {
        all[ind].push_back(tmpall[i]);
      }
      cnt_elements--;
      if (capacity > min_capacity && cnt_elements < capacity / 4) {
        rebuild();
      }
    }

    iterator find(KeyType x) {
      size_t ind = hasher(x) % capacity;
      for (size_t i = 0; i < all[ind].size(); ++i) {
        if (x == all[ind][i].first) {
          return iterator({static_cast<int>(ind), i}, this);
        }
      }
      return end();
    }

    const_iterator find(KeyType x) const {
      size_t ind = hasher(x) % capacity;
      for (size_t i = 0; i < all[ind].size(); ++i) {
        if (x == all[ind][i].first) {
          return const_iterator({static_cast<int>(ind), i}, this);
        }
      }
      return end();
    }

    ValueType & operator[](KeyType x) {
      iterator cur = find(x);
      if (cur != end()) {
        return cur->second;
      } else {
        insert({x, ValueType()});
        iterator tmp = find(x);
        return tmp->second;
      }
    }

    const ValueType & at(KeyType x) const {
      const_iterator cur = find(x);
      if (cur != end()) {
        return cur->second;
      } else {
        throw out_of_range("Element is not in the HashMap");
      }
    }

    void clear() {
      all.clear();
      capacity = min_capacity;
      cnt_elements = 0;
      all.resize(capacity);
    }

    int size() const {
      return cnt_elements;
    }

    bool empty() const {
      return (cnt_elements == 0);
    }
  };
