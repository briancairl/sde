#pragma once

// C++ Standard Library
#include <iosfwd>
#include <iterator>
#include <type_traits>

namespace sde
{

template <typename T> using bare_t = std::remove_const_t<std::remove_reference_t<T>>;

namespace detail
{
template <typename T, typename Eval = decltype((std::declval<std::ostream&>() << std::declval<const T&>()))>
constexpr bool test_ostream_overload([[maybe_unused]] const T* _)
{
  return true;
}
constexpr bool test_ostream_overload(...) { return false; }


template <typename T, typename Eval = decltype(std::hash<T>{}(std::declval<T>()))>
constexpr bool test_std_hash([[maybe_unused]] const T* _)
{
  return true;
}
constexpr bool test_std_hash(...) { return false; }


template <typename T, typename Eval = decltype((std::begin(std::declval<T>()) == std::end(std::declval<T>())))>
constexpr bool test_iterable([[maybe_unused]] const T* _)
{
  return true;
}
constexpr bool test_iterable(...) { return false; }

}  // namespace detail

template <typename T>
struct has_std_ostream_overload
    : std::integral_constant<bool, detail::test_ostream_overload(std::add_pointer_t<T>{nullptr})>
{};

template <typename T> constexpr bool has_std_ostream_overload_v = has_std_ostream_overload<T>::value;

template <typename T>
struct has_std_hash_specialization : std::integral_constant<bool, detail::test_std_hash(std::add_pointer_t<T>{nullptr})>
{};

template <typename T> constexpr bool has_std_hash_specialization_v = has_std_hash_specialization<T>::value;

template <typename T>
struct is_iterable : std::integral_constant<bool, detail::test_iterable(std::add_pointer_t<T>{nullptr})>
{};

template <typename T> constexpr bool is_iterable_v = is_iterable<T>::value;

}  // namespace sde