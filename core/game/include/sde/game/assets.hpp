/**
 * @copyright 2024-present Brian Cairl
 *
 * @file assets.hpp
 */
#pragma once

// SDE
#include "sde/audio/assets.hpp"
#include "sde/graphics/assets.hpp"
#include "sde/resource_cache_from_disk.hpp"

namespace sde::game
{

template <typename CacheT> struct OnDiskLoader
{
  typename CacheT::result_type operator()(CacheT& cache, const asset::path& path) const;
};

template <typename CacheT> using OnDisk = ResourceCacheFromDisk<CacheT, OnDiskLoader<CacheT>>;

/**
 * @brief Collection of active game assets
 */
struct Assets
{
  /// Collection of active audio assets
  audio::Assets audio;
  /// Collection of graphics audio assets
  graphics::Assets graphics;
  /// Font-from-disk resource loading wrapper
  OnDisk<graphics::FontCache> fonts_from_disk;
  /// Shader-from-disk resource loading wrapper
  OnDisk<graphics::ShaderCache> shaders_from_disk;
  /// Texture-from-disk resource loading wrapper
  OnDisk<graphics::TextureCache> textures_from_disk;
  /// Sound-from-disk resource loading wrapper
  OnDisk<audio::SoundCache> sounds_from_disk;

  Assets();
};

}  // namespace sde::game
