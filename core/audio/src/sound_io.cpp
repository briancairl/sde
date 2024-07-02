// C++ Standard Library
#include <ostream>

// SDE
#include "sde/audio/sound_io.hpp"
#include "sde/logging.hpp"
#include "sde/serial/std/filesystem.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::serial
{

template <>
void save<binary_ofarchive, audio::SoundCacheWithAssets>::operator()(
  binary_ofarchive& ar,
  const audio::SoundCacheWithAssets& cache) const
{
  ar << named{"element_count", cache.handles().size()};
  for (const auto& [handle, path] : cache.handles())
  {
    ar << named{"handle", handle};
    ar << named{"path", path};
    const auto* sound_info = cache.get_if(handle);
    SDE_ASSERT_NE(sound_info, nullptr);
    ar << named{"options", sound_info->options};
  }
}


template <>
void load<binary_ifarchive, audio::SoundCacheWithAssets>::operator()(
  binary_ifarchive& ar,
  audio::SoundCacheWithAssets& cache) const
{
  std::size_t element_count{0};
  ar >> named{"element_count", element_count};
  for (std::size_t element_idx = 0; element_idx < element_count; ++element_idx)
  {
    audio::SoundHandle handle;
    ar >> named{"handle", handle};
    asset::path path;
    ar >> named{"path", path};
    audio::SoundOptions options;
    ar >> named{"options", options};
    cache.load(handle, path, options);
  }
}

}  // namespace sde::serial
