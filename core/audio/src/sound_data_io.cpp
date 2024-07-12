// C++ Standard Library
#include <ostream>

// SDE
#include "sde/audio/sound_data.hpp"
#include "sde/audio/sound_data_io.hpp"
#include "sde/resource_cache_io.hpp"
#include "sde/resource_handle_io.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::serial
{

template <typename ArchiveT>
void save<ArchiveT, audio::SoundDataHandle>::operator()(ArchiveT& ar, const audio::SoundDataHandle& handle) const
{
  ar << handle.fundemental();
}

template <typename ArchiveT>
void load<ArchiveT, audio::SoundDataHandle>::operator()(ArchiveT& ar, audio::SoundDataHandle& handle) const
{
  ar >> handle.fundemental();
}

template <typename ArchiveT>
void save<ArchiveT, audio::SoundDataCache>::operator()(ArchiveT& ar, const audio::SoundDataCache& cache) const
{
  ar << cache.fundemental();
}

template <typename ArchiveT>
void load<ArchiveT, audio::SoundDataCache>::operator()(ArchiveT& ar, audio::SoundDataCache& cache) const
{
  ar >> cache.fundemental();
}

template struct save<binary_ofarchive, audio::SoundDataHandle>;
template struct load<binary_ifarchive, audio::SoundDataHandle>;
template struct save<binary_ofarchive, audio::SoundDataCache>;
template struct load<binary_ifarchive, audio::SoundDataCache>;

}  // namespace sde::serial
