// C++ Standard Library
#include <algorithm>
#include <ostream>

// SDE
#include "sde/graphics/assets.hpp"
#include "sde/graphics/render_buffer.hpp"
#include "sde/graphics/renderer.hpp"
#include "sde/graphics/shapes.hpp"
#include "sde/graphics/sprite.hpp"

namespace sde::graphics
{

Sprite::Sprite(const Options& options) : options_{options} {}

void Sprite::draw(RenderPass& rp, const Bounds2f& rect) const
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

  if (options_.frame_index >= frames->tile_bounds.size())
  {
    return;
  }

  if (const auto texture_unit_opt = rp.assign(frames->tile_atlas); texture_unit_opt.has_value())
  {
    rp->textured_quads.push_back(
      {.rect = rect,
       .rect_texture = frames->tile_bounds[options_.frame_index],
       .color = options_.tint_color,
       .texture_unit = (*texture_unit_opt)});
  }
}

bool operator==(const SpriteOptions& lhs, const SpriteOptions& rhs)
{
  return lhs.tint_color == rhs.tint_color && lhs.frames == rhs.frames && lhs.frame_index == rhs.frame_index;
}

bool operator==(const Sprite& lhs, const Sprite& rhs) { return lhs.options() == rhs.options(); }


AnimatedSprite::AnimatedSprite(const Options& options) : options_{options} {}

void AnimatedSprite::draw(RenderPass& rp, TimeOffset t, const Bounds2f& rect) const
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

  const auto frame = static_cast<std::size_t>((t + options_.time_offset) * options_.frames_per_second);
  const auto frame_idx_saturated = (options_.mode == Mode::kLooped) ? (frame % frames->tile_bounds.size())
                                                                    : std::min(frame, frames->tile_bounds.size() - 1UL);

  if (const auto texture_unit_opt = rp.assign(frames->tile_atlas); texture_unit_opt.has_value())
  {
    rp->textured_quads.push_back(
      {.rect = rect,
       .rect_texture = frames->tile_bounds[frame_idx_saturated],
       .color = options_.tint_color,
       .texture_unit = (*texture_unit_opt)});
  }
}


bool operator==(const AnimatedSpriteOptions& lhs, const AnimatedSpriteOptions& rhs)
{
  return lhs.tint_color == rhs.tint_color && lhs.frames == rhs.frames && lhs.time_offset == rhs.time_offset &&
    lhs.frames_per_second == rhs.frames_per_second && lhs.mode == rhs.mode;
}

bool operator==(const AnimatedSprite& lhs, const AnimatedSprite& rhs) { return lhs.options() == rhs.options(); }

}  // namespace sde::graphics
