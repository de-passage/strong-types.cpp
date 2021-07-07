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
                     st::commutative_under<st::plus, int> >;

int main(int argc, char** argv) {
  my_strong_value value{argc};
  // trying to compare with int would trigger a compilation error as
  // we haven't explicitly allowed it
  constexpr auto perfect_number_of_arguments = my_strong_value{7};

  if (value != perfect_number_of_arguments) {
    if (value + 1 <= perfect_number_of_arguments) {
      std::cout << "Not enough" << std::endl;
    }
    else if (value > perfect_number_of_arguments) {
      std::cout << "Too many" << std::endl;
    }
  }
  else if (value == perfect_number_of_arguments) {
    std::cout << "Perfect" << std::endl;
  }
  return 0;
}