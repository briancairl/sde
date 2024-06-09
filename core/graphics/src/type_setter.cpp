// C++ Standard Library
#include <algorithm>
#include <iostream>

// SDE
#include "sde/graphics/assets.hpp"
#include "sde/graphics/renderer.hpp"
#include "sde/graphics/shapes.hpp"
#include "sde/graphics/type_setter.hpp"

namespace sde::graphics
{
TypeSetter::TypeSetter(const TypeSetHandle& glyphs) : type_set_handle_{glyphs} {}

void TypeSetter::draw(
  RenderPass& rp,
  std::string_view text,
  const Vec2f& pos,
  const TextOptions& options,
  const Vec4f& color) const
{
  const auto* glyphs = rp.assets().type_sets(type_set_handle_);
  if (glyphs == nullptr)
  {
    return;
  }

  const Bounds2i text_bounds_px = glyphs->getTextBounds(text);
  const float text_width_px = text_bounds_px.max().x() - text_bounds_px.min().x();
  const float text_height_px = text_bounds_px.max().y() - text_bounds_px.min().y();
  const float text_scaling = options.height / text_height_px;
  const Bounds2f text_bounds{
    text_bounds_px.min().cast<float>() * text_scaling, text_bounds_px.max().cast<float>() * text_scaling};

  Vec2f text_pos = pos;

  if (options.justification_x == TextJusificationH::kRight)
  {
    text_pos.x() -= text_width_px * text_scaling;
  }
  else if (options.justification_x == TextJusificationH::kCenter)
  {
    text_pos.x() -= 0.5F * text_width_px * text_scaling;
  }

  if (options.justification_y == TextJusificationV::kBelow)
  {
    text_pos.y() -= text_height_px * text_scaling;
  }
  else if (options.justification_y == TextJusificationV::kCenter)
  {
    text_pos.y() -= 0.5F * text_height_px * text_scaling;
  }

  const Bounds2f text_aabb{text_pos + text_bounds.min().cast<float>(), text_pos + text_bounds.max().cast<float>()};

  if (!rp.getViewportInWorldBounds().intersects(text_aabb))
  {
    return;
  }

  const auto texture_unit_opt = rp.assign(glyphs->glyph_atlas);
  if (!texture_unit_opt.has_value())
  {
    return;
  }

  static std::vector<TexturedQuad> s__textured_quad_buffer;

  // Add vertex attribute data
  for (const char c : text)
  {
    const auto& glyph = glyphs->getGlyph(c);

    const Vec2f pos_rect_min =
      text_pos + Vec2f{glyph.bearing_px.x() * text_scaling, (glyph.bearing_px.y() - glyph.size_px.y()) * text_scaling};
    const Vec2f pos_rect_max = pos_rect_min + glyph.size_px.cast<float>() * text_scaling;

    s__textured_quad_buffer.push_back(
      {.rect = Bounds2f{pos_rect_min, pos_rect_max},
       .rect_texture = glyph.atlas_bounds,
       .color = color,
       .texture_unit = (*texture_unit_opt)});
    text_pos.x() += glyph.advance_px * text_scaling;
  }

  rp.submit(make_const_view(s__textured_quad_buffer));
  s__textured_quad_buffer.clear();
}

}  // namespace sde::graphics
