/**
 * @copyright 2024-present Brian Cairl
 *
 * @file hash.hpp
 */
#pragma once

// C++ Standard Library
#include <functional>

namespace sde
{

constexpr std::size_t hash_combine(std::size_t lhs, std::size_t rhs)
{
  constexpr std::size_t kMagic = 0x9e3779b9;
  return rhs + kMagic + (lhs << 6) + (lhs >> 2);
}

template <typename T> struct Hasher : std::hash<T>
{};

template <typename T> std::size_t hash(const T& v) { return Hasher<T>{}(v); }

template <typename LeftT, typename RightT, typename... OtherTs>
std::size_t hash(const LeftT& lhs, const RightT& rhs, OtherTs&&... others)
{
  return hash_combine(hash(lhs), hash(rhs, std::forward<OtherTs>(others)...));
}

}  // namespace sde