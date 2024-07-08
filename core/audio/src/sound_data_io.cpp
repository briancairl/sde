// C++ Standard Library
#include <ostream>

// SDE
#include "sde/audio/sound_data.hpp"
#include "sde/audio/sound_data_io.hpp"
#include "sde/logging.hpp"
#include "sde/serial/std/filesystem.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::serial
{

template <>
void save<binary_ofarchive, audio::SoundDataCache>::operator()(binary_ofarchive& ar, const audio::SoundDataCache& cache)
  const
{
  ar << named{"element_count", cache.size()};
  for (const auto& [handle, info] : cache)
  {
    ar << named{"handle", handle};
    ar << named{"path", info.path};
  }
}


template <>
void load<binary_ifarchive, audio::SoundDataCache>::operator()(binary_ifarchive& ar, audio::SoundDataCache& cache) const
{
  std::size_t element_count{0};
  ar >> named{"element_count", element_count};
  for (std::size_t element_idx = 0; element_idx < element_count; ++element_idx)
  {
    audio::SoundDataHandle handle;
    ar >> named{"handle", handle};
    asset::path path;
    ar >> named{"path", path};
    SDE_ASSERT_OK(cache.insert(handle, path, ResourceLoading::kDeferred));
  }
}

}  // namespace sde::serial
