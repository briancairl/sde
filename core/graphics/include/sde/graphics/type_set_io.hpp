/**
 * @copyright 2024-present Brian Cairl
 *
 * @file type_set_io.hpp
 */
#pragma once

// SDE
#include "sde/graphics/type_set_fwd.hpp"
#include "sde/graphics/type_set_handle.hpp"
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

template <typename Archive> struct save<Archive, graphics::TypeSetOptions>
{
  void operator()(Archive& ar, const graphics::TypeSetOptions& options) const;
};

template <typename Archive> struct load<Archive, graphics::TypeSetOptions>
{
  void operator()(Archive& ar, graphics::TypeSetOptions& options) const;
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
