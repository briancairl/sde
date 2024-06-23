// C++ Standard Library
#include <fstream>
#include <sstream>
#include <string>

// SDE
#include "sde/audio/sound_data.hpp"
#include "sde/game/assets.hpp"
#include "sde/graphics/image.hpp"
#include "sde/logging.hpp"

namespace sde::game
{
namespace
{

template <typename CacheT> ResourceCacheFromDisk<CacheT, OnDiskLoader<CacheT>> FromDisk(CacheT& cache)
{
  return from_disk(cache, OnDiskLoader<CacheT>{});
}

}  // namespace

template <>
graphics::FontCache::result_type
OnDiskLoader<graphics::FontCache>::operator()(graphics::FontCache& cache, const asset::path& path) const
{
  SDE_LOG_INFO_FMT("font loaded from disk: %s", path.string().c_str());
  return cache.create(path);
}

template <>
graphics::ShaderCache::result_type
OnDiskLoader<graphics::ShaderCache>::operator()(graphics::ShaderCache& cache, const asset::path& path) const
{
  if (!asset::exists(path))
  {
    return make_unexpected(graphics::ShaderError::kAssetNotFound);
  }
  std::ifstream ifs{path};
  std::stringstream ioss;
  ioss << ifs.rdbuf();
  SDE_LOG_INFO_FMT("shader loaded from disk: %s", path.string().c_str());
  return cache.create(ioss.str());
}

template <>
graphics::TextureCache::result_type
OnDiskLoader<graphics::TextureCache>::operator()(graphics::TextureCache& cache, const asset::path& path) const
{
  auto texture_source_image = graphics::Image::load(path, {.flags = {.flip_vertically = true}});
  SDE_ASSERT_TRUE(texture_source_image.has_value());
  SDE_LOG_INFO_FMT("texture loaded from disk: %s", path.string().c_str());
  return cache.create(*texture_source_image);
}

template <>
audio::SoundCache::result_type
OnDiskLoader<audio::SoundCache>::operator()(audio::SoundCache& cache, const asset::path& path) const
{
  auto sound_data_or_error = audio::SoundData::load(path);
  SDE_ASSERT_TRUE(sound_data_or_error.has_value());
  SDE_LOG_INFO_FMT("sound loaded from disk: %s", path.string().c_str());
  return cache.create(*sound_data_or_error);
}

Assets::Assets() :
    audio{},
    graphics{},
    fonts_from_disk{FromDisk(graphics.fonts)},
    shaders_from_disk{FromDisk(graphics.shaders)},
    textures_from_disk{FromDisk(graphics.textures)},
    sounds_from_disk{FromDisk(audio.sounds)}
{}

}  // namespace sde::game
