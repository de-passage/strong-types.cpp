#include <gtest/gtest.h>
#include <random>
#include <strong_types.hpp>
#include <type_traits>

namespace st = dpsg::strong_types;

using n = st::number<int, struct n_tag>;

using min = st::number<int, struct min_tag>;
using max = st::number<int, struct max_tag, st::comparable_with<min>>;

int rand(min m, max M) {
  assert(m <= M);
  static std::random_device rd;
  static std::minstd_rand gen(rd());
  std::uniform_int_distribution<> distr(m.value, M.value);
  return distr(gen);
}

TEST(Basic, Comparisons) {
  constexpr n n1{2}, n2{42}, n3{2};

  static_assert(n1 == n1, "should be reflexive");
  static_assert(n1 == n3, "should be equal");
  static_assert(!(n1 == n2), "should not be equal");
  static_assert(n1 != n2, "should be different");
  static_assert(!(n1 != n1), "should not be reflexive");
  static_assert(!(n1 != n3), "should not be equal");
  static_assert(n1 <= n1, "should be less or equal");
  static_assert(n1 <= n2, "should be less or equal");
  static_assert(n1 <= n3, "should be less or equal");
  static_assert(!(n2 <= n3), "should not be less or equal");
  static_assert(n1 < n2, "should be less");
  static_assert(!(n1 < n1), "should not be less");
  static_assert(!(n1 < n3), "should not be less");
  static_assert(n2 > n3, "should be greater");
  static_assert(!(n1 > n2), "should not be greater");
  static_assert(n2 >= n3, "should be greater or equal");
  static_assert(n1 >= n3, "should be greater or equal");
  static_assert(n3 >= n3, "should be greater or equal");
  static_assert(!(n3 >= n2), "should not be greater or equal");

  for (int i = 1; i <= 10000; i *= 10) {
    n o1{rand(min{0}, max{i - 1})};
    n o2{rand(min{o1.value + 1}, max{i})};
    n o3 = o1;
    std::cout << "checking o1(" << o1.value << ") & o2(" << o2.value << ")"
              << std::endl;
    ASSERT_EQ(o1, o3);
    ASSERT_EQ(o1, o1);
    ASSERT_NE(o1, o2);
    ASSERT_LE(o1, o2);
    ASSERT_LE(o1, o3);
    ASSERT_LT(o1, o2);
    ASSERT_FALSE(o1 == o2);
    ASSERT_FALSE(o1 != o1);
    ASSERT_FALSE(o1 != o3);
    ASSERT_LE(o1, o3);
    ASSERT_FALSE(o2 <= o3);
    ASSERT_FALSE(o1 < o1);
    ASSERT_FALSE(o1 < o3);
    ASSERT_GT(o2, o3);
    ASSERT_FALSE(o1 > o2);
    ASSERT_GE(o2, o3);
    ASSERT_GE(o1, o3);
    ASSERT_GE(o3, o3);
    ASSERT_FALSE(o3 >= o2);
  }
}

TEST(Basic, Addition) {
  constexpr n n1{1}, n2{0}, n3{42}, n4{21};
  static_assert(n1 + n2 == n1, "");
  static_assert(n1 + n2 == 1, "");
  static_assert(n3 == n4 + n4, "");
  static_assert((n1 + n3) + n4 == n1 + (n3 + n4), "");
  static_assert(n1 + n3 + n4 == 64, "");
  static_assert(n1 + n3 == n1 + n3, "");
  static_assert(n1 + n3 == n3 + n1, "");
  static_assert(n1 + n3 == 43, "");

  for (int i = 1; i <= 10000; i *= 10) {
    n o1{rand(min{0}, max{i})};
    n o2{rand(min{0}, max{i})};
    std::cout << "checking o1(" << o1.value << ") & o2(" << o2.value << ")"
              << std::endl;
    ASSERT_EQ(n1 + n2, n1);
    ASSERT_EQ(n3, n4 + n4);
    ASSERT_EQ((n1 + n3) + n4, n1 + (n3 + n4));
    ASSERT_EQ(n1 + n3, n1 + n3);
    ASSERT_EQ(n1 + n3, n3 + n1);
  }
}

TEST(Enum, Basic) {
  enum class e { z = 0, a = 1, b = 2, c = 4, d = 8 };
  using test_flag = st::flag<e, e>;

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

  static_assert(std::is_same<test_flag, decltype(f |= e::b)>::value, "");
  auto expected_result = static_cast<e>(static_cast<int>(e::a) | static_cast<int>(e::b));
  ASSERT_EQ((f |= e::b), expected_result);
  ASSERT_EQ(f, expected_result);
}
