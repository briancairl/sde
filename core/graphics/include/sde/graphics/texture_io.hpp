/**
 * @copyright 2024-present Brian Cairl
 *
 * @file texture_io.hpp
 */
#pragma once

// SDE
#include "sde/geometry_io.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/resource_handle_io.hpp"
#include "sde/serialization.hpp"

namespace sde::serial
{

template <typename Archive>
struct save<Archive, graphics::TextureHandle> : save<Archive, typename graphics::TextureHandle::rh_type>
{};

template <typename Archive>
struct load<Archive, graphics::TextureHandle> : load<Archive, typename graphics::TextureHandle::rh_type>
{};

template <typename Archive> struct serialize<Archive, graphics::TextureFlags>
{
  void operator()(Archive& ar, graphics::TextureFlags& flags) const
  {
    ar& named{"mask", reinterpret_cast<std::uint8_t&>(flags)};
  }
};

template <typename Archive> struct serialize<Archive, graphics::TextureOptions>
{
  void operator()(Archive& ar, graphics::TextureOptions& options) const
  {
    ar& named{"u_wrapping", options.u_wrapping};
    ar& named{"v_wrapping", options.v_wrapping};
    ar& named{"min_sampling", options.min_sampling};
    ar& named{"mag_sampling", options.mag_sampling};
    ar& named{"flags", options.flags};
  }
};

template <typename Archive> struct serialize<Archive, graphics::TextureShape>
{
  void operator()(Archive& ar, graphics::TextureShape& shape) const { ar& named{"value", shape.value}; }
};

template <typename Archive> struct save<Archive, graphics::TextureCacheWithAssets>
{
  void operator()(Archive& ar, const graphics::TextureCacheWithAssets& cache) const;
};

template <typename Archive> struct load<Archive, graphics::TextureCacheWithAssets>
{
  void operator()(Archive& ar, graphics::TextureCacheWithAssets& cache) const;
};

}  // namespace sde::serial
