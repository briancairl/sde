/**
 * @copyright 2024-present Brian Cairl
 *
 * @file type.hpp
 */
#pragma once

namespace sde
{

template <typename T> struct TypeTag
{};

template <typename T> constexpr TypeTag<const T> Type = {};

}  // namespace sde
