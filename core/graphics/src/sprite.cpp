// C++ Standard Library
#include <algorithm>
#include <iostream>

// SDE
#include "sde/graphics/renderer.hpp"
#include "sde/graphics/sprite.hpp"

namespace sde::graphics
{

Sprite::Sprite(const TextureHandle& atlas_handle, const Bounds2f& texbounds) :
    atlas_handle_{atlas_handle}, texbounds_{texbounds}
{}


void Sprite::draw(RenderPass& rp, const Bounds2f& rect, const Vec4f& tint) const
{
  const auto& textures = rp.resources().textures;
  if (const auto unit_itr = textures.find(atlas_handle_); unit_itr != textures.end())
  {
    const TexturedQuad texture_quad{
      .rect = rect,
      .rect_texture = texbounds_,
      .color = tint,
      .texture_unit = static_cast<std::size_t>(std::distance(std::begin(textures), unit_itr))};
    rp.submit(make_const_view(&texture_quad, 1UL));
  }
}

AnimatedSprite::AnimatedSprite(const TileSetHandle& frames_handle, float frames_per_second, Mode mode) :
    frames_handle_{frames_handle}, frames_per_second_{frames_per_second}, frame_{0}, mode_{mode}
{}

void AnimatedSprite::update(float t) { frame_ = static_cast<std::size_t>(t * frames_per_second_); }

void AnimatedSprite::draw(RenderPass& rp, const TileSetCache& tileset_cache, const Bounds2f& rect, const Vec4f& tint)
  const
{
  const auto* frames = tileset_cache.get_if(frames_handle_);
  const auto frame_idx_saturated = (mode_ == Mode::kLooped) ? (frame_ % frames->atlas_bounds.size())
                                                            : std::min(frame_, frames->atlas_bounds.size() - 1UL);

  const auto& textures = rp.resources().textures;
  if (const auto unit_itr = textures.find(frames->atlas); unit_itr != textures.end())
  {
    const TexturedQuad texture_quad{
      .rect = rect,
      .rect_texture = frames->atlas_bounds[frame_idx_saturated],
      .color = tint,
      .texture_unit = static_cast<std::size_t>(std::distance(std::begin(textures), unit_itr))};
    rp.submit(make_const_view(&texture_quad, 1UL));
  }
}

}  // namespace sde::graphics
