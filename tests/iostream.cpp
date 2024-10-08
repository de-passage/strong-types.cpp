#include <gtest/gtest.h>

#include <strong_types/iostream.hpp>

#include <sstream>

namespace st = dpsg::strong_types;

TEST(Iostream, Basic) {
  using streamable = st::number<int, struct streamable_tag, st::streamable>;

  std::stringstream ss;
  streamable a{42};
  ss << a;
  streamable b{0};
  ss >> b;
  ASSERT_EQ(a, b);
  ASSERT_EQ(b, 42);
}
