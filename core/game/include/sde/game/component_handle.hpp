/**
 * @copyright 2024-present Brian Cairl
 *
 * @file component_handle.hpp
 */
#pragma once

// SDE
#include "sde/resource_handle.hpp"

namespace sde::game
{
struct ComponentHandle : ResourceHandle<ComponentHandle>
{
  ComponentHandle() = default;
  explicit ComponentHandle(id_type id) : ResourceHandle<ComponentHandle>{id} {}
};
}  // namespace sde::game
