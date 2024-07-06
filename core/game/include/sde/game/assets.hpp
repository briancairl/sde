/**
 * @copyright 2024-present Brian Cairl
 *
 * @file assets.hpp
 */
#pragma once

// SDE
#include "sde/audio/assets.hpp"
#include "sde/graphics/assets.hpp"

namespace sde::game
{

enum class AssetError
{
  kFailedGraphicsLoading,
  kFailedAudioLoading,
};

/**
 * @brief Collection of active game assets
 */
struct Assets
{
  /// Collection of active audio assets
  audio::Assets audio;
  /// Collection of graphics audio assets
  graphics::Assets graphics;

  Assets();

  expected<void, AssetError> refresh();
};

}  // namespace sde::game
