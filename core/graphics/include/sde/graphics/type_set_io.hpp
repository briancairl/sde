/**
 * @copyright 2024-present Brian Cairl
 *
 * @file type_set_io.hpp
 */
#pragma once

// SDE
#include "sde/geometry_io.hpp"
#include "sde/graphics/type_set.hpp"
#include "sde/resource_handle_io.hpp"
#include "sde/serialization.hpp"

namespace sde::serial
{

template <typename Archive>
struct save<Archive, graphics::TypeSetHandle> : save<Archive, typename graphics::TypeSetHandle::base>
{};

template <typename Archive>
struct load<Archive, graphics::TypeSetHandle> : load<Archive, typename graphics::TypeSetHandle::base>
{};

template <typename Archive> struct serialize<Archive, graphics::TypeSetOptions>
{
  void operator()(Archive& ar, graphics::TypeSetOptions& options) const { ar& named{"height_px", options.height_px}; }
};

template <typename Archive> struct save<Archive, graphics::TypeSetCache>
{
  void operator()(Archive& ar, const graphics::TypeSetCache& cache) const;
};

template <typename Archive> struct load<Archive, graphics::TypeSetCache>
{
  void operator()(Archive& ar, graphics::TypeSetCache& cache) const;
};

}  // namespace sde::serial
