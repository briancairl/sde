/**
 * @copyright 2024-present Brian Cairl
 *
 * @file scene_handle.hpp
 */
#pragma once

// SDE
#include "sde/resource_handle.hpp"

namespace sde::game
{
struct SceneHandle : ResourceHandle<SceneHandle>
{
  SceneHandle() = default;
  explicit SceneHandle(id_type id) : ResourceHandle<SceneHandle>{id} {}
};
}  // namespace sde::game
