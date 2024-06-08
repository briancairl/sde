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

struct GlyphSetHandle : ResourceHandle<GlyphSetHandle>
{
  explicit GlyphSetHandle(id_type id) : ResourceHandle<GlyphSetHandle>{id} {}
};

}  // namespace sde::graphics
