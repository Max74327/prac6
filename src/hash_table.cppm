module;

#include <cstddef>
#include <cstdint>
#include <format>
#include <functional>
#include <list>
#include <optional>
#include <ostream>
#include <vector>

export module hash_table;

import record_concept;

export template <RecordType Rec> class HashTable {
public:
  struct Entry {
    std::uint64_t key;
    std::size_t record_no;
  };

  explicit HashTable(std::size_t init_capacity = 11) : buckets_(next_prime(init_capacity)) {}

  // Вставить пару (key, record_no). Если ключ уже есть — обновить record_no.
  // Возвращает true, если добавлен новый ключ; false — если обновлён.
  bool insert(std::uint64_t key, std::size_t record_no) {
    auto &chain = buckets_[bucket_of(key)];
    for (auto &e : chain) {
      if (e.key == key) {
        e.record_no = record_no;
        return false;
      }
    }
    chain.push_back({key, record_no});
    ++size_;
    if (load_factor() > LOAD_FACTOR_MAX)
      rehash(next_prime(capacity() * 2));
    return true;
  }

  bool erase(std::uint64_t key) {
    auto &chain = buckets_[bucket_of(key)];
    for (auto it = chain.begin(); it != chain.end(); ++it) {
      if (it->key == key) {
        chain.erase(it);
        --size_;
        return true;
      }
    }
    return false;
  }

  std::optional<std::size_t> find(std::uint64_t key) const {
    const auto &chain = buckets_[bucket_of(key)];
    for (const auto &e : chain) {
      if (e.key == key)
        return e.record_no;
    }
    return std::nullopt;
  }

  // Число сравнений ключей при поиске (не считает вычисление хеша).
  // Успех  -> позиция ключа в цепочке (1-based).
  // Неудача -> длина цепочки.
  std::size_t comparisons_on_find(std::uint64_t key) const {
    const auto &chain = buckets_[bucket_of(key)];
    std::size_t cmp = 0;
    for (const auto &e : chain) {
      ++cmp;
      if (e.key == key)
        return cmp;
    }
    return cmp;
  }

  void rehash(std::size_t new_capacity) {
    std::vector<std::list<Entry>> nb(next_prime(new_capacity));
    for (auto &chain : buckets_) {
      for (auto &e : chain)
        nb[Rec::hash_key(e.key) % nb.size()].push_back(e);
    }
    buckets_ = std::move(nb);
  }

  void clear() {
    for (auto &c : buckets_)
      c.clear();
    size_ = 0;
  }

  void print(std::ostream &os) const {
    for (std::size_t i = 0; i < buckets_.size(); ++i) {
      os << std::format("[{}]", i);
      for (const auto &e : buckets_[i])
        os << std::format(" -> {{{}, idx={}}}", e.key, e.record_no);
      os << '\n';
    }
    os << std::format("size={} capacity={} load={:.3f}\n", size_, capacity(), load_factor());
  }

  double load_factor() const { return double(size_) / capacity(); }
  std::size_t size() const { return size_; }
  std::size_t capacity() const { return buckets_.size(); }

  static constexpr double LOAD_FACTOR_MAX = 0.75;

private:
  std::size_t bucket_of(std::uint64_t key) const { return Rec::hash_key(key) % buckets_.size(); }

  static std::size_t next_prime(std::size_t n) {
    auto is_prime = [](std::size_t x) {
      if (x < 2)
        return false;
      if (x % 2 == 0)
        return x == 2;
      for (std::size_t d = 3; d * d <= x; d += 2)
        if (x % d == 0)
          return false;
      return true;
    };
    while (!is_prime(n))
      ++n;
    return n;
  }

  std::vector<std::list<Entry>> buckets_;
  std::size_t size_ = 0;
};
