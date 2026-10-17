module;

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

export module file_io;

import record_concept;

export template <RecordType Rec> class BinaryFile {
public:
  explicit BinaryFile(std::string path) : path_(std::move(path)) {}

  // Создать пустой файл (обрезать, если был).
  bool create_empty() const {
    std::ofstream f(path_, std::ios::binary | std::ios::trunc);
    return static_cast<bool>(f);
  }

  // Число записей = размер файла / sizeof(Rec).
  // Размер записи фиксирован (static_assert в Record), поэтому деление корректно.
  std::size_t count() const {
    if (!std::filesystem::exists(path_))
      return 0;
    auto sz = std::filesystem::file_size(path_);
    return sz / sizeof(Rec);
  }

  // Прямой доступ по номеру записи: seekg(idx * sizeof(Rec)).
  std::optional<Rec> read(std::size_t idx) const {
    std::ifstream f(path_, std::ios::binary);
    if (!f)
      return std::nullopt;
    f.seekg(static_cast<std::streamoff>(idx * sizeof(Rec)));
    Rec r{};
    f.read(reinterpret_cast<char *>(&r), sizeof(Rec));
    if (!f)
      return std::nullopt;
    return r;
  }

  // Перезаписать запись по номеру (in-place).
  bool write(std::size_t idx, const Rec &r) {
    std::fstream f(path_, std::ios::binary | std::ios::in | std::ios::out);
    if (!f)
      return false;
    f.seekp(static_cast<std::streamoff>(idx * sizeof(Rec)));
    f.write(reinterpret_cast<const char *>(&r), sizeof(Rec));
    return static_cast<bool>(f);
  }

  // Добавить в конец; вернуть номер (индекс) новой записи.
  std::size_t append(const Rec &r) {
    std::size_t idx = count();
    std::ofstream f(path_, std::ios::binary | std::ios::app);
    f.write(reinterpret_cast<const char *>(&r), sizeof(Rec));
    return idx;
  }

  // Tombstone-удаление: пометить deleted = true, не сдвигая запись.
  // record_no остальных записей НЕ меняются -> HashTable::rebuild не нужен.
  bool mark_deleted(std::size_t idx) {
    auto r = read(idx);
    if (!r)
      return false;
    r->deleted = true;
    return write(idx, *r);
  }

  // Компактификация: перезаписать файл без tombstone-записей.
  // ВНИМАНИЕ: меняет record_no -> требует HashTable::rebuild.
  // Возвращает новое число записей.
  std::size_t compact() {
    auto n = count();
    std::vector<Rec> alive;
    alive.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
      auto r = read(i);
      if (r && !r->deleted)
        alive.push_back(*r);
    }
    std::ofstream f(path_, std::ios::binary | std::ios::trunc);
    for (const auto &r : alive)
      f.write(reinterpret_cast<const char *>(&r), sizeof(Rec));
    return alive.size();
  }

  const std::string &path() const { return path_; }

private:
  std::string path_;
};
