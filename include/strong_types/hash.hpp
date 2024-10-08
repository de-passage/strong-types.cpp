#ifndef GUARD_DPSG_STRONG_TYPES_HASH_HPP
#define GUARD_DPSG_STRONG_TYPES_HASH_HPP

#include <cstddef>
#include <type_traits>

#include <strong_types.hpp>

namespace dpsg {
namespace strong_types {

namespace detail {
template <class T>
struct get_first;
template <template <class...> class C, class T, class... Ts>
struct get_first<C<T, Ts...>> {
  using type = T;
};
}  // namespace detail
struct hashable {
  template <class T>
  struct type {
    using hashable = typename detail::get_first<T>::type;
  };
};

namespace detail {
  template<class T, class Enable = void>
    struct is_hashable : std::false_type {};
  template<class T>
    struct is_hashable<T, void_t<typename T::hashable>> : std::true_type {};
}

template<class T>
using is_hashable = detail::is_hashable<T>;
template<class T>
constexpr bool is_hashable_v = is_hashable<T>::value;


#ifdef __cpp_concepts
template <class T>
concept Hashable = requires(T obj) {
  typename T::hashable;
};
#endif
}  // namespace strong_types
}  // namespace dpsg

namespace std {
  template<class T>
    struct hash;
#ifdef __cpp_concepts
template <::dpsg::strong_types::Hashable T>
struct hash<T> {
  std::size_t operator()(const T& value) const {
    return std::hash<typename T::hashable>{}(
        ::dpsg::strong_types::get_value_t{}(value));
  }
};
#define DPSG_STRONG_TYPES_MAKE_HASHABLE(type)
#else
#define DPSG_STRONG_TYPES_MAKE_HASHABLE(type) \
  namespace std {                            \
  template <>                                \
  struct hash<type> {                        \
    std::size_t operator()(const type& value) const { \
      return std::hash<typename type::hashable>{}(value.value); \
    } \
  }; \
  }
#endif
}  // namespace std

#endif  // GUARD_DPSG_STRONG_TYPES_HASH_HPP
