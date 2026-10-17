module;
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <format>
#include <random>
#include <string_view>
#include <type_traits>

export module record_car;

export struct CarOwner {
  std::uint64_t plate; // ключ
  char brand[32];
  char owner[64];
  bool stolen;
  bool deleted;
  char newline;

  std::uint64_t key() const noexcept { return plate; }

  static std::size_t hash_key(std::uint64_t k) noexcept {
    return static_cast<std::size_t>(k * 2654435761ull);
  }

  void set_brand(std::string_view s) noexcept {
    std::snprintf(brand, sizeof(brand), "%.*s", static_cast<int>(s.size()), s.data());
  }
  void set_owner(std::string_view s) noexcept {
    std::snprintf(owner, sizeof(owner), "%.*s", static_cast<int>(s.size()), s.data());
  }
  std::string_view brand_sv() const noexcept { return brand; }
  std::string_view owner_sv() const noexcept { return owner; }

  // Генерация случайного содержимого. Ответственность типа, а не gen.
  template <std::uniform_random_bit_generator Engine>
  void random_fill(Engine &eng, std::uint64_t k) {
    static constexpr char alphabet[] = "0123456789"
                                       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                       "abcdefghijklmnopqrstuvwxyz";
    static constexpr std::size_t ALPHA_SIZE = sizeof(alphabet) - 1;

    std::uniform_int_distribution<std::size_t> dist(0, ALPHA_SIZE - 1);
    std::bernoulli_distribution coin(0.5);

    plate = k;
    stolen = coin(eng);
    deleted = false;
    newline = '\n';

    for (std::size_t j = 0; j + 1 < sizeof(brand); ++j)
      brand[j] = alphabet[dist(eng)];
    brand[sizeof(brand) - 1] = '\0';

    for (std::size_t j = 0; j + 1 < sizeof(owner); ++j)
      owner[j] = alphabet[dist(eng)];
    owner[sizeof(owner) - 1] = '\0';
  }

  static CarOwner make_demo(std::uint64_t k, std::size_t i) {
    CarOwner r{};
    r.plate = k;
    r.stolen = (i % 3 == 0);
    r.deleted = false;
    r.newline = '\n';
    r.set_brand("Brand" + std::to_string(i % 5));
    r.set_owner("Owner" + std::to_string(i));
    return r;
  }
};

export template <> struct std::formatter<CarOwner> {
  char mode = '\0';

  constexpr auto parse(std::format_parse_context &ctx) {
    auto it = ctx.begin();
    if (it != ctx.end() && *it != '}')
      mode = *it++;
    return it;
  }

  auto format(const CarOwner &r, std::format_context &ctx) const {
    switch (mode) {
    case 'k':
      return std::format_to(ctx.out(), "{}", r.plate);
    case 's':
      return std::format_to(ctx.out(), "{}:{}", r.plate, r.brand_sv());
    case 't':
      return std::format_to(ctx.out(), "| {} | {} | {} | {} |", r.plate, r.brand_sv(), r.owner_sv(),
                            r.stolen ? "yes" : "no");
    case '\0':
    default:
      return std::format_to(ctx.out(), "plate={} brand={} owner={} stolen={} deleted={}", r.plate,
                            r.brand_sv(), r.owner_sv(), r.stolen ? "yes" : "no",
                            r.deleted ? "yes" : "no");
    }
  }
};

static_assert(sizeof(CarOwner) == 112, "CarOwner layout changed");
static_assert(offsetof(CarOwner, newline) + 1 <= sizeof(CarOwner),
              "newline must be the last byte (no trailing padding after it)");
static_assert(std::is_trivially_copyable_v<CarOwner>);
