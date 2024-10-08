#include <iostream>
#include <strong_types.hpp>
#include <span>
#include "strong_types/iostream.hpp"

namespace st = dpsg::strong_types;

enum class flag_values { z = 0, a = 1, b = 2, c = 4, d = 8, e = 16, f = 32 };

struct flag : st::flag_derivation<
                  flag,
                  flag_values,
                  st::streamable_as<std::underlying_type_t<flag_values>>> {
  using enum flag_values;

  flag_values value;

  constexpr flag(flag_values f) : value{f} {}
  constexpr flag() : value{flag_values::z} {}
};

int main(int argc, const char** argv) {
  std::span<const char*> args(argv + 1, argc - 1);
  flag f{};
  for (std::string_view arg : args) {
    if (arg == "a") {
      std::cerr << "adding a\n";
      f |= flag::a;
    } else if (arg == "b") {
      std::cerr << "adding b\n";
      f |= flag::b;
    } else if (arg == "c") {
      std::cerr << "adding c\n";
      f |= flag::c;
    } else if (arg == "d") {
      std::cerr << "adding d\n";
      f |= flag::d;
    } else if (arg == "e") {
      std::cerr << "adding e\n";
      f |= flag::e;
    } else if (arg == "f") {
      std::cerr << "adding f\n";
      f |= flag::f;
    } else if (arg == "!a") {
      std::cerr << "removing a\n";
      f &= ~flag{flag::a};
    } else if (arg == "!b") {
      std::cerr << "removing b\n";
      f &= ~flag{flag::b};
    } else if (arg == "!c") {
      std::cerr << "removing c\n";
      f &= ~flag{flag::c};
    } else if (arg == "!d") {
      std::cerr << "removing d\n";
      f &= ~flag{flag::d};
    } else if (arg == "!e") {
      std::cerr << "removing e\n";
      f &= ~flag{flag::e};
    } else if (arg == "!f") {
      std::cerr << "removing f\n";
      f &= ~flag{flag::f};
    } else {
      std::cerr << "unknown flag: '" << arg << "'\n";
    }
  }
  std::cout << f << '\n';
}
