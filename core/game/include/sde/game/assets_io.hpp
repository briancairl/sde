/**
 * @copyright 2024-present Brian Cairl
 *
 * @file assets_io.hpp
 */
#pragma once

// SDE
#include "sde/game/assets_fwd.hpp"
#include "sde/serialization.hpp"

namespace sde::serial
{

template <typename Archive> struct save<Archive, game::Assets>
{
  void operator()(Archive& ar, const game::Assets& cache) const;
};

template <typename Archive> struct load<Archive, game::Assets>
{
  void operator()(Archive& ar, game::Assets& cache) const;
};

}  // namespace sde::serial
