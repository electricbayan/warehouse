#pragma once

#include "htable.hpp"
#include "warehouse.hpp"
#include <cstring>
#include <stdexcept>
#include <string>
#include <type_traits>

constexpr size_t MIN_CAPACITY = 8;

template < typename T >
const char* hashmap_key_name(const T& key) {
  if constexpr (std::is_same_v< T, const char* >) {
    return key ? key : "";
  } else if constexpr (std::is_same_v< T, std::string >) {
    return key.c_str();
  } else if constexpr (std::is_same_v< decltype(std::declval< T >().name), std::string >) {
    return key.name.c_str();
  } else {
    return key.name ? key.name : "";
  }
}

template < typename T >
bool hashmap_keys_equal(const T& a, const T& b) {
  if constexpr (std::is_same_v< T, std::string >) {
    return a == b;
  } else if constexpr (std::is_same_v< decltype(std::declval< T >().name), std::string >) {
    return a.name == b.name;
  } else if constexpr (std::is_same_v< T, const char* >) {
    if (a == b) {
      return true;
    }
    if (!a || !b) {
      return false;
    }
    return std::strcmp(a, b) == 0;
  } else {
    if (a.name == b.name) {
      return true;
    }
    if (!a.name || !b.name) {
      return false;
    }
    return std::strcmp(a.name, b.name) == 0;
  }
}

template < typename T >
struct HashMapHash {
  size_t operator()(const T& key) const {
    size_t hash = 5381;
    const char* name = hashmap_key_name(key);
    for (const char* p = name; *p; ++p) {
      hash = ((hash << 5) + hash) + static_cast< unsigned char >(*p);
    }
    return hash;
  }
};

template < typename T >
struct HashMapEqual {
  bool operator()(const T& a, const T& b) const {
    return hashmap_keys_equal(a, b);
  }
};

template < class T, class K >
class HashMap {
  using Table = volkovich::HashTable< T, K*, HashMapHash< T >, HashMapEqual< T > >;

  Table table_;

  T key_from_name(const char* name) const {
    if constexpr (std::is_same_v< T, std::string >) {
      return std::string(name ? name : "");
    } else if constexpr (std::is_same_v< T, const char* >) {
      return name;
    } else {
      T probe{};
      if constexpr (std::is_same_v< decltype(probe.name), std::string >) {
        probe.name = std::string(name ? name : "");
      } else {
        probe.name = name;
      }
      return probe;
    }
  }

 public:
  using iterator = typename Table::iterator;
  using constIterator = typename Table::constIterator;

  explicit HashMap(size_t initial_capacity = 16)
      : table_(HashMapHash< T >{},
            initial_capacity < MIN_CAPACITY ? MIN_CAPACITY : initial_capacity,
            initial_capacity < MIN_CAPACITY ? MIN_CAPACITY : initial_capacity, 1) {
  }

  HashMap(const HashMap&) = delete;
  HashMap& operator=(const HashMap&) = delete;

  void insert(const T& key, K* value) {
    if (!value) {
      throw std::invalid_argument("HashMap value must not be null");
    }
    K** stored = table_.find(key);
    if (stored) {
      *stored = value;
      return;
    }
    table_.add(key, value);
  }

  K* get(const T& key) const {
    K* const* value = table_.find(key);
    return value ? *value : nullptr;
  }

  K* get(const char* name) const {
    return get(key_from_name(name));
  }

  bool contains(const T& key) const {
    return table_.has(key);
  }

  bool remove(const T& key) {
    if (!table_.has(key)) {
      return false;
    }
    table_.drop(key);
    return true;
  }

  size_t count() const {
    return table_.table_size();
  }

  iterator begin() {
    return table_.begin();
  }

  iterator end() {
    return table_.end();
  }

  constIterator begin() const {
    return table_.begin();
  }

  constIterator end() const {
    return table_.end();
  }
};
