#include <gtest/gtest.h>

#include <strong_types.hpp>
#include <strong_types/flags.hpp>

namespace st = dpsg::strong_types;

TEST(Enum, Basic) {
  enum class e { z = 0, a = 1, b = 2, c = 4, d = 8 };
  using test_flag = dpsg::strong_types::flag<e, e>;

  constexpr test_flag can_be_constructed{e::a};

  static_assert(test_flag{e::a} == can_be_constructed, "");
  constexpr auto result = test_flag{e::a} & test_flag{e::a};
  static_assert(result == can_be_constructed, "");
  static_assert((test_flag{e::a} | test_flag{e::a}) == test_flag{e::a}, "");
  static_assert((test_flag{e::a} ^ test_flag{e::a}) == test_flag{e::z}, "");

  enum class e2 : uint8_t { z = 0, a = 1, b = 2, c = 4, d = 8 };
  using test_flag2 = st::flag<e2, e2>;
  static_assert(test_flag2{e2::a} == e2::a, "");
  constexpr auto result2 = test_flag2{e2::a} & test_flag2{e2::a};
  static_assert(result2 == e2::a, "");
  static_assert((test_flag2{e2::a} | test_flag2{e2::a}) == test_flag2{e2::a},
                "");
  static_assert((test_flag2{e2::a} ^ test_flag2{e2::a}) == test_flag2{e2::z},
                "");

  static_assert(((test_flag{} | e::a) & e::b) == e::z, "");
  static_assert(
      (test_flag{} | e::a | e::b) ==
          static_cast<e>(static_cast<int>(e::a) | static_cast<int>(e::b)),
      "");

  test_flag f{e::a};

  static_assert(std::is_same<test_flag&, decltype(f |= e::b)>::value, "");
  auto expected_result = static_cast<e>(static_cast<int>(e::a) | static_cast<int>(e::b));
  ASSERT_EQ((f |= e::b), expected_result);
  ASSERT_EQ(f, expected_result);
}
