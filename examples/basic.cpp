#include <iostream>
#include <sstream>
#include <strong_types.hpp>

namespace st = dpsg::strong_types;

// Allows streaming to/from std::basic_[io]stream
namespace custom_modifier {
namespace meta = st::black_magic;
struct streamable {
  template <class T>
  struct type
      : meta::for_each<
            meta::tuple<meta::tuple<st::shift_left, std::basic_istream<char>>,
                        meta::tuple<st::shift_right, std::basic_ostream<char>>>,
            meta::apply<st::implement_binary_operation, T>> {};
};
}  // namespace custom_modifier

namespace newton {
using acceleration =
    st::number<double, struct acceleration_tag>;  // most basic form
using mass = st::number<unsigned int,
                        struct mass_tag,
                        custom_modifier::streamable>;  // additionnal modifier.
using force =
    st::number<unsigned int,
               struct force_tag,
               // compatible under division with mass to produce acceleration
               st::compatible_under<st::divides,
                                    mass,
                                    st::construct_t<acceleration>,
                                    st::get_value_then_cast_t<double>,
                                    st::get_value_then_cast_t<double>>,
               custom_modifier::streamable>;
using speed = st::number<double, struct speed_tag, custom_modifier::streamable>;
using time = st::number<
    int,
    struct time_tag,
    // compatible with acceleration under multiplication to produce speed.
    // Additionally multiplication is commutative, i.e. t*a=a*t
    st::commutative_under<st::multiplies, acceleration, st::construct_t<speed>>,
    custom_modifier::streamable>;
}  // namespace newton

int main(int argc, char** argv) {
  if (argc < 3) {
    std::cerr << "usage: basic-example <MASS> <FORCE> [<SPEED>]" << std::endl;
    return 1;
  }

  std::stringstream format;
  format << argv[1] << ' ' << argv[2] << ' ';

  newton::force force;
  newton::mass mass;
  format >> force >>
      mass;  // thanks to our custom modifier, our types are streamable

  // we can of course compare and do arithmetics with integers.
  if (mass == 0) {
    mass += 1;
  }

  if (argc <= 3) {
    newton::acceleration acceleration = force / mass;  // f/m=a, as expected

    // this would cause a compilation failure: invalid operands to binary
    // operation auto wut = mass / force;

    std::cout << "force(" << force << ") / mass(" << mass
              << ") = acceleration("
              // the underlying value is always available this way
              << acceleration.value << ")" << std::endl;
  }
  else {
    format << argv[3];
    newton::time time;
    format >> time;

    newton::speed speed = force / mass * time;
    [[maybe_unused]] newton::speed speed2 =
        time * (force / mass);  // both forms work

    // however, this will fail because it is parsed as (t*f)/m
    // newton::speed speed = time * force / mass;

    std::cout << "force(" << force << ") / mass(" << mass << ") * time(" << time
              << ") = speed(" << speed << ")" << std::endl;
  }

  return 0;
}