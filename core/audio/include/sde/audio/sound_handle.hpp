/**
 * @copyright 2024-present Brian Cairl
 *
 * @file sound_handle.hpp
 */
#pragma once

// SDE
#include "sde/resource_handle.hpp"

namespace sde::audio
{

struct SoundHandle : ResourceHandle<SoundHandle>
{
  SoundHandle() = default;
  explicit SoundHandle(id_type id) : ResourceHandle<SoundHandle>{id} {}
};

}  // namespace sde::audio
