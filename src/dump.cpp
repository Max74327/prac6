// usage: dump [--table]
//   без флага: "#<i> <полная запись>"
//   --table:   markdown-таблица (для отчёта)
//
// stdin: бинарный поток записей Record
//
#include <cstddef>
#include <cstdint>
#include <format>
#include <iostream>
#include <string>
#include <string_view>

import record_select;
import bin_io;

namespace {

void print_header() {
  std::cout << "| # | plate | brand | owner | stolen |\n"
            << "|---|-------|-------|-------|--------|\n";
}

} // namespace

int main(int argc, char **argv) {
  bool table = false;
  for (int i = 1; i < argc; ++i) {
    std::string_view a = argv[i];
    if (a == "--table")
      table = true;
    else {
      std::cerr << std::format("dump: unknown arg '{}'\n", a);
      return 2;
    }
  }

  if (table)
    print_header();

  std::size_t i = 0;
  Record rec{};
  while (read_record(std::cin, rec)) {
    if (table)
      std::cout << std::format("| {} {:t}\n", i, rec);
    else
      std::cout << std::format("#{:<4} {}\n", i, rec);
    ++i;
  }

  // диагностика — в stderr, чтобы не мешать пайпу
  std::cerr << std::format("dump: {} records\n", i);
  return 0;
}
