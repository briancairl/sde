// C++ Standard Library
#include <ostream>

// SDE
#include "sde/audio/sound.hpp"
#include "sde/audio/sound_io.hpp"
#include "sde/resource_cache_io.hpp"
#include "sde/resource_handle_io.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::serial
{

template <typename ArchiveT>
void save<ArchiveT, audio::SoundHandle>::operator()(ArchiveT& ar, const audio::SoundHandle& handle) const
{
  ar << handle.fundemental();
}

template <typename ArchiveT>
void load<ArchiveT, audio::SoundHandle>::operator()(ArchiveT& ar, audio::SoundHandle& handle) const
{
  ar >> handle.fundemental();
}

template <typename ArchiveT>
void save<ArchiveT, audio::SoundCache>::operator()(ArchiveT& ar, const audio::SoundCache& cache) const
{
  ar << cache.fundemental();
}

template <typename ArchiveT>
void load<ArchiveT, audio::SoundCache>::operator()(ArchiveT& ar, audio::SoundCache& cache) const
{
  ar >> cache.fundemental();
}

template struct save<binary_ofarchive, audio::SoundHandle>;
template struct load<binary_ifarchive, audio::SoundHandle>;
template struct save<binary_ofarchive, audio::SoundCache>;
template struct load<binary_ifarchive, audio::SoundCache>;

}  // namespace sde::serial
