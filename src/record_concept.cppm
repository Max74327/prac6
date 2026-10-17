module;
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <random>
#include <type_traits>

export module record_concept;

export template <typename T>
concept RecordType =
    requires(const T &r, T &m, std::uint64_t k, std::size_t i, std::mt19937_64 &eng) {
      { r.key() } -> std::convertible_to<std::uint64_t>;
      { T::hash_key(k) } -> std::convertible_to<std::size_t>;
      { r.deleted } -> std::convertible_to<bool>;
      { m.deleted = true };
      requires std::is_trivially_copyable_v<T>;
      { m.random_fill(eng, k) };
    };
