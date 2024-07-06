/**
 * @copyright 2024-present Brian Cairl
 *
 * @file render_target_io.hpp
 */
#pragma once

// SDE
#include "sde/graphics/render_target_fwd.hpp"
#include "sde/graphics/render_target_handle.hpp"
#include "sde/resource_handle_io.hpp"
#include "sde/serialization.hpp"

namespace sde::serial
{

template <typename Archive>
struct save<Archive, graphics::RenderTargetHandle> : save<Archive, typename graphics::RenderTargetHandle::base>
{};

template <typename Archive>
struct load<Archive, graphics::RenderTargetHandle> : load<Archive, typename graphics::RenderTargetHandle::base>
{};

template <typename Archive> struct save<Archive, graphics::RenderTargetCache>
{
  void operator()(Archive& ar, const graphics::RenderTargetCache& cache) const;
};

template <typename Archive> struct load<Archive, graphics::RenderTargetCache>
{
  void operator()(Archive& ar, graphics::RenderTargetCache& cache) const;
};

}  // namespace sde::serial
