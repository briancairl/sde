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

struct LoadFontFromDisk
{
  graphics::FontCache::result_type operator()(graphics::FontCache& cache, const asset::path& path) const;
};

struct LoadTextureFromDisk
{
  graphics::TextureCache::result_type operator()(graphics::TextureCache& cache, const asset::path& path) const;
};

struct LoadSoundFromDisk
{
  audio::SoundCache::result_type operator()(audio::SoundCache& cache, const asset::path& path) const;
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
  /// Font-from-disk resource loading wrapper
  ResourceCacheFromDisk<graphics::FontCache, LoadFontFromDisk> fonts_from_disk;
  /// Texture-from-disk resource loading wrapper
  ResourceCacheFromDisk<graphics::TextureCache, LoadTextureFromDisk> textures_from_disk;
  /// Sound-from-disk resource loading wrapper
  ResourceCacheFromDisk<audio::SoundCache, LoadSoundFromDisk> sounds_from_disk;

  Assets();
};

}  // namespace sde::game
