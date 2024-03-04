/**
 * @copyright 2024-present Brian Cairl
 *
 * @file expected.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>

#ifdef __cpp_lib_expected
// C++ Standard Library
#include <expected>
#define SDE_CORE_COMMON_EXPECTED_NAMESPACE ::std
#else  // __cpp_lib_expected
// TartanLlama
#include <tl/expected.hpp>
#define SDE_CORE_COMMON_EXPECTED_NAMESPACE ::tl
#endif  // __cpp_lib_expected

namespace sde
{

template <typename T, typename E> using expected = SDE_CORE_COMMON_EXPECTED_NAMESPACE::expected<T, E>;

template <typename E> using unexpected = SDE_CORE_COMMON_EXPECTED_NAMESPACE::unexpected<E>;

template <class E> decltype(auto) make_unexpected(E&& e)
{
  return SDE_CORE_COMMON_EXPECTED_NAMESPACE::make_unexpected(std::forward<E>(e));
}

template <typename T, typename E> std::ostream& operator<<(std::ostream& os, const expected<T, E>& value_or_error)
{
  if (value_or_error.has_value())
  {
    return os << "{ value: " << value_or_error.value() << " }";
  }
  return os << "{ error: " << value_or_error.error() << " }";
}

}  // namespace sde
