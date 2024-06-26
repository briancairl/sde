// C++ Standard Library
#include <algorithm>

// SDE
#include "sde/graphics/assets.hpp"
#include "sde/graphics/render_buffer.hpp"
#include "sde/graphics/renderer.hpp"
#include "sde/graphics/shapes.hpp"
#include "sde/graphics/sprite.hpp"

namespace sde::graphics
{

Sprite::Sprite(const TextureHandle& tile_atlas, const Bounds2f& tile_bounds) :
    tile_atlas_{tile_atlas}, tile_bounds_{tile_bounds}
{}

void Sprite::draw(RenderPass& rp, const Bounds2f& rect, const Vec4f& tint) const
{
  if (!rp.visible(rect))
  {
    return;
  }

  if (const auto texture_unit_opt = rp.assign(tile_atlas_); texture_unit_opt.has_value())
  {
    rp->textured_quads.push_back(
      {.rect = rect, .rect_texture = tile_bounds_, .color = tint, .texture_unit = (*texture_unit_opt)});
  }
}

AnimatedSprite::AnimatedSprite(const Options&& options) : options_{options} {}

void AnimatedSprite::draw(RenderPass& rp, TimeOffset t, const Bounds2f& rect, const Vec4f& tint) const
{
  if (!rp.visible(rect))
  {
    return;
  }

  const auto* frames = rp.assets().tile_sets(options_.frames);
  if (frames == nullptr)
  {
    return;
  }

  const auto frame = static_cast<std::size_t>(t * options_.frames_per_second);
  const auto frame_idx_saturated = (options_.mode == Mode::kLooped) ? (frame % frames->tile_bounds.size())
                                                                    : std::min(frame, frames->tile_bounds.size() - 1UL);

  if (const auto texture_unit_opt = rp.assign(frames->tile_atlas); texture_unit_opt.has_value())
  {
    rp->textured_quads.push_back(
      {.rect = rect,
       .rect_texture = frames->tile_bounds[frame_idx_saturated],
       .color = tint,
       .texture_unit = (*texture_unit_opt)});
  }
}

}  // namespace sde::graphics
