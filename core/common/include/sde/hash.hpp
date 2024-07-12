/**
 * @copyright 2024-present Brian Cairl
 *
 * @file hash.hpp
 */
#pragma once

// C++ Standard Library
#include <functional>
#include <type_traits>

namespace sde
{

constexpr std::size_t hash_combine() { return 0; }

constexpr std::size_t hash_combine(std::size_t h) { return h; }

template <typename... OtherTs> constexpr std::size_t hash_combine(std::size_t lhs, std::size_t rhs, OtherTs... others)
{
  constexpr std::size_t kMagic = 0x9e3779b9;
  const std::size_t hash = rhs + kMagic + (lhs << 6) + (lhs >> 2);
  return hash_combine(hash, std::forward<OtherTs>(others)...);
}

template <typename T> struct Hasher : std::hash<T>
{};

template <typename... Ts> std::size_t HashMultiple(Ts&&... values)
{
  return hash_combine(Hasher<std::remove_const_t<std::remove_reference_t<Ts>>>{}(values)...);
}

}  // namespace sde