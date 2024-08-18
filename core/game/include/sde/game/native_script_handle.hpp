/**
 * @copyright 2024-present Brian Cairl
 *
 * @file library_handle.hpp
 */
#pragma once

// SDE
#include "sde/resource_handle.hpp"

namespace sde::game
{
struct NativeScriptHandle : ResourceHandle<NativeScriptHandle>
{
  NativeScriptHandle() = default;
  explicit NativeScriptHandle(id_type id) : ResourceHandle<NativeScriptHandle>{id} {}
};
}  // namespace sde::game
