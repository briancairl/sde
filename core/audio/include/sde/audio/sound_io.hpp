/**
 * @copyright 2024-present Brian Cairl
 *
 * @file sound_io.hpp
 */
#pragma once

// SDE
#include "sde/audio/sound_fwd.hpp"
#include "sde/audio/sound_handle.hpp"
#include "sde/resource_handle_io.hpp"
#include "sde/serialization.hpp"

namespace sde::serial
{

template <typename Archive> struct save<Archive, audio::SoundHandle> : save<Archive, typename audio::SoundHandle::base>
{};

template <typename Archive> struct load<Archive, audio::SoundHandle> : load<Archive, typename audio::SoundHandle::base>
{};

template <typename Archive> struct save<Archive, audio::SoundCache>
{
  void operator()(Archive& ar, const audio::SoundCache& cache) const;
};

template <typename Archive> struct load<Archive, audio::SoundCache>
{
  void operator()(Archive& ar, audio::SoundCache& cache) const;
};

}  // namespace sde::serial
