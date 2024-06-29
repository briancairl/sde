/**
 * @copyright 2024-present Brian Cairl
 *
 * @file render_target_handle.hpp
 */
#pragma once

// SDE
#include "sde/resource_handle.hpp"

namespace sde::graphics
{

struct RenderTargetHandle : ResourceHandle<RenderTargetHandle>
{
  explicit RenderTargetHandle(id_type id) : ResourceHandle<RenderTargetHandle>{id} {}
};

}  // namespace sde::graphics
