/**
 * @copyright 2024-present Brian Cairl
 *
 * @file library_handle.hpp
 */
#pragma once

// SDE
#include "sde/resource_handle.hpp"

namespace sde::game
{
struct LibraryHandle : ResourceHandle<LibraryHandle>
{
  LibraryHandle() = default;
  explicit LibraryHandle(id_type id) : ResourceHandle<LibraryHandle>{id} {}
};
}  // namespace sde::game

namespace sde
{
template <> struct Hasher<game::LibraryHandle> : ResourceHandleHash
{};
}  // namespace sde
