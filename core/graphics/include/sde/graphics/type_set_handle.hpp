/**
 * @copyright 2024-present Brian Cairl
 *
 * @file glyph_set_handle.hpp
 */
#pragma once

// SDE
#include "sde/resource_handle.hpp"

namespace sde::graphics
{

struct TypeSetHandle : ResourceHandle<TypeSetHandle>
{
  TypeSetHandle() = default;
  explicit TypeSetHandle(id_type id) : ResourceHandle<TypeSetHandle>{id} {}
};

}  // namespace sde::graphics

namespace sde
{
template <> struct Hasher<graphics::TypeSetHandle> : ResourceHandleHash
{};
}  // namespace sde
