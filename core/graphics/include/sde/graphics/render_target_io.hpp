/**
 * @copyright 2024-present Brian Cairl
 *
 * @file render_target_io.hpp
 */
#pragma once

// SDE
#include "sde/graphics/render_target_fwd.hpp"
#include "sde/serialization.hpp"

namespace sde::serial
{

template <typename Archive> struct serialize<Archive, graphics::RenderTarget>
{
  void operator()(Archive& ar, graphics::RenderTarget& render_target) const;
};

template <typename Archive> struct serialize<Archive, graphics::RenderTargetHandle>
{
  void operator()(Archive& ar, graphics::RenderTargetHandle& handle) const;
};

template <typename Archive> struct serialize<Archive, graphics::RenderTargetCache>
{
  void operator()(Archive& ar, graphics::RenderTargetCache& cache) const;
};

}  // namespace sde::serial
