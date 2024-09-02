/**
 * @copyright 2024-present Brian Cairl
 *
 * @file archive.hpp
 */
#pragma once

// SDE
#include "sde/memory.hpp"

/// EnTT
#include <entt/entity/registry.hpp>

namespace sde::game
{

using EntityID = entt::entity;
using Registry = entt::basic_registry<EntityID, common_allocator<EntityID>>;

}  // namespace sde::game
