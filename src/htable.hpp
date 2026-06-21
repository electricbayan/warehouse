#ifndef HTABLE
#define HTABLE
#define DEFAULT_CAPACITY 8
#define DEFAULT_BUCKET_COUNT 64
#define DEFAULT_OVERFLOW_CAPACITY 64
#include <boost/hash2/siphash.hpp>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

namespace volkovich {

  class SipHash {
   public:
    size_t operator()(const std::string& s) const {
      boost::hash2::siphash_32 hasher;
      hasher.update(s.data(), s.size());
      return static_cast< size_t >(hasher.result());
    };
  };

  template < class Key, class Value, class Hash, class Equal >
  class HashTable {
    struct Record {
      Key key;
      Value value;
    };

    struct Slot {
      enum class State { EMPTY, OCCUPIED, TOMBSTONE };
      std::optional< Record > data;
      State state = State::EMPTY;
    };

   public:
    class iterator {
      friend class HashTable;
      Slot *cur_, *buckets_end_;
      Slot *overflow_begin_, *overflow_end_;
      bool in_overflow = false;

     public:
      void findNotEmpty() {
        while (true) {
          Slot* end = in_overflow ? overflow_end_ : buckets_end_;
          while (cur_ != end) {
            if (cur_->state == Slot::State::OCCUPIED) return;
            ++cur_;
          }
          if (!in_overflow) {
            in_overflow = true;
            cur_ = overflow_begin_;
            continue;
          }
          return;
        }
      }

      iterator(Slot* start_, size_t slot_count, Slot* overflow, size_t overflow_size)
          : cur_(start_),
            buckets_end_(start_ + slot_count),
            overflow_begin_(overflow),
            overflow_end_(overflow + overflow_size) {
        findNotEmpty();
      }

      iterator& operator++() {
        if (!in_overflow && cur_ == buckets_end_) {
          in_overflow = true;
          cur_ = overflow_begin_;
        } else {
          cur_++;
        }
        findNotEmpty();
        return *this;
      };
      iterator operator++(int) {
        iterator tmp = *this;
        ++(*this);
        return tmp;
      }
      bool operator==(const iterator& other) const {
        return cur_ == other.cur_;
      }
      bool operator!=(const iterator& other) const {
        return cur_ != other.cur_;
      }
      Record& operator*() const {
        return *cur_->data;
      }
      Record* operator->() const {
        return &*cur_->data;
      }
    };

    class constIterator {
      friend class HashTable;
      const Slot *cur_, *buckets_end_;
      const Slot *overflow_begin_, *overflow_end_;
      bool in_overflow = false;

     public:
      void findNotEmpty() {
        while (true) {
          const Slot* end = in_overflow ? overflow_end_ : buckets_end_;
          while (cur_ != end) {
            if (cur_->state == Slot::State::OCCUPIED) return;
            ++cur_;
          }
          if (!in_overflow) {
            in_overflow = true;
            cur_ = overflow_begin_;
            continue;
          }
          return;
        }
      }

      constIterator(
          const Slot* start_, size_t slot_count, const Slot* overflow, size_t overflow_size)
          : cur_(start_),
            buckets_end_(start_ + slot_count),
            overflow_begin_(overflow),
            overflow_end_(overflow + overflow_size) {
        findNotEmpty();
      }

      constIterator& operator++() {
        if (!in_overflow && cur_ == buckets_end_) {
          in_overflow = true;
          cur_ = overflow_begin_;
        } else {
          cur_++;
        }
        findNotEmpty();
        return *this;
      };
      constIterator operator++(int) {
        constIterator tmp = *this;
        ++(*this);
        return tmp;
      }
      bool operator==(const constIterator& other) const {
        return cur_ == other.cur_;
      }
      bool operator!=(const constIterator& other) const {
        return cur_ != other.cur_;
      }
      const Record& operator*() const {
        return *cur_->data;
      }
      const Record* operator->() const {
        return &*cur_->data;
      }
    };

   private:
    Slot *slots_{}, *overflow_{};
    size_t buckets_count_ = DEFAULT_BUCKET_COUNT, bucket_capacity_ = DEFAULT_CAPACITY,
           overflow_capacity_ = DEFAULT_OVERFLOW_CAPACITY, real_size_ = 0, real_overflow_size = 0;
    Hash hashf_;
    Equal eq_;

    size_t slotsCapacity() const {
      return bucket_capacity_ * buckets_count_;
    };

    size_t gcd(size_t left, size_t right) const {
      while (right != 0) {
        size_t tmp = left % right;
        left = right;
        right = tmp;
      }
      return left;
    }

    size_t mixedHash(size_t hash) const {
      hash ^= hash >> 33;
      hash *= static_cast< size_t >(0xff51afd7ed558ccdULL);
      hash ^= hash >> 33;
      hash *= static_cast< size_t >(0xc4ceb9fe1a85ec53ULL);
      hash ^= hash >> 33;
      return hash;
    }

    size_t primaryHash(const Key& key) const {
      return hashf_(key) % slotsCapacity();
    };

    size_t secondaryHash(const Key& key) const {
      size_t capacity = slotsCapacity();
      if (capacity <= 1) {
        return 1;
      }
      size_t step = 1 + mixedHash(hashf_(key)) % (capacity - 1);
      while (gcd(step, capacity) != 1) {
        step++;
        if (step == capacity) {
          step = 1;
        }
      }
      return step;
    };

    size_t probeIndex(size_t start, size_t step, size_t probe) const {
      size_t capacity = slotsCapacity();
      return (start + (probe * step) % capacity) % capacity;
    }

    Slot* getFromSlots(const Key& key) {
      size_t start = primaryHash(key);
      size_t step = secondaryHash(key);
      for (size_t i = 0; i < slotsCapacity(); i++) {
        Slot& s = slots_[probeIndex(start, step, i)];
        if (s.state == Slot::State::EMPTY) {
          return nullptr;
        }
        if (s.state == Slot::State::OCCUPIED && eq_(s.data->key, key)) {
          return &s;
        }
      }
      return nullptr;
    }

    const Slot* getFromSlots(const Key& key) const {
      size_t start = primaryHash(key);
      size_t step = secondaryHash(key);
      for (size_t i = 0; i < slotsCapacity(); i++) {
        const Slot& s = slots_[probeIndex(start, step, i)];
        if (s.state == Slot::State::EMPTY) {
          return nullptr;
        }
        if (s.state == Slot::State::OCCUPIED && eq_(s.data->key, key)) {
          return &s;
        }
      }
      return nullptr;
    }

    Slot* get(const Key& key) {
      Slot* s = getFromSlots(key);
      return s ? s : getFromOverflow(key);
    }

    Slot* get(const Key& key, bool& overflow) {
      Slot* s = getFromSlots(key);
      if (s) {
        overflow = false;
        return s;
      }
      overflow = true;
      return getFromOverflow(key);
    }

    const Slot* get(const Key& key) const {
      const Slot* s = getFromSlots(key);
      return s ? s : getFromOverflow(key);
    }

    Slot* getFromOverflow(const Key& key) {
      for (size_t i = 0; i < overflow_capacity_; i++) {
        Slot& s = overflow_[i];
        if (s.state == Slot::State::OCCUPIED && eq_(s.data->key, key)) {
          return &s;
        }
      }
      return nullptr;
    };

    const Slot* getFromOverflow(const Key& key) const {
      for (size_t i = 0; i < overflow_capacity_; i++) {
        const Slot& s = overflow_[i];
        if (s.state == Slot::State::OCCUPIED && eq_(s.data->key, key)) {
          return &s;
        }
      }
      return nullptr;
    };

    Slot* getEmptySlotSpace(const Key& key) {
      size_t start = primaryHash(key);
      size_t step = secondaryHash(key);
      Slot* tombstone = nullptr;
      for (size_t i = 0; i < slotsCapacity(); i++) {
        Slot& s = slots_[probeIndex(start, step, i)];
        if (s.state == Slot::State::TOMBSTONE && !tombstone) {
          tombstone = &s;
        } else if (s.state == Slot::State::EMPTY) {
          return tombstone ? tombstone : &s;
        }
      }
      return tombstone;
    }

    Slot* getEmptyOverflowSpace() {
      for (size_t i = 0; i < overflow_capacity_; i++) {
        Slot& s = overflow_[i];
        if (s.state == Slot::State::EMPTY || s.state == Slot::State::TOMBSTONE) {
          return &s;
        }
      }
      return nullptr;
    }

    double loadFactor() const {
      return static_cast< double >(real_size_) /
             static_cast< double >(bucket_capacity_ * buckets_count_);
    }
    double overflowLoadFactor() const {
      return static_cast< double >(real_overflow_size) / static_cast< double >(overflow_capacity_);
    }

   public:
    iterator begin() {
      return iterator(slots_, bucket_capacity_ * buckets_count_, overflow_, overflow_capacity_);
    };
    iterator end() {
      iterator it(slots_, bucket_capacity_ * buckets_count_, overflow_, overflow_capacity_);
      it.in_overflow = true;
      it.cur_ = it.overflow_end_;
      return it;
    };
    constIterator begin() const {
      return constIterator(
          slots_, bucket_capacity_ * buckets_count_, overflow_, overflow_capacity_);
    };
    constIterator end() const {
      constIterator it(slots_, bucket_capacity_ * buckets_count_, overflow_, overflow_capacity_);
      it.in_overflow = true;
      it.cur_ = it.overflow_end_;
      return it;
    };
    bool add(const Key& k, const Value& v) {
      if (loadFactor() > 0.75 || overflowLoadFactor() > 0.8) {
        rehash(buckets_count_ * 2);
      }
      if (has(k)) {
        return false;
      }
      Slot* dest = getEmptySlotSpace(k);
      if (!dest) {
        dest = getEmptyOverflowSpace();
        if (dest) {
          real_overflow_size++;
        }
      }
      if (!dest) {
        return false;
      }
      dest->data = Record{k, v};
      dest->state = Slot::State::OCCUPIED;
      real_size_++;
      return true;
    };

    Value* find(const Key& key) {
      Slot* s = get(key);
      if (!s) {
        return nullptr;
      }
      return &s->data->value;
    };

    const Value* find(const Key& key) const {
      const Slot* s = get(key);
      if (!s) {
        return nullptr;
      }
      return &s->data->value;
    };

    Value drop(const Key& k) {
      bool overflow;
      Slot* s = get(k, overflow);
      if (!s) {
        throw std::out_of_range("Key not found");
      }
      if (overflow) {
        real_overflow_size--;
      }
      Value v = std::move(s->data->value);
      s->data.reset();
      s->state = Slot::State::TOMBSTONE;
      real_size_--;
      return v;
    };
    bool has(const Key& k) const {
      return get(k) != nullptr;
    };
    void rehash(size_t new_bucket_count) {
      HashTable tmp(hashf_, new_bucket_count, overflow_capacity_ * 2, bucket_capacity_);
      for (size_t i = 0; i < bucket_capacity_ * buckets_count_; i++) {
        if (slots_[i].state == Slot::State::OCCUPIED) {
          tmp.add(slots_[i].data->key, slots_[i].data->value);
        }
      }
      for (size_t i = 0; i < overflow_capacity_; i++) {
        if (overflow_[i].state == Slot::State::OCCUPIED) {
          tmp.add(overflow_[i].data->key, overflow_[i].data->value);
        }
      }
      swap(tmp);
    };

    HashTable(Hash hash_func, size_t bucket_count = DEFAULT_BUCKET_COUNT,
        size_t overflow_capacity = DEFAULT_OVERFLOW_CAPACITY,
        size_t bucket_capacity = DEFAULT_CAPACITY)
        : buckets_count_(bucket_count),
          bucket_capacity_(bucket_capacity),
          overflow_capacity_(overflow_capacity),
          hashf_(hash_func) {
      slots_ = new Slot[bucket_capacity * bucket_count];
      overflow_ = new Slot[overflow_capacity_];
    };
    HashTable()
        : buckets_count_(DEFAULT_BUCKET_COUNT),
          bucket_capacity_(DEFAULT_CAPACITY),
          overflow_capacity_(DEFAULT_OVERFLOW_CAPACITY),
          hashf_() {
      slots_ = new Slot[bucket_capacity_ * buckets_count_];
      overflow_ = new Slot[overflow_capacity_];
    };

    ~HashTable() {
      delete[] slots_;
      delete[] overflow_;
    };

    HashTable(HashTable&& other)
        : slots_(other.slots_),
          overflow_(other.overflow_),
          buckets_count_(other.buckets_count_),
          bucket_capacity_(other.bucket_capacity_),
          overflow_capacity_(other.overflow_capacity_),
          real_size_(other.real_size_),
          real_overflow_size(other.real_overflow_size),
          hashf_(std::move(other.hashf_)),
          eq_(std::move(other.eq_)) {
      other.slots_ = nullptr;
      other.overflow_ = nullptr;
      other.real_size_ = 0;
      other.overflow_capacity_ = 0;
      other.buckets_count_ = 0;
      other.bucket_capacity_ = 0;
      other.real_overflow_size = 0;
    };

    HashTable(const HashTable& other)
        : buckets_count_(other.buckets_count_),
          bucket_capacity_(other.bucket_capacity_),
          overflow_capacity_(other.overflow_capacity_),
          real_size_(other.real_size_),
          real_overflow_size(other.real_overflow_size),
          hashf_(other.hashf_),
          eq_(other.eq_) {
      slots_ = new Slot[other.buckets_count_ * other.bucket_capacity_];
      for (size_t i = 0; i < other.buckets_count_ * other.bucket_capacity_; i++) {
        slots_[i] = other.slots_[i];
      }
      overflow_ = new Slot[other.overflow_capacity_];
      for (size_t i = 0; i < other.overflow_capacity_; i++) {
        overflow_[i] = other.overflow_[i];
      }
    };

    HashTable& operator=(HashTable&& other) {
      swap(other);
      return *this;
    };

    HashTable& operator=(const HashTable& other) {
      if (this == &other) return *this;
      HashTable tmp(other);
      swap(tmp);
      return *this;
    };

    void swap(HashTable& other) noexcept {
      std::swap(slots_, other.slots_);
      std::swap(overflow_, other.overflow_);
      std::swap(bucket_capacity_, other.bucket_capacity_);
      std::swap(buckets_count_, other.buckets_count_);
      std::swap(overflow_capacity_, other.overflow_capacity_);
      std::swap(real_size_, other.real_size_);
      std::swap(hashf_, other.hashf_);
      std::swap(eq_, other.eq_);
      std::swap(real_overflow_size, other.real_overflow_size);
    };

    size_t table_size() const noexcept {
      return real_size_;
    };

    bool isEmpty() const noexcept {
      return real_size_ == 0;
    };
  };

}
#endif
