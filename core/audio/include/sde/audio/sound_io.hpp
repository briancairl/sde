/**
 * @copyright 2024-present Brian Cairl
 *
 * @file sound_io.hpp
 */
#pragma once

// SDE
#include "sde/audio/sound.hpp"
#include "sde/resource_handle_io.hpp"
#include "sde/serialization.hpp"

namespace sde::serial
{

template <typename Archive> struct save<Archive, audio::SoundHandle> : save<Archive, typename audio::SoundHandle::base>
{};

template <typename Archive> struct load<Archive, audio::SoundHandle> : load<Archive, typename audio::SoundHandle::base>
{};

template <typename Archive> struct serialize<Archive, audio::SoundOptions>
{
  void operator()(Archive& ar, audio::SoundOptions& options) const { (void)options; }
};

template <typename Archive> struct save<Archive, audio::SoundCacheWithAssets>
{
  void operator()(Archive& ar, const audio::SoundCacheWithAssets& cache) const;
};

template <typename Archive> struct load<Archive, audio::SoundCacheWithAssets>
{
  void operator()(Archive& ar, audio::SoundCacheWithAssets& cache) const;
};

}  // namespace sde::serial
