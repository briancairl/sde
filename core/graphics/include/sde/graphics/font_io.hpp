/**
 * @copyright 2024-present Brian Cairl
 *
 * @file font_io.hpp
 */
#pragma once

// SDE
#include "sde/geometry_io.hpp"
#include "sde/graphics/font.hpp"
#include "sde/resource_handle_io.hpp"
#include "sde/serialization.hpp"

namespace sde::serial
{

template <typename Archive>
struct save<Archive, graphics::FontHandle> : save<Archive, typename graphics::FontHandle::base>
{};

template <typename Archive>
struct load<Archive, graphics::FontHandle> : load<Archive, typename graphics::FontHandle::base>
{};

template <typename Archive> struct save<Archive, graphics::FontCacheWithAssets>
{
  void operator()(Archive& ar, const graphics::FontCacheWithAssets& cache) const;
};

template <typename Archive> struct load<Archive, graphics::FontCacheWithAssets>
{
  void operator()(Archive& ar, graphics::FontCacheWithAssets& cache) const;
};

}  // namespace sde::serial
