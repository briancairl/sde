/**
 * @copyright 2024-present Brian Cairl
 *
 * @file sound_data_handle.hpp
 */
#pragma once

// SDE
#include "sde/resource_handle.hpp"

namespace sde::audio
{

struct SoundDataHandle : ResourceHandle<SoundDataHandle>
{
  SoundDataHandle() = default;
  explicit SoundDataHandle(id_type id) : ResourceHandle<SoundDataHandle>{id} {}
};

}  // namespace sde::audio
