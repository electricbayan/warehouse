#pragma once

#include "warehouse.hpp"

size_t MIN_CAPACITY = 8;
double MAX_LOAD_FACTOR = 0.75;


template <class T, class K> class HashMap {
  struct Node {
    T key;
    K *value;
    Node *next;
  };

  Node **buckets;
  size_t bucket_count;
  size_t size;

  size_t hash_key(const T &key) const {
    size_t hash = 5381;
    if (key.name) {
      for (const char *p = key.name; *p; ++p) {
        hash = ((hash << 5) + hash) + static_cast<unsigned char>(*p);
      }
    }
    return hash % bucket_count;
  };
  bool keys_equal(const T &a, const T &b) const {
    if (a.name == b.name) {
      return true;
    }
    if (!a.name || !b.name) {
      return false;
    }
    return std::strcmp(a.name, b.name) == 0;
  };
  void rehash() {
    const size_t old_count = bucket_count;
    Node **old_buckets = buckets;

    bucket_count *= 2;
    buckets = new Node *[bucket_count]();
    size = 0;

    for (size_t i = 0; i < old_count; i++) {
      Node *node = old_buckets[i];
      while (node) {
        insert(node->key, node->value);
        Node *next = node->next;
        delete node;
        node = next;
      }
    }

    delete[] old_buckets;
  };

public:
  explicit HashMap(size_t initial_capacity = 16)
      : buckets(nullptr), bucket_count(0), size(0) {
    if (initial_capacity < MIN_CAPACITY) {
      initial_capacity = MIN_CAPACITY;
    }
    bucket_count = initial_capacity;
    buckets = new Node *[bucket_count]();
  };

  ~HashMap() {
    for (size_t i = 0; i < bucket_count; i++) {
      Node *node = buckets[i];
      while (node) {
        Node *next = node->next;
        delete node;
        node = next;
      }
    }
    delete[] buckets;
  };

  HashMap(const HashMap &) = delete;
  HashMap &operator=(const HashMap &) = delete;

  void insert(const T &key, K *value) {
    const size_t index = hash_key(key);
    for (Node *node = buckets[index]; node; node = node->next) {
      if (keys_equal(node->key, key)) {
        node->value = value;
        return;
      }
    }

    Node *node = new Node{key, value, buckets[index]};
    buckets[index] = node;
    size++;

    if (static_cast<double>(size) / bucket_count > MAX_LOAD_FACTOR) {
      rehash();
    }
  };
  K *get(const T &key) const {
    const size_t index = hash_key(key);
    for (Node *node = buckets[index]; node; node = node->next) {
      if (keys_equal(node->key, key)) {
        return node->value;
      }
    }
    return nullptr;
  };
  K *get(const char *name) const {
    T probe{};
    probe.name = name;
    return get(probe);
  };
  bool contains(const T &key) const { return get(key) != nullptr; };
  bool remove(const T &key) {
    const size_t index = hash_key(key);
    Node *prev = nullptr;
    for (Node *node = buckets[index]; node; node = node->next) {
      if (keys_equal(node->key, key)) {
        if (prev) {
          prev->next = node->next;
        } else {
          buckets[index] = node->next;
        }
        delete node;
        size--;
        return true;
      }
      prev = node;
    }
    return false;
  };
  size_t count() const { return size; };
};
