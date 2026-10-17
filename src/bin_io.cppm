module;
#include <cstddef>
#include <istream>
#include <ostream>
#include <type_traits>

export module bin_io;

// Пишет одну запись в поток как сырые байты.
// Требует trivially copyable — иначе reinterpret_cast в write() даст UB.
export template <typename Rec>
  requires std::is_trivially_copyable_v<Rec>
void write_record(std::ostream &os, const Rec &r) {
  os.write(reinterpret_cast<const char *>(&r), sizeof(Rec));
}

export template <typename Rec>
  requires std::is_trivially_copyable_v<Rec>
bool read_record(std::istream &is, Rec &r) {
  is.read(reinterpret_cast<char *>(&r), sizeof(Rec));
  return is.gcount() == static_cast<std::streamsize>(sizeof(Rec));
}
