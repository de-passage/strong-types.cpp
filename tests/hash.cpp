#include <gtest/gtest.h>

#include <strong_types/hash.hpp>

#include <type_traits>
#include <unordered_set>

#ifndef __cpp_concepts
using id = dpsg::strong_types::strong_value<int,
                                            struct id_tag,
                                            dpsg::strong_types::hashable,
                                            dpsg::strong_types::comparable>;
DPSG_STRONG_TYPES_MAKE_HASHABLE(id);
#endif

TEST(Hash, Basic) {
#ifdef __cpp_concepts
  using id = dpsg::strong_types::strong_value<int,
                                              struct id_tag,
                                              dpsg::strong_types::hashable,
                                              dpsg::strong_types::comparable>;
#endif
  std::unordered_set<id> set;

  set.insert(id{42});
  ASSERT_EQ(set.size(), 1);
  ASSERT_EQ(set.count(id{42}), 1);
}
