/**
 * @copyright 2024-present Brian Cairl
 *
 * @file assets_io.hpp
 */
#pragma once

// SDE
#include "sde/audio/assets_fwd.hpp"
#include "sde/serialization.hpp"

namespace sde::serial
{

template <typename Archive> struct save<Archive, audio::Assets>
{
  void operator()(Archive& ar, const audio::Assets& cache) const;
};

template <typename Archive> struct load<Archive, audio::Assets>
{
  void operator()(Archive& ar, audio::Assets& cache) const;
};

}  // namespace sde::serial
