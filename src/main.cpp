// Практическая работа 6_1. Вариант 4: владелец автомобиля (цепочки).
//
// Меню:
//   1 — задание 1: HashTable в памяти (20 записей)
//   2 — задание 2: BinaryFile + FileHashIndex
//   3 — табл. 11: N = 100/1000/10000, сравнения при поиске
//   0 — выход
//
// Все ключи — std::uint64_t. Ввод — десятичное число.
// Вывод — std::print / std::println (C++23).

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <print>
#include <string>
#include <vector>

import record_select;
import hash_table;
import file_io;
import file_hash;
import record_concept;

namespace {

// ---------------------------------------------------------------------------
// Утилиты
// ---------------------------------------------------------------------------

std::uint64_t read_key() {
  std::uint64_t k{};
  std::print("key> ");
  if (!(std::cin >> k)) {
    std::cin.clear();
    std::cin.ignore(1000, '\n');
    return 0;
  }
  std::cin.ignore(1000, '\n');
  return k;
}

// ---------------------------------------------------------------------------
// Задание 1: HashTable в памяти
// ---------------------------------------------------------------------------

void task1() {
  // 20 записей с нелинейными ключами.
  // 11, 22, 33 — кратны 11: при capacity=11 попадут в бакет 0 (h(k)%11==0).
  std::vector<Record> arr;
  arr.reserve(20);
  for (std::size_t i = 0; i < 20; ++i) {
    std::uint64_t k = 100 + i * 37;
    if (i == 3)
      k = 11;
    if (i == 7)
      k = 22;
    if (i == 11)
      k = 33;
    arr.push_back(Record::make_demo(k, i));
  }

  std::println("\nИсходный массив (20 записей):");
  for (std::size_t i = 0; i < arr.size(); ++i)
    std::println("  [{}] {}", i, arr[i]);

  HashTable<Record> ht(11);

  // Сначала 8 вставок — до rehash, чтобы был лог «до рехеширования».
  for (std::size_t i = 0; i < 8; ++i)
    ht.insert(arr[i].key(), i);
  std::println("\n=== До rehash (8 вставок, capacity=11) ===");
  ht.print(std::cout); // print таблицы — через ostream (см. ниже)

  // Остальные 12 — сработает rehash (11 -> 23 -> 47).
  for (std::size_t i = 8; i < arr.size(); ++i)
    ht.insert(arr[i].key(), i);
  std::println("\n=== После вставки 20 записей (после rehash) ===");
  ht.print(std::cout);

  // Поиск: без коллизии, с коллизией, отсутствующий.
  for (std::uint64_t k : {arr[0].key(), arr[3].key(), std::uint64_t{999}}) {
    auto no = ht.find(k);
    auto cmp = ht.comparisons_on_find(k);
    if (no)
      std::println("find({}): idx={} ({} cmp)", k, *no, cmp);
    else
      std::println("find({}): not found ({} cmp)", k, cmp);
  }

  // Удаление и повторный поиск.
  std::println("\nerase({}): {}", arr[3].key(), ht.erase(arr[3].key()) ? "ok" : "not found");
  auto after = ht.find(arr[3].key());
  std::println("find({}) after erase: {}", arr[3].key(), after ? "found" : "not found");
  std::println("\n=== После удаления ключа 11 ===");
  ht.print(std::cout);

  // Принудительный rehash вниз: 47 -> 23 (демонстрация).
  ht.rehash(23);
  std::println("\n=== После rehash (capacity=23) ===");
  ht.print(std::cout);
}

// ---------------------------------------------------------------------------
// Задание 2: BinaryFile + FileHashIndex
// ---------------------------------------------------------------------------

void task2() {
  const std::string path = "data/file_hash.bin";
  std::filesystem::create_directories("data");

  BinaryFile<Record> file(path);
  file.create_empty();

  FileHashIndex<Record> idx(file);

  // 1. Вставить 10 записей.
  for (std::size_t i = 0; i < 10; ++i) {
    std::uint64_t k = 100 + i * 37;
    if (i == 3)
      k = 11; // коллизионный
    idx.insert(Record::make_demo(k, i));
  }
  std::println("\nafter insert: file={} alive={} cap={} load={:.3f}", file.count(), idx.size(),
               idx.capacity(), idx.load_factor());

  // 2. Поиск: record_no берём из table_, запись — из файла.
  for (std::uint64_t k : {std::uint64_t{100}, std::uint64_t{11}, std::uint64_t{999}}) {
    auto rec = idx.find(k);
    auto no = idx.table().find(k);
    auto cmp = idx.comparisons_on_find(k);
    if (rec && no)
      std::println("find({}): record_no={} ({} cmp) {}", k, *no, cmp, *rec);
    else
      std::println("find({}): not found ({} cmp)", k, cmp);
  }

  // 3. Tombstone-удаление.
  std::println("\nremove(11): {}", idx.remove(11) ? "ok" : "not found");
  auto after = idx.find(11);
  std::println("find(11) after remove: {}", after ? "found" : "not found");
  std::println("file={} alive={} (tombstone remains on disk)", file.count(), idx.size());

  // 4. Компактификация.
  idx.compact();
  std::println("after compact: file={} alive={} cap={} load={:.3f}", file.count(), idx.size(),
               idx.capacity(), idx.load_factor());

  // 5. Поиск после компактификации.
  auto r100 = idx.find(100);
  auto n100 = idx.table().find(100);
  if (r100 && n100)
    std::println("find(100) after compact: record_no={} {}", *n100, *r100);
  else
    std::println("find(100) after compact: not found");
}

// ---------------------------------------------------------------------------
// Табл. 11: N = 100/1000/10000, сравнения при поиске
// ---------------------------------------------------------------------------

void run_table_11() {
  std::println("\n=== Табл. 11: число сравнений при поиске ===");
  std::println("{:>6} | {:>14} | {:>14} | {:>14}", "N", "first", "middle", "last");
  std::println("{}", std::string(6 + 14 + 14 + 14 + 9, '-'));

  for (std::size_t N : {std::size_t{100}, std::size_t{1000}, std::size_t{10000}}) {
    const std::string path = std::format("data/n{}.bin", N);
    BinaryFile<Record> file(path);
    if (file.count() != N) {
      std::println("N={}: file '{}' has {} records, expected {} "
                   "(run gen {} {} first)",
                   N, path, file.count(), N, N, path);
      continue;
    }

    FileHashIndex<Record> idx(file);

    std::size_t positions[] = {0, N / 2, N - 1};
    std::size_t cmps[3] = {0, 0, 0};
    for (int j = 0; j < 3; ++j) {
      auto r = file.read(positions[j]);
      if (r)
        cmps[j] = idx.comparisons_on_find(r->key());
    }

    std::println("{:>6} | {:>14} | {:>14} | {:>14}", N, cmps[0], cmps[1], cmps[2]);
  }
  std::println("\n(если файлы отсутствуют — сгенерировать:\n"
               "  ./build/clang-debug/gen 100    data/n100.bin\n"
               "  ./build/clang-debug/gen 1000   data/n1000.bin\n"
               "  ./build/clang-debug/gen 10000  data/n10000.bin )");
}

// ---------------------------------------------------------------------------
// Меню
// ---------------------------------------------------------------------------

void print_menu() {
  std::println("\n=== Практическая работа 6_1 (вариант 4: владелец автомобиля) ===\n"
               "  1 — задание 1: хеш-таблица в памяти (20 записей)\n"
               "  2 — задание 2: бинарный файл + хеш-таблица\n"
               "  3 — табл. 11: N = 100/1000/10000\n"
               "  0 — выход");
  std::print("choice> ");
}

} // namespace

int main() {
  while (true) {
    print_menu();
    std::string line;
    if (!std::getline(std::cin, line))
      break;
    if (line == "0")
      break;
    if (line == "1") {
      task1();
      continue;
    }
    if (line == "2") {
      task2();
      continue;
    }
    if (line == "3") {
      run_table_11();
      continue;
    }
    std::println("неизвестная команда: {}", line);
  }
  return 0;
}
