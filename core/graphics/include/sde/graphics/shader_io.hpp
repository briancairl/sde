/**
 * @copyright 2024-present Brian Cairl
 *
 * @file shader_io.hpp
 */
#pragma once

// SDE
#include "sde/graphics/shader.hpp"
#include "sde/resource_handle_io.hpp"
#include "sde/serialization.hpp"

namespace sde::serial
{

template <typename Archive>
struct save<Archive, graphics::ShaderHandle> : save<Archive, typename graphics::ShaderHandle::rh_type>
{};

template <typename Archive>
struct load<Archive, graphics::ShaderHandle> : load<Archive, typename graphics::ShaderHandle::rh_type>
{};


template <typename Archive> struct save<Archive, graphics::ShaderCacheWithAssets>
{
  void operator()(Archive& ar, const graphics::ShaderCacheWithAssets& cache) const;
};

template <typename Archive> struct load<Archive, graphics::ShaderCacheWithAssets>
{
  void operator()(Archive& ar, graphics::ShaderCacheWithAssets& cache) const;
};

}  // namespace sde::serial
