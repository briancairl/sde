/**
 * @copyright 2024-present Brian Cairl
 *
 * @file font_io.hpp
 */
#pragma once

// SDE
#include "sde/graphics/font_fwd.hpp"
#include "sde/serialization.hpp"

namespace sde::serial
{

template <typename Archive> struct save<Archive, graphics::FontHandle>
{
  void operator()(Archive& ar, const graphics::FontHandle& handle) const;
};

template <typename Archive> struct load<Archive, graphics::FontHandle>
{
  void operator()(Archive& ar, graphics::FontHandle& handle) const;
};

template <typename Archive> struct save<Archive, graphics::FontCache>
{
  void operator()(Archive& ar, const graphics::FontCache& cache) const;
};

template <typename Archive> struct load<Archive, graphics::FontCache>
{
  void operator()(Archive& ar, graphics::FontCache& cache) const;
};

}  // namespace sde::serial
