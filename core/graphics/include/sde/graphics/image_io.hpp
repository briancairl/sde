/**
 * @copyright 2024-present Brian Cairl
 *
 * @file image_io.hpp
 */
#pragma once

// SDE
#include "sde/graphics/image_fwd.hpp"
#include "sde/serialization.hpp"

namespace sde::serial
{

template <typename Archive> struct save<Archive, graphics::ImageHandle>
{
  void operator()(Archive& ar, const graphics::ImageHandle& handle) const;
};

template <typename Archive> struct load<Archive, graphics::ImageHandle>
{
  void operator()(Archive& ar, graphics::ImageHandle& handle) const;
};

template <typename Archive> struct save<Archive, graphics::ImageOptions>
{
  void operator()(Archive& ar, const graphics::ImageOptions& options) const;
};

template <typename Archive> struct load<Archive, graphics::ImageOptions>
{
  void operator()(Archive& ar, graphics::ImageOptions& options) const;
};

template <typename Archive> struct save<Archive, graphics::ImageCache>
{
  void operator()(Archive& ar, const graphics::ImageCache& cache) const;
};

template <typename Archive> struct load<Archive, graphics::ImageCache>
{
  void operator()(Archive& ar, graphics::ImageCache& cache) const;
};

}  // namespace sde::serial
