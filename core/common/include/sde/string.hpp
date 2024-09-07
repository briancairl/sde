/**
 * @copyright 2024-present Brian Cairl
 *
 * @file string.hpp
 */
#pragma once

// C++ Standard Library
#include <string>
#include <string_view>
#include <type_traits>

// SDE
#include "sde/memory.hpp"

namespace sde
{

using string = std::basic_string<char, std::char_traits<char>, allocator<char>>;

}  // namespace sde

namespace std
{

template <> struct hash<sde::string>
{
  std::size_t operator()(const sde::string& v) const { return std::hash<std::string_view>{}(std::string_view{v}); }
};

}  // namespace std