This is the result of my life-long (OK, maybe just-a-couple-years-long) quest for the ultimate strong type engine for C++. Other solutions do exist but I haven't found anything that satisfies the level of granularity that I want for my strong type declarations.

# Getting Started

## Installation

At this point, all of the following steps require you to clone/download the repository.

### Vcpkg

A port is available for vcpkg in the *support/vcpkg* directory. I'll publish it someday, maybe. For now, you can copy the *strong_types* directory in your overlay directory and everything should work.

In your vcpkg.json:
``` json
{
    "dependencies": [
        "strong-types"
    ]
}
```

### Using CMake

The following should install the library in your system directories.

``` bash
cmake -B build -DBUILD_TESTING=OFF
cmake --install build
```

To use the library, add the following to your CMakeLists.txt:
``` cmake
find_package(strong_types REQUIRED)
target_link_libraries(<your target> PRIVATE strong_types::strong_types)
```

### Manual installation

The library is header only, simply copy the files in the include directory somewhere where your compiler can find it and you're good to go.

### Building the tests and examples

To build the tests, keep BUILD_TESTING to ON when configuring CMake.

To build the examples, use the target *examples*:
``` bash
cmake --build build --target examples
```

## Example

This example is a bit long but showcases basically all the functionalities of the library, take the time to read it through.
``` cpp
#include <iostream>
#include <sstream>
#include <strong_types.hpp>

namespace st = dpsg::strong_types;

// Allows streaming to/from std::basic_[io]stream
// There is a predefined version of this in the strong_types/iostream.hpp file
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
                        custom_modifier::streamable>;  // additionnal modifier
using force =
    st::number<unsigned int,
               struct force_tag,
               // compatible under division with mass to produce acceleration
               st::compatible_under</* operation */
                                    st::divides,
                                    /* operand   */
                                    mass,
                                    /* how to build the result type */
                                    st::construct_t<acceleration>,
                                    /* how to unpack "force" */
                                    st::get_value_then_cast_t<double>,
                                    /* how to unpack "mass" */
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
    // Try turning on -Wconversion and replacing '1' with '-1', you'll get a
    // proper warning telling you that your int is implicitely converted to
    // unsigned int
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
    newton::speed speed2 = time * (force / mass);  // both forms work
    speed2 += speed;  // just in case it wasn't clear already, strong numbers
                      // are compatible with other numbers with the same tag

    // however, this will fail because it is parsed as (t*f)/m
    // newton::speed speed = time * force / mass;

    std::cout << "force(" << force << ") / mass(" << mass << ") * time(" << time
              << ") = speed(" << speed << ")" << std::endl;
  }

  return 0;
}
```

# Rationale

What are strong types you say? Well, let's start from an example you've probably seen before:
``` cpp
double compute_speed(double force, double mass, double time);

double acceleration = compute_speed(time, force, mass);
```
Now if you don't see what's wrong with the code above, this repo isn't for you. If, however, you enjoy robust code that can be modified and reused with confidence, the solution you're yearning for is *strong types*, the ability to produce a type that shares some or all of the characteristics of another, without being freely substitutable with the latter.
In other words, you'd like the following code to fail to compile:
``` cpp
typedef int barable;

void foo(int);
void bar(barable);

int i;
barable b;

foo(b); // fail here please
bar(i); // ugh...
```

Some day, we might get a nice standard solution to do that, but for now we have to resort to manually implement our strong types ourselves. There are several potential solutions to that problem:
1. declare that this isn't a real problem, stick with `int` everywhere and never have a civil conversation with me again
2. try to implement manually wrapper types everytime you need one before giving up and going back to option 1
3. write a template type that wraps any numeric type, some other common types and call it a day. That's not a bad solution, and there's a few options doing just that available online. It just doesn't quite satisfy my strong type OCD. I want to be able to define more precisely what different types can and cannot do.
4. implement some sort of code generator that you feed with some sort of meta-language in return for your C++ boilerplate. Again something I wouldn't consider unless your project is big enough with all the tooling necessary to support an extra step in your development process.
5. ditch C++ for some fancy functional language with great support for strong types. Boring.
6. sacrifice your soul to the Great Old C++ Ones and acquire the forbidden knowledge of template metaprogramming, then proceed to summon with dark magic a meta-beast that will your bidding from within the code itself. This is the solution I chose here. It's always the solution I choose...

# Usage

At its core this library is simply a way to quickly define an operator overload, with a few convenience classes to help you get started. Let's take a look at the `dspg::strong_types::strong_value` struct. Its usage is very similar to `dpsg::strong_types::number` that we saw in the previous example.

```cpp
using my_strong_value =
                      // 'int' is the underlying type
    st::strong_value< int,
                      // we use a unique tag to differenciate our type from other
                      // strong values. The type doesn't need to be actually defined
                      struct some_tag,
                      // After the tag, we can add modifiers. strong_value doesn't
                      // embed anything by default, so with these declarations we'll only
                      // be able to compare it with itself and to add 'int's to it.
                      st::comparable,
                      st::commutative_under<st::plus, int>
                      >
```
The definition is quite simple (edited for clarity):
```cpp
template <class Type, class Tag, class... Params>
struct strong_value :
      derive_t< strong_value<Type, Tag, Params...>,
                Params...
              >
{
  using value_type = Type;

  constexpr strong_value() noexcept : value{} {}

  constexpr explicit number(const value_type& v) noexcept : value{v} {}

  value_type value;
};
```
As you can see it's a very simple wrapper around a `value` member with an explicit constructor to prevent silent conversions. The magic happens in the inheritance definition `derive_t<strong_value<Type, Tag, Params...>, Params...>`: we call the `derive_t` meta function with the type that we're defining and the list of modifiers that we want to use to generate operator overloads.
`number` is defined in a similar fashion:
```cpp
template <class Type, class Tag, class... Params>
struct number :
            derive_t< number<Type, Tag, Params...>,
                      arithmetic,
                      comparable,
                      arithmetically_compatible_with<Type>,
                      comparable_with<Type>,
                      Params...
                    >
{
  using value_type = Type;

  constexpr number() noexcept = default;

  constexpr explicit number(const value_type& v) noexcept : value{v} {}

  value_type value;
};
```
The only difference with `strong_value` is that we give it a set of modifiers to automatically implement a number of overloads corresponding to arithmetic and comparison operations between two `number`s of the same tag or a `number` and a variable implicitely compatible into the type of its underlying value.

`derive_t` itself is extremely simple, it takes a type, and a list of types with a nested template named `type` (such as `streamable` from the first example), and applies the first type to each nested template:
```cpp
template <class T, class... Ts>
struct derive_t : Ts::template type<T>... {};
```

## Custom types
As you may have gathered this library is basically just `derive_t` and a bunch of predefined modifiers defining functions and operator overloads (plus some TMP utilities that we'll detail later). With this in our toolbelt, we already have a lot of possibilities. If you tried to play with the first example, you may have noticed that there is no good way to encode `acceleration * mass = force`, since C++ doesn't allow us to predeclare `using` declarations. `force` and `mass` do not exist at the point where `acceleration` is defined so we couldn't use the modifier `commutative_under<multiplies, mass, construct_t<force>>` to implement the appropriate `operator*` overloads.
Using our new knowledge of `derive_t`, we can now implement the complete relationship:
```cpp
namespace st = dpsg::strong_types;

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
                                          // We do not need to cast to double anymore,
                                          // the default behavior is to get the "value" member,
                                          // so we don't need to add anything
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

```

## Flags

On top of `strong_value` and `number`, the library provides a utility class `flag` to manipulate enums like bitwise flags.
```cpp

```

# Extensions

Some extensions are provided for common interactions with the standard library. These are in their own header not to drag the whole standard library with the core strong type definitions.

## Streamable

The `streamable` modifier allows you to stream your strong types to and from `std::basic_[io]stream`.
```cpp
#include <strong_types/iostream.hpp>

namespace st = dpsg::strong_types;

using streamable_number = st::number<int, struct streamable_tag, st::streamable>;
// Also dpsg::strong_types::wstreamable for wide streams,
// and dpsg::strong_types::basic_streamable<Char, CharTraits> for custom streams

int main() {
  streamable_number n{42};
  std::cout << n << std::endl;
  std::cin >> n;
  std::cout << n << std::endl;
}
```

## Hashable

The `hashable` modifier allows you to use your strong types as keys in `std::unordered_map` and `std::unordered_set` (remember that these classes also require your type to be comparable).

Due to technical limitations in C++14, you need to call a macro to make your type hashable. From C++20 onwards, this is not necessary anymore. The macro expands to nothing in this case, and can be kept for backward compatibility.
```cpp
#include <strong_types/hash.hpp>

namespace st = dpsg::strong_types;

using hashable_number = st::number<int, struct hashable_tag, st::hashable, st::comparable>;

// Only required pre-C++20
DPSG_STRONG_TYPES_MAKE_HASHABLE(hashable_number);

int main() {
  hashable_number n{42};
  std::unordered_map<hashable_number, int> m;
  m[n] = 42;
}
```
