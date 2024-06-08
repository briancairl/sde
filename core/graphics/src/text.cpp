// C++ Standard Library
#include <algorithm>
#include <iostream>

// SDE
#include "sde/graphics/assets.hpp"
#include "sde/graphics/renderer.hpp"
#include "sde/graphics/shapes.hpp"
#include "sde/graphics/text.hpp"

namespace sde::graphics
{
namespace
{

std::vector<TexturedQuad> textured_quad_buffer;

}  // namespace

TypeSetter::TypeSetter(const GlyphSetHandle& glyphs) : glyph_set_handle_{glyphs} {}

void TypeSetter::draw(RenderPass& rp, std::string_view text, const Vec2f& pos, float height, const Vec4f& color) const
{
  const auto* glyphs = rp.assets().glyph_sets(glyph_set_handle_);
  if (glyphs == nullptr)
  {
    return;
  }

  const auto text_bounds = glyphs->getTextBounds(text);
  const float text_height_original = text_bounds.max().y() - text_bounds.min().y();
  const float scaling = height / text_height_original;

  const Bounds2f text_aabb{
    pos + text_bounds.min().cast<float>() * scaling, pos + text_bounds.max().cast<float>() * scaling};
  if (!rp.getViewportInWorldBounds().intersects(text_aabb))
  {
    return;
  }

  const auto texture_unit_opt = rp.resources().textures(glyphs->glyph_atlas);
  if (!texture_unit_opt.has_value())
  {
    return;
  }

  Vec2f text_pos = pos;

  // Add vertex attribute data
  for (const char c : text)
  {
    const auto& glyph = glyphs->getGlyph(c);

    const Vec2f pos_rect_min =
      text_pos + Vec2f{glyph.bearing_px.x() * scaling, (glyph.bearing_px.y() - glyph.size_px.y()) * scaling};
    const Vec2f pos_rect_max = pos_rect_min + glyph.size_px.cast<float>() * scaling;

    textured_quad_buffer.push_back(
      {.rect = Bounds2f{pos_rect_min, pos_rect_max},
       .rect_texture = glyph.atlas_bounds,
       .color = color,
       .texture_unit = (*texture_unit_opt)});
    text_pos.x() += glyph.advance_px * scaling;
  }

  rp.submit(make_const_view(textured_quad_buffer));
  textured_quad_buffer.clear();
}

}  // namespace sde::graphics
