// C++ Standard Library
#include <algorithm>
#include <iostream>

// SDE
#include "sde/graphics/renderer.hpp"
#include "sde/graphics/sprite.hpp"

namespace sde::graphics
{

Sprite::Sprite(const TextureHandle& tile_atlas, const Bounds2f& tile_bounds) :
    tile_atlas_{tile_atlas}, tile_bounds_{tile_bounds}
{}


void Sprite::draw(RenderPass& rp, const Bounds2f& rect, const Vec4f& tint) const
{
  const auto& textures = rp.resources().textures;
  if (const auto texture_unit_opt = textures(tile_atlas_); texture_unit_opt.has_value())
  {
    const TexturedQuad texture_quad{
      .rect = rect, .rect_texture = tile_bounds_, .color = tint, .texture_unit = (*texture_unit_opt)};
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
  const auto frame_idx_saturated = (mode_ == Mode::kLooped) ? (frame_ % frames->tile_bounds.size())
                                                            : std::min(frame_, frames->tile_bounds.size() - 1UL);

  const auto& textures = rp.resources().textures;
  if (const auto texture_unit_opt = textures(frames->tile_atlas); texture_unit_opt.has_value())
  {
    const TexturedQuad texture_quad{
      .rect = rect,
      .rect_texture = frames->tile_bounds[frame_idx_saturated],
      .color = tint,
      .texture_unit = (*texture_unit_opt)};
    rp.submit(make_const_view(&texture_quad, 1UL));
  }
}

}  // namespace sde::graphics
