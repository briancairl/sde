/**
 * @copyright 2024-present Brian Cairl
 *
 * @file sound_data_io.hpp
 */
#pragma once

// SDE
#include "sde/audio/sound_data_fwd.hpp"
#include "sde/serialization.hpp"

namespace sde::serial
{

template <typename Archive> struct save<Archive, audio::SoundDataHandle>
{
  void operator()(Archive& ar, const audio::SoundDataHandle& handle) const;
};

template <typename Archive> struct load<Archive, audio::SoundDataHandle>
{
  void operator()(Archive& ar, audio::SoundDataHandle& handle) const;
};

template <typename Archive> struct save<Archive, audio::SoundDataCache>
{
  void operator()(Archive& ar, const audio::SoundDataCache& cache) const;
};

template <typename Archive> struct load<Archive, audio::SoundDataCache>
{
  void operator()(Archive& ar, audio::SoundDataCache& cache) const;
};

}  // namespace sde::serial
