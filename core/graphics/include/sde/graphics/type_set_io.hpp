/**
 * @copyright 2024-present Brian Cairl
 *
 * @file type_set_io.hpp
 */
#pragma once

// SDE
#include "sde/graphics/type_set_fwd.hpp"
#include "sde/serialization.hpp"

namespace sde::serial
{

template <typename Archive> struct save<Archive, graphics::TypeSetHandle>
{
  void operator()(Archive& ar, const graphics::TypeSetHandle& handle) const;
};

template <typename Archive> struct load<Archive, graphics::TypeSetHandle>
{
  void operator()(Archive& ar, graphics::TypeSetHandle& handle) const;
};

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
