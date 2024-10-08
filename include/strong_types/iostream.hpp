#ifndef GUARD_DPSG_STRONG_TYPES_IOSTREAM_HPP
#define GUARD_DPSG_STRONG_TYPES_IOSTREAM_HPP

#include <iostream>
#include <strong_types.hpp>

namespace dpsg {
namespace strong_types {

template <class Char, class CharTraits = std::char_traits<Char>>
struct basic_streamable {
  template <class T>
  struct type
      : black_magic::for_each<
            black_magic::tuple<
                black_magic::tuple<shift_left,
                                   std::basic_istream<Char, CharTraits>>,
                black_magic::tuple<shift_right,
                                   std::basic_ostream<Char, CharTraits>>>,
            black_magic::apply<implement_binary_operation, T>> {};
};

struct streamable : basic_streamable<char> {};
struct wstreamable : basic_streamable<wchar_t> {};

template <class Transform,
          class Char,
          class CharTraits = std::char_traits<Char>>
struct basic_streamable_transform {
  template <class T>
  struct type
      : black_magic::for_each<
            black_magic::tuple<
                black_magic::tuple<shift_left,
                                   std::basic_istream<Char, CharTraits>>,
                black_magic::tuple<shift_right,
                                   std::basic_ostream<Char, CharTraits>>>,
            black_magic::apply<implement_binary_operation,
                               T,
                               passthrough_t,
                               passthrough_t,
                               Transform>> {};
};

template <class Transform>
struct streamable_transform : basic_streamable_transform<Transform, char> {};
template <class Transform>
struct wstreamable_transform : basic_streamable_transform<Transform, wchar_t> {
};

template <class As, class Char, class CharTraits = std::char_traits<Char>>
struct basic_streamable_as
    : basic_streamable_transform<get_value_then_cast_t<As>, Char, CharTraits> {
};

template <class As>
struct streamable_as : basic_streamable_as<As, char> {};
template <class As>
struct wstreamable_as : basic_streamable_as<As, wchar_t> {};

}  // namespace strong_types
}  // namespace dpsg
#endif  // GUARD_DPSG_STRONG_TYPES_IOSTREAM_HPP
