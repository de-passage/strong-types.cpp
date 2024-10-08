#ifndef GUARD_DPSG_STRONG_TYPES_FLAGS_HPP
#define GUARD_DPSG_STRONG_TYPES_FLAGS_HPP

#include <strong_types.hpp>

namespace dpsg {
namespace strong_types {

using self_assigning_bitwise_operators = black_magic::tuple<binary_and_assign,
                                                            binary_or_assign,
                                                            binary_xor_assign,
                                                            shift_left_assign,
                                                            shift_right_assign>;
using non_assigning_bitwise_operators = black_magic::
    tuple<binary_and, binary_or, binary_xor, shift_left, shift_right>;

/// Ignore the result of the operation and return the left argument to a binary
/// operator
struct return_left_argument {
  template <class Result, class L, class R>
  constexpr decltype(auto) operator()(Result&&, L&& l, R&&) const noexcept {
    return std::forward<L>(l);
  }
};

template <class Type>
struct access_value_as_ref {
  template <class T>
  constexpr auto& operator()(T& t) const noexcept {
    return *reinterpret_cast<Type*>(&t.value);
  }
};

template <class Enum, class UnderlyingType = std::underlying_type_t<Enum>>
struct bitwise_compatible_with_enum {
  template <class Type>
  struct type
      : black_magic::for_each<
            non_assigning_bitwise_operators,
            make_commutative_operator<Type,
                                      Enum,
                                      cast_to_then_construct_t<Enum, Type>,
                                      get_value_then_cast_t<UnderlyingType>,
                                      get_value_then_cast_t<UnderlyingType>>>,
        black_magic::for_each<
            self_assigning_bitwise_operators,
            make_binary_operator<Type,
                                 Enum,
                                 return_left_argument,
                                 access_value_as_ref<UnderlyingType>,
                                 get_value_then_cast_t<UnderlyingType>>> {};
};

template <class Enum, class UnderlyingType = std::underlying_type_t<Enum>>
struct bitwise_enum {
  template <class Type>
  struct type
      : black_magic::for_each<
            non_assigning_bitwise_operators,
            make_symmetric_operator<Type,
                                    cast_to_then_construct_t<Enum, Type>,
                                    get_value_then_cast_t<UnderlyingType>>>,
        black_magic::for_each<
            self_assigning_bitwise_operators,
            make_binary_operator<Type,
                                 Type,
                                 return_left_argument,
                                 access_value_as_ref<UnderlyingType>,
                                 get_value_then_cast_t<UnderlyingType>>>,
        black_magic::for_each<
            unary_bitwise_operators,
            make_unary_operator<Type,
                                cast_to_then_construct_t<Enum, Type>,
                                get_value_then_cast_t<UnderlyingType>>> {};
};

template <class Self, class Type, class... Args>
using flag_derivation = derive_t<Self,
                                 comparable,
                                 comparable_with<Type>,
                                 bitwise_enum<Type>,
                                 bitwise_compatible_with_enum<Type>,
                                 Args...>;

template <class Type, class Tag, class... Args>
struct flag : flag_derivation<flag<Type, Tag, Args...>, Type, Args...> {
 public:
  static_assert(std::is_enum<Type>::value,
                "Underlying type for flag must be an enum");
  using underlying_type = std::underlying_type_t<Type>;
  using value_type = Type;

  value_type value{static_cast<Type>(0)};

  template <
      class U,
      std::enable_if_t<std::is_convertible<std::decay_t<U>, value_type>::value,
                       int> = 0>
  constexpr explicit flag(U t) noexcept : value{t} {}

  constexpr flag() noexcept = default;
};

}  // namespace strong_types
}  // namespace dpsg

#endif  // GUARD_DPSG_STRONG_TYPES_FLAGS_HPP
