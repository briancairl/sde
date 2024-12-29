/**
 * @copyright 2024-present Brian Cairl
 *
 * @file native_script_instance_handle.hpp
 */
#pragma once

// SDE
#include "sde/resource_handle.hpp"

namespace sde::game
{
struct NativeScriptInstanceHandle : ResourceHandle<NativeScriptInstanceHandle>
{
  NativeScriptInstanceHandle() = default;
  explicit NativeScriptInstanceHandle(id_type id) : ResourceHandle<NativeScriptInstanceHandle>{id} {}
};
}  // namespace sde::game

namespace sde
{
template <> struct Hasher<game::NativeScriptInstanceHandle> : ResourceHandleHash
{};
}  // namespace sde
