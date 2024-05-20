/**
 * @copyright 2024-present Brian Cairl
 *
 * @file texture_handle.hpp
 */
#pragma once

// SDE
#include "sde/graphics/resource_handle.hpp"

namespace sde::graphics
{

struct ShaderHandle : ResourceHandle<ShaderHandle>
{
  explicit ShaderHandle(id_type id) : ResourceHandle<ShaderHandle>{id} {}
};

}  // namespace sde::graphics

namespace std
{

template <> struct hash<sde::graphics::ShaderHandle> : sde::graphics::ResourceHandleHash
{};

}  // namespace std