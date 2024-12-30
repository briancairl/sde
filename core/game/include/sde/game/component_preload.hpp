/**
 * @copyright 2024-present Brian Cairl
 *
 * @file component_preload.hpp
 */
#pragma once

// SDE
#include "sde/asset.hpp"
#include "sde/game/component_fwd.hpp"
#include "sde/game/game_resources_fwd.hpp"

namespace sde::game
{

expected<void, ComponentError> componentPreload(GameResources& resources, const asset::path& preload_manifest_path);

}  // namespace sde::game
