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

struct TextureHandle : ResourceHandle<TextureHandle>
{
  TextureHandle() = default;
  explicit TextureHandle(id_type id) : ResourceHandle<TextureHandle>{id} {}
};

}  // namespace sde::graphics

namespace sde
{
template <> struct Hasher<graphics::TextureHandle> : ResourceHandleHash
{};
}  // namespace sde