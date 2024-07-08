// C++ Standard Library
#include <ostream>

// SDE
#include "sde/audio/sound.hpp"
#include "sde/audio/sound_data_io.hpp"
#include "sde/audio/sound_io.hpp"
#include "sde/logging.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::serial
{

template <>
void save<binary_ofarchive, audio::SoundCache>::operator()(binary_ofarchive& ar, const audio::SoundCache& cache) const
{
  ar << named{"element_count", cache.size()};
  for (const auto& [handle, info] : cache)
  {
    ar << named{"handle", handle};
    ar << named{"sound_data", info.sound_data};
  }
}


template <>
void load<binary_ifarchive, audio::SoundCache>::operator()(binary_ifarchive& ar, audio::SoundCache& cache) const
{
  std::size_t element_count{0};
  ar >> named{"element_count", element_count};
  for (std::size_t element_idx = 0; element_idx < element_count; ++element_idx)
  {
    audio::SoundHandle handle;
    ar >> named{"handle", handle};
    audio::SoundDataHandle sound_data;
    ar >> named{"sound_data", sound_data};
    SDE_ASSERT_OK(cache.insert(handle, sound_data, ResourceLoading::kDeferred));
  }
}

}  // namespace sde::serial
