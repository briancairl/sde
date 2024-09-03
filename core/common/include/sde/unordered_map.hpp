/**
 * @copyright 2024-present Brian Cairl
 *
 * @file unordered_map.hpp
 */
#pragma once

// C++ Standard Library
#include <unordered_map>

// SDE
#include "sde/memory.hpp"

namespace sde
{

template <typename Key, typename Value, typename Hash = std::hash<Key>, typename Equal = std::equal_to<Key>>
using unordered_map = std::unordered_map<Key, Value, Hash, Equal, allocator<std::pair<const Key, Value>>>;

}  // namespace sde