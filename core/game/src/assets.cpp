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

graphics::FontCache::result_type LoadFontFromDisk::operator()(graphics::FontCache& cache, const asset::path& path) const
{
  SDE_LOG_INFO_FMT("font loaded from disk: %s", path.string().c_str());
  return cache.create(path);
}

graphics::ShaderCache::result_type
LoadShaderFromDisk::operator()(graphics::ShaderCache& cache, const asset::path& path) const
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

graphics::TextureCache::result_type
LoadTextureFromDisk::operator()(graphics::TextureCache& cache, const asset::path& path) const
{
  auto texture_source_image = graphics::Image::load(path, {.flags = {.flip_vertically = true}});
  SDE_ASSERT_TRUE(texture_source_image.has_value());
  SDE_LOG_INFO_FMT("texture loaded from disk: %s", path.string().c_str());
  return cache.create(*texture_source_image);
}

audio::SoundCache::result_type LoadSoundFromDisk::operator()(audio::SoundCache& cache, const asset::path& path) const
{
  auto sound_data_or_error = audio::SoundData::load(path);
  SDE_ASSERT_TRUE(sound_data_or_error.has_value());
  SDE_LOG_INFO_FMT("sound loaded from disk: %s", path.string().c_str());
  return cache.create(*sound_data_or_error);
}

Assets::Assets() :
    audio{},
    graphics{},
    fonts_from_disk{from_disk(graphics.fonts, LoadFontFromDisk{})},
    shaders_from_disk{from_disk(graphics.shaders, LoadShaderFromDisk{})},
    textures_from_disk{from_disk(graphics.textures, LoadTextureFromDisk{})},
    sounds_from_disk{from_disk(audio.sounds, LoadSoundFromDisk{})}
{}

}  // namespace sde::game
