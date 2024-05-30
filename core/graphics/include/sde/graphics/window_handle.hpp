/**
 * @copyright 2024-present Brian Cairl
 *
 * @file window_handle.hpp
 */
#pragma once

// SDE
#include "sde/resource_handle.hpp"

namespace sde::graphics
{

struct WindowHandle : ResourceHandle<WindowHandle, void*, nullptr>
{
  WindowHandle() = default;
  explicit WindowHandle(id_type id) : ResourceHandle<WindowHandle, void*, nullptr>{id} {}
};

}  // namespace sde::graphics
