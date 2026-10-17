module;

#include <cstddef>
#include <cstdint>
#include <optional>

export module file_hash;

import record_concept;
import file_io;
import hash_table;

export template <RecordType Rec> class FileHashIndex {
public:
  explicit FileHashIndex(BinaryFile<Rec> &f) : file_(f) { rebuild(); }

  // Полная перестройка индекса: пройти файл, вставить все ЖИВЫЕ записи.
  // Вызывать после compact() или при старте.
  void rebuild() {
    table_.clear();
    std::size_t n = file_.count();
    for (std::size_t i = 0; i < n; ++i) {
      auto r = file_.read(i);
      if (r && !r->deleted)
        table_.insert(r->key(), i);
    }
  }

  // Поиск: ключ -> record_no -> прямой доступ к записи файла.
  // Если ключ в таблице есть, запись в файле жива (tombstone не мешает).
  std::optional<Rec> find(std::uint64_t key) {
    auto no = table_.find(key);
    if (!no)
      return std::nullopt;
    return file_.read(*no);
  }

  std::size_t comparisons_on_find(std::uint64_t key) const {
    return table_.comparisons_on_find(key);
  }

  // Вставка: только append в конец; существующие record_no не меняются.
  std::size_t insert(const Rec &r) {
    auto idx = file_.append(r);
    table_.insert(r.key(), idx);
    return idx;
  }

  // Tombstone-удаление: пометить в файле + убрать из таблицы.
  // record_no остальных не меняются -> rebuild не нужен.
  bool remove(std::uint64_t key) {
    auto no = table_.find(key);
    if (!no)
      return false;
    if (!file_.mark_deleted(*no))
      return false;
    table_.erase(key);
    return true;
  }

  // Физическая компактификация: убирает tombstone-записи.
  // Меняет record_no -> перестраивает индекс.
  void compact() {
    file_.compact();
    rebuild();
  }

  const HashTable<Rec> &table() const { return table_; }
  std::size_t size() const { return table_.size(); }
  std::size_t capacity() const { return table_.capacity(); }
  double load_factor() const { return table_.load_factor(); }

private:
  BinaryFile<Rec> &file_;
  HashTable<Rec> table_;
};
