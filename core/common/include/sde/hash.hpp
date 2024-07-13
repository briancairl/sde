/**
 * @copyright 2024-present Brian Cairl
 *
 * @file hash.hpp
 */
#pragma once

// C++ Standard Library
#include <functional>
#include <iostream>
#include <type_traits>
namespace sde
{
constexpr std::size_t kHashSeed{0x9e3779b9};

constexpr std::size_t hash_combine(std::size_t h) { return h; }

template <typename... OtherTs> constexpr std::size_t hash_combine(std::size_t lhs, std::size_t rhs, OtherTs... others)
{
  return rhs + kHashSeed + (lhs << 6) + (lhs >> 2);
}

template <typename T> struct Hasher : std::hash<T>
{};

constexpr std::size_t Hash() { return kHashSeed; }

template <typename T, typename... OtherTs> std::size_t Hash(const T& first, OtherTs&&... others)
{
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
  return hash_combine(Hasher<T>{}(first), Hash(std::forward<OtherTs>(others)...));
}

}  // namespace sde