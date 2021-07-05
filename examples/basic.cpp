#include <iostream>
#include <sstream>
#include <strong_types.hpp>

namespace st = dpsg::strong_types;

namespace custom_modifier {
namespace meta = st::black_magic;
// Allows streaming to/from std::basic_[io]stream
struct streamable {
  template <class T>
  struct type
      : meta::for_each<
            meta::tuple<
                meta::tuple<st::shift_left_t, std::basic_istream<char>>,
                meta::tuple<st::shift_right_t, std::basic_ostream<char>>>,
            meta::apply<st::binary_operation_implementation, T>> {};
};
}  // namespace custom_modifier

namespace newton {
using acceleration =
    st::strong_literal<double, struct acceleration_tag>;  // most basic form
using mass = st::
    strong_literal<unsigned int, struct mass_tag, custom_modifier::streamable>;
using force =
    st::strong_literal<unsigned int,
                       struct force_tag,
                       st::compatible_under<st::divides_t,
                                            mass,
                                            st::construct_t<acceleration>,
                                            st::get_value_then_cast_t<double>,
                                            st::get_value_then_cast_t<double>>,
                       custom_modifier::streamable>;
using speed =
    st::strong_literal<double, struct speed_tag, custom_modifier::streamable>;
using time = st::strong_literal<int,
                                struct time_tag,
                                st::commutative_under<st::multiplies_t,
                                                      acceleration,
                                                      st::construct_t<speed>>,
                                custom_modifier::streamable>;
}  // namespace newton

int main(int argc, char** argv) {
  if (argc < 3) {
    std::cerr << "usage: basic-example <MASS> <FORCE> [SPEED]" << std::endl;
    return 1;
  }

  std::stringstream format;
  format << argv[1] << ' ' << argv[2] << ' ';

  newton::force force;
  newton::mass mass;
  format >> force >> mass;

  if (mass == 0) {
    mass += 1;
  }

  if (argc <= 3) {
    newton::acceleration acceleration = force / mass;
    std::cout << "force(" << force << ") / mass(" << mass << ") = acceleration("
              << acceleration.value << ")" << std::endl;
  }
  else {
    format << argv[3];
    newton::time time;
    format >> time;

    newton::speed speed = force / mass * time;

    std::cout << "force(" << force << ") / mass(" << mass << ") * time(" << time
              << ") = speed(" << speed << ")" << std::endl;
  }

  return 0;
}