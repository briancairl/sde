/**
 * @copyright 2024-present Brian Cairl
 *
 * @file texture_handle.hpp
 */
#pragma once

// SDE
#include "sde/resource_handle.hpp"

namespace sde::graphics
{

struct ShaderHandle : ResourceHandle<ShaderHandle>
{
  ShaderHandle() = default;
  explicit ShaderHandle(id_type id) : ResourceHandle<ShaderHandle>{id} {}
};

}  // namespace sde::graphics
