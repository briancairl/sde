/**
 * @copyright 2024-present Brian Cairl
 *
 * @file font_handle.hpp
 */
#pragma once

// SDE
#include "sde/resource_handle.hpp"

namespace sde::graphics
{

struct FontHandle : ResourceHandle<FontHandle>
{
  FontHandle() = default;
  explicit FontHandle(id_type id) : ResourceHandle<FontHandle>{id} {}
};

}  // namespace sde::graphics

namespace sde
{
template <> struct Hasher<graphics::FontHandle> : ResourceHandleHash
{};
}  // namespace sde