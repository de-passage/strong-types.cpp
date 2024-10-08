#ifndef GUARD_DPSG_STRONG_TYPES_HPP
#define GUARD_DPSG_STRONG_TYPES_HPP

#include <type_traits>
#include <utility>

namespace dpsg {
namespace strong_types {

namespace detail {
template <class...>
struct void_t_impl {
  using type = void;
};
template <class... Ts>
using void_t = typename void_t_impl<Ts...>::type;
template <class T, class = void>
struct has_value : std::false_type {};
template <class T>
struct has_value<T, void_t<decltype(std::declval<T>().value)>>
    : std::true_type {};

template <class Self>
struct implement_ignored_values {
  template <class T, class... Args>
  inline constexpr decltype(auto) operator()(T&& t, Args&&... /* ignored */)
      const noexcept {
    return static_cast<const Self*>(this)->operator()(std::forward<T>(t));
  }
};
}  // namespace detail
template <class T>
using has_value = detail::has_value<T>;
template <class T>
constexpr bool has_value_v = has_value<T>::value;

struct get_value_t : detail::implement_ignored_values<get_value_t> {
  using detail::implement_ignored_values<get_value_t>::operator();
  template <class T, std::enable_if_t<has_value_v<T>, int> = 0>
  inline constexpr decltype(auto) operator()(T&& t) noexcept {
    return std::forward<T>(t).value;
  }
  template <class T, std::enable_if_t<!has_value_v<T>, int> = 0>
  inline constexpr decltype(auto) operator()(T&& t) noexcept {
    return std::forward<T>(t);
  }
  template <class T, std::enable_if_t<has_value_v<T>, int> = 0>
  inline constexpr auto& operator()(T& t) noexcept {
    return t.value;
  }
  template <class T, std::enable_if_t<!has_value_v<T>, int> = 0>
  inline constexpr auto& operator()(T& t) noexcept {
    return t;
  }
};

template <class To>
struct get_value_then_cast_t
    : detail::implement_ignored_values<get_value_then_cast_t<To>> {
  using detail::implement_ignored_values<get_value_then_cast_t>::operator();
  template <class T>
  inline constexpr To operator()(T&& t) noexcept {
    return static_cast<To>(get_value_t{}(std::forward<T>(t)));
  }
};

template <class To, class Cl>
struct cast_to_then_construct_t {
  template <class T, class... Ignored>
  inline constexpr Cl operator()(T&& t, Ignored&&... /* ignored */) noexcept {
    return Cl{static_cast<To>(std::forward<T>(t))};
  }
};

struct passthrough_t : detail::implement_ignored_values<passthrough_t> {
  using detail::implement_ignored_values<passthrough_t>::operator();
  template <class T>
  inline constexpr decltype(auto) operator()(T&& t) const noexcept {
    return std::forward<T>(t);
  }
};

template <class Cl>
struct construct_t : detail::implement_ignored_values<construct_t<Cl>> {
  using detail::implement_ignored_values<construct_t>::operator();
  template <class T>
  inline constexpr Cl operator()(T&& ts) const noexcept {
    return Cl{std::forward<T>(ts)};
  }
};

namespace black_magic {
template <class... Ts>
struct tuple;

template <class T1, class T2>
struct concat_tuples;
template <class... T1s, class... T2s>
struct concat_tuples<tuple<T1s...>, tuple<T2s...>> {
  using type = tuple<T1s..., T2s...>;
};
template <class T1, class T2>
using concat_tuples_t = typename concat_tuples<T1, T2>::type;

template <class T, class U>
struct for_each;
template <class U, class... Ts>
struct for_each<tuple<Ts...>, U> : U::template type<Ts>... {};

template <template <class...> class S, class... Ts>
struct apply {
  template <class Tuple>
  struct type;
  template <template <class...> class Tuple, class Op, class Target>
  struct type<Tuple<Op, Target>> : S<Op, Target, Ts...> {};
};

struct deduce;
namespace detail {
template <class T, class D, class R, class = void>
struct deduce_return_type_impl {
  using type = T;
};
template <class D, class R>
struct deduce_return_type_impl<deduce, D, R> {
  using type = D;
};
template <class D, class R>
struct deduce_return_type_impl<R, D, R> {
  using type = R;
};

template <template <class...> class T, class D, class R, class... Ts>
struct deduce_return_type_impl<
    T<Ts...>,
    D,
    R,
    std::enable_if_t<!std::is_same<T<Ts...>, R>::value>> {
  using type = T<typename deduce_return_type_impl<Ts, R, R>::type...>;
};

}  // namespace detail

/** \brief Deduce the return type of the implemented operators based on the
 * given arguments.
 *
 */
template <class T, class D, class R>
using deduce_return_type =
    typename detail::deduce_return_type_impl<T, D, R>::type;
}  // namespace black_magic

// clang-tidy off
#define DPSG_DEFINE_BINARY_OPERATOR(name, sym)                             \
  struct name {                                                            \
    template <class T, class U>                                            \
    constexpr inline decltype(auto) operator()(T&& left,                   \
                                               U&& right) const noexcept { \
      return std::forward<T>(left) sym std::forward<U>(right);             \
    }                                                                      \
  };

#define DPSG_DEFINE_UNARY_OPERATOR(name, sym)                          \
  struct name {                                                        \
    template <class U>                                                 \
    constexpr inline decltype(auto) operator()(U&& u) const noexcept { \
      return sym std::forward<U>(u);                                   \
    }                                                                  \
  };

#define DPSG_APPLY_TO_BINARY_OPERATORS(f)                            \
  f(plus, +) f(minus, -) f(divides, /) f(multiplies, *) f(modulo, %) \
      f(equal, ==) f(not_equal, !=) f(lesser, <) f(greater, >)       \
          f(lesser_equal, <=) f(greater_equal, >=) f(binary_or, |)   \
              f(binary_and, &) f(binary_xor, ^) f(shift_right, <<)   \
                  f(shift_left, >>) f(boolean_or, ||) f(boolean_and, &&)

#define DPSG_APPLY_TO_SELF_ASSIGNING_BINARY_OPERATORS(f)                       \
  f(plus_assign, +=) f(minus_assign, -=) f(divides_assign, /=)                 \
      f(multiplies_assign, *=) f(modulo_assign, %=) f(shift_right_assign, <<=) \
          f(shift_left_assign, >>=) f(binary_and_assign, &=)                   \
              f(binary_or_assign, |=) f(binary_xor_assign, ^=)

#define DPSG_APPLY_TO_UNARY_OPERATORS(f)                           \
  f(boolean_not, !) f(binary_not, ~) f(negate, -) f(positivate, +) \
      f(dereference, *) f(address_of, &) f(increment, ++) f(decrement, --)

DPSG_APPLY_TO_BINARY_OPERATORS(DPSG_DEFINE_BINARY_OPERATOR)
DPSG_APPLY_TO_SELF_ASSIGNING_BINARY_OPERATORS(DPSG_DEFINE_BINARY_OPERATOR)
DPSG_APPLY_TO_UNARY_OPERATORS(DPSG_DEFINE_UNARY_OPERATOR)

#undef DPSG_DEFINE_UNARY_OPERATOR
#undef DPSG_DEFINE_BINARY_OPERATOR
// clang-tidy on

struct post_increment {
  template <class U>
  constexpr inline decltype(auto) operator()(U& u) const noexcept {
    return u++;
  }
};

struct post_decrement {
  template <class U>
  constexpr inline decltype(auto) operator()(U& u) const noexcept {
    return u--;
  }
};

namespace detail {
template <class Op,
          class Left,
          class Right,
          class Result = passthrough_t,
          class TransformLeft = get_value_t,
          class TransformRight = get_value_t>
struct implement_binary_operation;

template <class Op,
          class Arg,
          class Result = passthrough_t,
          class Transform = get_value_t>
struct implement_unary_operation;

#define DPSG_DEFINE_FRIEND_BINARY_OPERATOR_IMPLEMENTATION(op, sym)            \
  template <class Left,                                                       \
            class Right,                                                      \
            class Result,                                                     \
            class TransformLeft,                                              \
            class TransformRight>                                             \
  struct implement_binary_operation<op,                                       \
                                    Left,                                     \
                                    Right,                                    \
                                    Result,                                   \
                                    TransformLeft,                            \
                                    TransformRight> {                         \
    friend constexpr decltype(auto) operator sym(const Left& left,            \
                                                 const Right& right) {        \
      return Result{}(                                                        \
          op{}(TransformLeft{}(left), TransformRight{}(right)), left, right); \
    }                                                                         \
    friend constexpr decltype(auto) operator sym(Left& left,                  \
                                                 const Right& right) {        \
      return Result{}(                                                        \
          op{}(TransformLeft{}(left), TransformRight{}(right)), left, right); \
    }                                                                         \
    friend constexpr decltype(auto) operator sym(const Left& left,            \
                                                 Right& right) {              \
      return Result{}(                                                        \
          op{}(TransformLeft{}(left), TransformRight{}(right)), left, right); \
    }                                                                         \
    friend constexpr decltype(auto) operator sym(Left& left, Right& right) {  \
      return Result{}(                                                        \
          op{}(TransformLeft{}(left), TransformRight{}(right)), left, right); \
    }                                                                         \
  };

#define DPSG_DEFINE_FRIEND_SELF_ASSIGN_BINARY_OPERATOR_IMPLEMENTATION(op, sym) \
  template <class Left,                                                        \
            class Right,                                                       \
            class Result,                                                      \
            class TransformLeft,                                               \
            class TransformRight>                                              \
  struct implement_binary_operation<op,                                        \
                                    Left,                                      \
                                    Right,                                     \
                                    Result,                                    \
                                    TransformLeft,                             \
                                    TransformRight> {                          \
    template <                                                                 \
        class T,                                                               \
        std::enable_if_t<std::is_same<std::decay_t<T>, Left>::value, int> = 0> \
    friend constexpr decltype(auto) operator sym(T& left,                      \
                                                 const Right& right) {         \
      return Result{}(                                                         \
          op{}(TransformLeft{}(left), TransformRight{}(right)), left, right);  \
    }                                                                          \
  };

#define DPSG_DEFINE_FRIEND_UNARY_OPERATOR_IMPLEMENTATION(op, sym)             \
  template <class Arg, class Result, class Transform>                         \
  struct implement_unary_operation<op, Arg, Result, Transform> {              \
    template <                                                                \
        class T,                                                              \
        std::enable_if_t<std::is_same<std::decay_t<T>, Arg>::value, int> = 0> \
    friend constexpr decltype(auto) operator sym(T&& arg) {                   \
      return Result{}(op{}(Transform{}(std::forward<T>(arg))));               \
    }                                                                         \
  };

template <class L, class R, class TL>
struct implement_unary_operation<post_increment, L, R, TL> {
  template <class T,
            std::enable_if_t<std::is_same<std::decay_t<T>, L>::value, int> = 0>
  friend constexpr decltype(auto) operator++(T& left, int) {
    post_increment{}(TL{}(left));
    return left;
  }
};

template <class L, class R, class TL>
struct implement_unary_operation<post_decrement, L, R, TL> {
  template <class T,
            std::enable_if_t<std::is_same<std::decay_t<T>, L>::value, int> = 0>
  friend constexpr decltype(auto) operator--(T& left, int) {
    post_decrement{}(TL{}(left));
    return left;
  }
};

DPSG_APPLY_TO_BINARY_OPERATORS(
    DPSG_DEFINE_FRIEND_BINARY_OPERATOR_IMPLEMENTATION)
DPSG_APPLY_TO_SELF_ASSIGNING_BINARY_OPERATORS(
    DPSG_DEFINE_FRIEND_SELF_ASSIGN_BINARY_OPERATOR_IMPLEMENTATION)
DPSG_APPLY_TO_UNARY_OPERATORS(DPSG_DEFINE_FRIEND_UNARY_OPERATOR_IMPLEMENTATION)

#undef DPSG_DEFINE_FRIEND_UNARY_OPERATOR_IMPLEMENTATION
#undef DPSG_DEFINE_FRIEND_SELF_ASSIGN_BINARY_OPERATOR_IMPLEMENTATION
#undef DPSG_DEFINE_FRIEND_BINARY_OPERATOR_IMPLEMENTATION

}  // namespace detail

template <class Op,
          class Left,
          class Right,
          class Result = passthrough_t,
          class TransformLeft = get_value_t,
          class TransformRight = get_value_t>
using implement_binary_operation =
    detail::implement_binary_operation<Op,
                                       Left,
                                       Right,
                                       Result,
                                       TransformLeft,
                                       TransformRight>;

template <class Op,
          class Arg,
          class Result = passthrough_t,
          class Transform = get_value_t>
using implement_unary_operation =
    detail::implement_unary_operation<Op, Arg, Result, Transform>;

template <class Operation,
          class Arg,
          class Result = construct_t<Arg>,
          class Transform = get_value_t>
struct implement_symmetric_operation : implement_binary_operation<Operation,
                                                                  Arg,
                                                                  Arg,
                                                                  Result,
                                                                  Transform,
                                                                  Transform> {};

template <class Operation,
          class Left,
          class Right,
          class Return = passthrough_t,
          class TransformLeft = get_value_t,
          class TransformRight = get_value_t>
struct implement_commutative_operation
    : implement_binary_operation<Operation,
                                 Left,
                                 Right,
                                 Return,
                                 TransformLeft,
                                 TransformRight>,
      implement_binary_operation<Operation,
                                 Right,
                                 Left,
                                 Return,
                                 TransformRight,
                                 TransformLeft> {};

using comparison_operators = black_magic::
    tuple<equal, not_equal, lesser_equal, greater_equal, lesser, greater>;

using unary_boolean_operators = black_magic::tuple<boolean_not>;
using binary_boolean_operators = black_magic::tuple<boolean_and, boolean_or>;
using boolean_operators =
    black_magic::concat_tuples_t<unary_boolean_operators,
                                 binary_boolean_operators>;

using unary_bitwise_operators = black_magic::tuple<binary_not>;
using binary_bitwise_operators = black_magic::tuple<binary_and,
                                                    binary_or,
                                                    binary_xor,
                                                    binary_and_assign,
                                                    binary_or_assign,
                                                    binary_xor_assign,
                                                    shift_left,
                                                    shift_right,
                                                    shift_left_assign,
                                                    shift_right_assign>;
using bitwise_operators =
    black_magic::concat_tuples_t<unary_bitwise_operators,
                                 binary_bitwise_operators>;

using unary_arithmetic_operators = black_magic::tuple<negate,
                                                      positivate,
                                                      increment,
                                                      decrement,
                                                      post_increment,
                                                      post_decrement>;
using binary_arithmetic_operators = black_magic::tuple<plus,
                                                       minus,
                                                       multiplies,
                                                       divides,
                                                       modulo,
                                                       plus_assign,
                                                       minus_assign,
                                                       multiplies_assign,
                                                       divides_assign,
                                                       modulo_assign>;
using arithmetic_operators =
    black_magic::concat_tuples_t<unary_arithmetic_operators,
                                 binary_arithmetic_operators>;

template <class Arg1,
          class Arg2,
          class R,
          class T1 = get_value_t,
          class T2 = get_value_t>
struct make_commutative_operator {
  template <class Op>
  using type = implement_commutative_operation<Op, Arg1, Arg2, R, T1, T2>;
};
template <class Arg, class R = black_magic::deduce, class T = get_value_t>
struct make_symmetric_operator {
  template <class Op>
  using type = implement_symmetric_operation<Op, Arg, R, T>;
};
template <class Arg, class R = black_magic::deduce, class T = get_value_t>
struct make_unary_operator {
  template <class Op>
  using type = implement_unary_operation<Op, Arg, R, T>;
};
template <class Left,
          class Right,
          class R = black_magic::deduce,
          class TL = get_value_t,
          class TR = get_value_t>
struct make_binary_operator {
  template <class Op>
  using type = implement_binary_operation<Op, Left, Right, R, TL, TR>;
};

struct comparable {
  template <class Arg>
  struct type
      : black_magic::for_each<
            comparison_operators,
            make_symmetric_operator<
                Arg,
                construct_t<bool> /* passthrough causes rt errors on MSVC */>> {
  };
};

template <class Arg2>
struct comparable_with {
  template <class Arg1>
  struct type
      : black_magic::for_each<
            comparison_operators,
            make_commutative_operator<
                Arg1,
                Arg2,
                construct_t<bool> /* passthrough causes rt errors on MSVC */>> {
  };
};

struct arithmetic {
  template <class Arg>
  struct type
      : black_magic::for_each<binary_arithmetic_operators,
                              make_symmetric_operator<Arg, construct_t<Arg>>>,
        black_magic::for_each<
            unary_arithmetic_operators,
            make_unary_operator<Arg, construct_t<Arg>, get_value_t>> {};
};

template <class Op,
          class Return = black_magic::deduce,
          class Transform = get_value_t>
struct symmetric {
  template <class T>
  using type =
      implement_binary_operation<Op, T, T, Return, Transform, Transform>;
};

template <class Arg2,
          class R = black_magic::deduce,
          class T1 = get_value_t,
          class T2 = get_value_t>
struct arithmetically_compatible_with {
  template <class Arg1>
  struct type
      : black_magic::for_each<
            binary_arithmetic_operators,
            make_commutative_operator<
                Arg1,
                Arg2,
                black_magic::deduce_return_type<R, construct_t<Arg1>, Arg1>,
                T1,
                T2>> {};
};

template <class Op,
          class Arg2,
          class R = black_magic::deduce,
          class T1 = get_value_t,
          class T2 = get_value_t>
struct commutative_under {
  template <class Arg1>
  using type = implement_commutative_operation<
      Op,
      Arg1,
      Arg2,
      black_magic::deduce_return_type<R, construct_t<Arg1>, Arg1>,
      T1,
      T2>;
};

template <class Op,
          class Arg2,
          class R = black_magic::deduce,
          class T1 = get_value_t,
          class T2 = get_value_t>
struct compatible_under {
  template <class Arg1>
  using type = implement_binary_operation<
      Op,
      Arg1,
      Arg2,
      black_magic::deduce_return_type<R, construct_t<Arg1>, Arg1>,
      T1,
      T2>;
};

struct bitwise {
  template <class Type>
  struct type : black_magic::for_each<binary_bitwise_operators,
                                      make_symmetric_operator<Type>>,
                black_magic::for_each<unary_bitwise_operators,
                                      make_unary_operator<Type>> {};
};

template <class Arg2,
          class R = black_magic::deduce,
          class T1 = get_value_t,
          class T2 = get_value_t>
struct bitwise_compatible_with {
  template <class Arg1>
  struct type
      : black_magic::for_each<
            binary_bitwise_operators,
            make_commutative_operator<
                Arg1,
                Arg2,
                black_magic::deduce_return_type<R, construct_t<Arg1>, Arg1>,
                T1,
                T2>> {};
};

template <class T, class... Ts>
struct derive_t : Ts::template type<T>... {};

template <class Type, class Tag, class... Params>
struct strong_value : derive_t<strong_value<Type, Tag, Params...>, Params...> {
  using value_type = Type;

  template <
      class U,
      std::enable_if_t<std::is_convertible<std::decay_t<U>, value_type>::value,
                       int> = 0>
  constexpr explicit strong_value(U&& u) noexcept : value{std::forward<U>(u)} {}

  constexpr strong_value() noexcept : value{} {}

  value_type value;
};

template <class Type, class Tag, class... Params>
struct number
    : derive_t<number<Type, Tag, Params...>,
               arithmetic,
               comparable,
               arithmetically_compatible_with<
                   Type,
                   cast_to_then_construct_t<Type, black_magic::deduce>>,
               comparable_with<Type>,
               Params...> {
  using value_type = Type;

  static_assert(std::is_arithmetic<value_type>::value,
                "number expects a literal as base (first template parameter)");

  constexpr number() noexcept = default;

  template <class U,
            std::enable_if_t<
                std::is_constructible<value_type, std::decay_t<U>>::value,
                int> = 0>
  constexpr explicit number(U&& u) noexcept : value{std::forward<U>(u)} {}

  value_type value;
};

}  // namespace strong_types
}  // namespace dpsg

#endif  // GUARD_DPSG_STRONG_TYPES_HPP
