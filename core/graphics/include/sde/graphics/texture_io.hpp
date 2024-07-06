/**
 * @copyright 2024-present Brian Cairl
 *
 * @file texture_io.hpp
 */
#pragma once

// SDE
#include "sde/graphics/texture_fwd.hpp"
#include "sde/graphics/texture_handle.hpp"
#include "sde/resource_handle_io.hpp"
#include "sde/serialization.hpp"

namespace sde::serial
{

template <typename Archive>
struct save<Archive, graphics::TextureHandle> : save<Archive, typename graphics::TextureHandle::base>
{};

template <typename Archive>
struct load<Archive, graphics::TextureHandle> : load<Archive, typename graphics::TextureHandle::base>
{};

template <typename Archive> struct save<Archive, graphics::TextureOptions>
{
  void operator()(Archive& ar, const graphics::TextureOptions& options) const;
};

template <typename Archive> struct load<Archive, graphics::TextureOptions>
{
  void operator()(Archive& ar, graphics::TextureOptions& options) const;
};

template <typename Archive> struct save<Archive, graphics::TextureShape>
{
  void operator()(Archive& ar, const graphics::TextureShape& shape) const;
};

template <typename Archive> struct load<Archive, graphics::TextureShape>
{
  void operator()(Archive& ar, graphics::TextureShape& shape) const;
};

template <typename Archive> struct save<Archive, graphics::TextureCache>
{
  void operator()(Archive& ar, const graphics::TextureCache& cache) const;
};

template <typename Archive> struct load<Archive, graphics::TextureCache>
{
  void operator()(Archive& ar, graphics::TextureCache& cache) const;
};

}  // namespace sde::serial
