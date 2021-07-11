#include <iostream>
#include <strong_types.hpp>

namespace st = dpsg::strong_types;

using my_strong_value =
    // 'int' is the underlying type
    st::strong_value<int,
                     // we use a unique tag to differenciate our type from other
                     // strong values. The type doesn't need to be defined, it's
                     // what's known as a phantom type. You can also use an
                     // actual type to imply some sort of relationship. For
                     // example a numeric identifier for some class could be
                     // parameterized with the type it's refering to to enable
                     // some more meta-magic down the line
                     struct some_tag,
                     // After the tag, we can add modifiers. strong_value
                     // doesn't embed anything by default, so with these
                     // declarations we'll only be able to compare it with
                     // itself and to add 'int's to it.
                     st::comparable,
                     // by default, when implementing a new operation through
                     // strong_type::derive (see below), we'll try to construct
                     // the current type as a result. Thus in this case
                     // 'int + my_strong_value' will have type 'my_strong_value'
                     // We could specify a different result type by giving a
                     // construction function such as 'construct_t' as a third
                     // template parameter
                     st::commutative_under<st::plus, int>,
                     st::comparable_with<int>>;

template <class T, class U, class... Ts>
using derive_number = st::derive_t<T,
                                   st::arithmetic,
                                   st::comparable,
                                   st::arithmetically_compatible_with<U>,
                                   st::comparable_with<U>,
                                   Ts...>;

struct mass : derive_number<mass, double> {
  constexpr mass() noexcept = default;
  constexpr explicit mass(double m) noexcept : value{m} {}
  double value{};
};
struct acceleration
    : derive_number<acceleration,
                    double,
                    st::commutative_under<st::multiplies,
                                          mass,
                                          st::construct_t<struct force>>> {
  constexpr acceleration() noexcept = default;
  constexpr explicit acceleration(double a) noexcept : value{a} {}
  double value{};
};
struct force
    : derive_number<force,
                    double,
                    st::compatible_under<st::divides,
                                         mass,
                                         st::construct_t<acceleration>>> {
  constexpr force() noexcept = default;
  constexpr explicit force(double f) noexcept : value{f} {}
  double value{};
};

int main(int argc, char**) {
  my_strong_value value{argc};
  // trying to compare with int would trigger a compilation error as
  // we haven't explicitly allowed it
  constexpr auto perfect_number_of_arguments = my_strong_value{7};

  if (value != perfect_number_of_arguments) {
    if ((value + 1) <= perfect_number_of_arguments) {
      std::cout << "Not enough" << std::endl;
    }
    else if (value > perfect_number_of_arguments) {
      std::cout << "Too many" << std::endl;
    }
  }
  else if (value == perfect_number_of_arguments) {
    std::cout << "Perfect" << std::endl;
  }
  acceleration a{static_cast<double>(argc)};
  mass m{3.78};
  force f1 = m * a;
  a = f1 / m;
  force f2 = a * m;
  std::cout << "f1(" << f1.value << ") = f2(" << f2.value << ")" << std::endl;
  return 0;
}