/**
 * @copyright 2024-present Brian Cairl
 *
 * @file unordered_set.hpp
 */
#pragma once

// C++ Standard Library
#include <unordered_set>

// SDE
#include "sde/memory.hpp"

namespace sde
{

template <typename Value, typename Hash = std::hash<Value>, typename Equal = std::equal_to<Value>>
using unordered_set = std::unordered_set<Value, Hash, Equal, allocator<Value>>;

}  // namespace sde