/**
 * @copyright 2024-present Brian Cairl
 *
 * @file entity_handle.hpp
 */
#pragma once

// SDE
#include "sde/resource_handle.hpp"

namespace sde::game
{
struct EntityHandle : ResourceHandle<EntityHandle>
{
  EntityHandle() = default;
  explicit EntityHandle(id_type id) : ResourceHandle<EntityHandle>{id} {}
};
}  // namespace sde::game

namespace sde
{
template <> struct Hasher<game::EntityHandle> : ResourceHandleHash
{};
}  // namespace sde
