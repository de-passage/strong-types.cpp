#include <iostream>
#include <sstream>
#include <strong_types.hpp>

namespace st = dpsg::strong_types;

namespace newton {
using acceleration =
    st::strong_literal<double, struct acceleration_tag>;  // most basic form
using mass = st::strong_literal<unsigned int, struct mass_tag>;
using force =
    st::strong_literal<unsigned int,
                       struct force_tag,
                       st::compatible_under<st::divides_t,
                                            mass,
                                            st::construct_t<acceleration>,
                                            st::get_value_then_cast_t<double>,
                                            st::get_value_then_cast_t<double>>>;
using speed = st::strong_literal<double, struct speed_tag>;
using time = st::strong_literal<int,
                                struct time_tag,
                                st::commutative_under<st::multiplies_t,
                                                      acceleration,
                                                      st::construct_t<speed>>>;
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
  format >> force.value >> mass.value;

  if (mass == 0) {
    mass += 1;
  }

  if (argc <= 3) {
    newton::acceleration acceleration = force / mass;
    std::cout << "force(" << force.value << ") / mass(" << mass.value
              << ") = acceleration(" << acceleration.value << ")" << std::endl;
  }
  else {
    format << argv[3];
    newton::time time;
    format >> time.value;

    newton::speed speed = force / mass * time;

    std::cout << "force(" << force.value << ") / mass(" << mass.value
              << ") * time(" << time.value << ") = speed(" << speed.value << ")"
              << std::endl;
  }

  return 0;
}