/**
 * @copyright 2024-present Brian Cairl
 *
 * @file text.hpp
 */
#pragma once

// C++ Standard Library
#include <string_view>

// SDE
#include "sde/graphics/glyph_set_fwd.hpp"
#include "sde/graphics/renderer_fwd.hpp"
#include "sde/graphics/shapes.hpp"

namespace sde::graphics
{

class TypeSetter
{
public:
  explicit TypeSetter(const GlyphSetHandle& glyphs);

  void
  draw(RenderPass& rp, std::string_view text, const Vec2f& pos, float height_px, const Vec4f& color = Vec4f::Ones())
    const;

private:
  GlyphSetHandle glyph_set_handle_;
};

}  // namespace sde::graphics
