/**
 * @copyright 2024-present Brian Cairl
 *
 * @file vector.hpp
 */
#pragma once

// C++ Standard Library
#include <type_traits>
#include <vector>

// SDE
#include "sde/memory.hpp"

namespace sde
{

template <typename T> using vector = std::vector<T, allocator<T>>;

template <typename T, typename... OtherTs>
vector<std::remove_const_t<std::remove_reference_t<T>>> make_vector(T&& first, OtherTs&&... others)
{
  vector<std::remove_const_t<std::remove_reference_t<T>>> v = {
    std::forward<T>(first), std::forward<OtherTs>(others)...};
  return v;
}

}  // namespace sde