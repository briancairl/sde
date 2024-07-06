/**
 * @copyright 2024-present Brian Cairl
 *
 * @file assets_io.hpp
 */
#pragma once

// SDE
#include "sde/graphics/assets_fwd.hpp"
#include "sde/serialization.hpp"

namespace sde::serial
{

template <typename Archive> struct save<Archive, graphics::Assets>
{
  void operator()(Archive& ar, const graphics::Assets& cache) const;
};

template <typename Archive> struct load<Archive, graphics::Assets>
{
  void operator()(Archive& ar, graphics::Assets& cache) const;
};

}  // namespace sde::serial
