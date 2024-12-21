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
