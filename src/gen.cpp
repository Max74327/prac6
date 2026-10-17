// usage: gen [N] [filename]
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <format>
#include <fstream>
#include <iostream>
#include <random>
#include <ranges>
#include <string>
#include <vector>

import record_select;
import bin_io;

namespace {

struct GenConfig {
  std::size_t n = 100;
  std::string out = "-";
  bool ok = true;
};

GenConfig parse_args(int argc, char **argv) {
  GenConfig cfg;
  if (argc > 1) {
    try {
      cfg.n = std::stoull(argv[1]);
    } catch (const std::exception &e) {
      std::cerr << std::format("gen: invalid N '{}': {}\n", argv[1], e.what());
      cfg.ok = false;
      return cfg;
    }
  }
  if (argc > 2)
    cfg.out = argv[2];
  if (argc > 3) {
    std::cerr << "usage: gen [N] [filename]\n";
    cfg.ok = false;
  }
  return cfg;
}

template <typename Rec, std::uniform_random_bit_generator Engine>
void generate(Engine &eng, std::ostream &out, std::size_t amount) {
  std::vector<std::size_t> keys =
      std::views::iota(std::size_t{1}, amount + 1) | std::ranges::to<std::vector<std::size_t>>();
  std::ranges::shuffle(keys, eng);

  for (std::size_t k : keys) {
    Rec entry{};
    entry.random_fill(eng, static_cast<std::uint64_t>(k));
    write_record(out, entry);
  }
}

} // namespace

int main(int argc, char **argv) {
  GenConfig cfg = parse_args(argc, argv);
  if (!cfg.ok)
    return 2;

  std::ofstream f;
  std::ostream *os = &std::cout;
  if (cfg.out != "-") {
    f.open(cfg.out, std::ios::binary | std::ios::trunc);
    if (!f) {
      std::cerr << std::format("gen: cannot open '{}' for writing\n", cfg.out);
      return 1;
    }
    os = &f;
  }

  std::mt19937_64 eng(42);
  generate<Record>(eng, *os, cfg.n);

  os->flush();
  if (!*os) {
    std::cerr << std::format("gen: write failed for '{}'\n", cfg.out);
    return 1;
  }
  std::cerr << std::format("gen: {} records, {} bytes/record, {} bytes total -> {}\n", cfg.n,
                           sizeof(Record), cfg.n * sizeof(Record), cfg.out);
  return 0;
}
