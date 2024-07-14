/**
 * @copyright 2024-present Brian Cairl
 *
 * @file resource_tag.hpp
 */
#pragma once

// C++ Standard Library
#include <type_traits>

namespace sde
{

struct resource_tag
{};

template <typename T> struct has_resource_tag : std::is_base_of<resource_tag, T>
{};

template <typename T> constexpr bool has_resource_tag_v = has_resource_tag<std::remove_const_t<T>>::value;

}  // namespace sde
