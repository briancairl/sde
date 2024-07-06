/**
 * @copyright 2024-present Brian Cairl
 *
 * @file shader_io.hpp
 */
#pragma once

// SDE
#include "sde/graphics/shader_fwd.hpp"
#include "sde/graphics/shader_handle.hpp"
#include "sde/resource_handle_io.hpp"
#include "sde/serialization.hpp"

namespace sde::serial
{

template <typename Archive>
struct save<Archive, graphics::ShaderHandle> : save<Archive, typename graphics::ShaderHandle::base>
{};

template <typename Archive>
struct load<Archive, graphics::ShaderHandle> : load<Archive, typename graphics::ShaderHandle::base>
{};


template <typename Archive> struct save<Archive, graphics::ShaderCache>
{
  void operator()(Archive& ar, const graphics::ShaderCache& cache) const;
};

template <typename Archive> struct load<Archive, graphics::ShaderCache>
{
  void operator()(Archive& ar, graphics::ShaderCache& cache) const;
};

}  // namespace sde::serial
