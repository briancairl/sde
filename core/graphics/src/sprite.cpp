// C++ Standard Library
#include <algorithm>
#include <iostream>

// SDE
#include "sde/graphics/renderer.hpp"
#include "sde/graphics/sprite.hpp"

namespace sde::graphics
{

Sprite::Sprite(const TextureHandle& atlas, const Bounds2f& texbounds) : atlas_{atlas}, texbounds_{texbounds} {}


void Sprite::draw(RenderPass& rp, const Bounds2f& rect, const Vec4f& tint) const
{
  const auto& textures = rp.resources().textures;
  if (const auto unit_itr = textures.find(atlas_); unit_itr != textures.end())
  {
    const TexturedQuad texture_quad{
      .rect = rect,
      .rect_texture = texbounds_,
      .color = tint,
      .texture_unit = static_cast<std::size_t>(std::distance(std::begin(textures), unit_itr))};
    rp.submit(make_const_view(&texture_quad, 1UL));
  }
}

}  // namespace sde::graphics
