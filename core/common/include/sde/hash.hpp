/**
 * @copyright 2024-present Brian Cairl
 *
 * @file hash.hpp
 */
#pragma once

// C++ Standard Library
#include <functional>
#include <iosfwd>
#include <type_traits>

// SDE
#include "sde/traits.hpp"

namespace sde
{
constexpr std::size_t kHashOffset{0x9e3779b9};

constexpr std::size_t hash_digest(std::size_t h) { return h + kHashOffset; }

template <typename... OtherTs> constexpr std::size_t hash_digest(std::size_t lhs, std::size_t rhs)
{
  return hash_digest(rhs) + (lhs << 6) + (lhs >> 2);
}

struct Hash
{
  std::size_t value = hash_digest(0);

  Hash& operator+=(const Hash& other)
  {
    this->value = hash_digest(this->value, other.value);
    return *this;
  }
};

constexpr Hash operator+(Hash lhs, Hash rhs) { return {hash_digest(lhs.value, rhs.value)}; }

constexpr bool operator==(Hash lhs, Hash rhs) { return lhs.value == rhs.value; }

constexpr bool operator!=(Hash lhs, Hash rhs) { return lhs.value != rhs.value; }

constexpr bool operator==(Hash lhs, std::size_t rhs_value) { return lhs.value == rhs_value; }

constexpr bool operator!=(Hash lhs, std::size_t rhs_value) { return lhs.value != rhs_value; }

template <typename T, bool U = T::kDoNotHash> constexpr bool has_do_not_hash_tag(T*) { return U; }

constexpr bool has_do_not_hash_tag(...) { return false; }

template <typename T> constexpr bool do_not_hash_v = has_do_not_hash_tag(static_cast<T*>(nullptr));

template <typename T> struct Hasher
{
  template <bool U = do_not_hash_v<T>> constexpr std::enable_if_t<U, Hash> operator()(const T& v) const { return {0}; }

  template <bool U = do_not_hash_v<T>> constexpr std::enable_if_t<!U, Hash> operator()(const T& v) const
  {
    static_assert(has_std_hash_specialization<T>() or is_iterable<T>());
    if constexpr (has_std_hash_specialization<T>())
    {
      return {std::hash<T>{}(v)};
    }
    else if constexpr (is_iterable<T>())
    {
      Hash h;
      std::for_each(std::begin(v), std::end(v), [&h](const auto& v) { h += Hasher<bare_t<decltype(v)>>{}(v); });
      return h;
    }
    return {0};
  }
};

template <typename F, typename S> struct Hasher<std::pair<F, S>>
{
  constexpr Hash operator()(const std::pair<F, S>& v) const
  {
    Hash h;
    h += Hasher<F>{}(std::get<0>(v));
    h += Hasher<S>{}(std::get<1>(v));
    return h;
  }
};

template <typename T> using hashable_t = std::remove_const_t<std::remove_reference_t<T>>;

template <typename T> constexpr std::size_t ComputeTypeHashValue()
{
  return std::hash<std::string_view>{}(std::string_view{__PRETTY_FUNCTION__});
}

template <typename T> constexpr Hash ComputeTypeHash()
{
  return {ComputeTypeHashValue<std::remove_reference_t<std::remove_const_t<T>>>()};
}

constexpr Hash ComputeHash() { return {}; }

template <typename FirstT, typename... OtherTs> constexpr Hash ComputeHash(const FirstT& first, OtherTs&&... others)
{
  return Hasher<FirstT>{}(first) + ComputeHash(others...);
}

inline std::ostream& operator<<(std::ostream& os, const Hash& hash) { return os << "{ hash: " << hash.value << " }"; }

}  // namespace sde