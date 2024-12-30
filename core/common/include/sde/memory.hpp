/**
 * @copyright 2024-present Brian Cairl
 *
 * @file memory.hpp
 */
#pragma once

// C++ Standard Library
#include <memory>

namespace sde
{
template <typename T> using allocator = ::std::allocator<T>;
}  // namespace sde
