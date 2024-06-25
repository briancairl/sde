/**
 * @copyright 2024-present Brian Cairl
 *
 * @file sprite.hpp
 */
#pragma once

// SDE
#include "sde/geometry_types.hpp"
#include "sde/graphics/render_buffer_fwd.hpp"
#include "sde/graphics/renderer_fwd.hpp"
#include "sde/graphics/texture_handle.hpp"
#include "sde/graphics/tile_set_handle.hpp"
#include "sde/time.hpp"

namespace sde::graphics
{

class Sprite
{
public:
  Sprite(const TextureHandle& tile_atlas, const Bounds2f& tile_bounds);

  void draw(RenderBuffer& rb, RenderPass& rp, const Bounds2f& rect, const Vec4f& tint = Vec4f::Ones()) const;

  void draw(RenderPass& rp, const Bounds2f& rect, const Vec4f& tint = Vec4f::Ones()) const;

  TextureHandle atlas() const { return tile_atlas_; }

  const Bounds2f& texbounds() const { return tile_bounds_; }

private:
  TextureHandle tile_atlas_;
  Bounds2f tile_bounds_;
};

enum class AnimatedSpriteMode
{
  kLooped,
  kOneShot
};

struct AnimatedSpriteOptions
{
  TileSetHandle frames;
  Rate frames_per_second = Hertz(5.0);
  AnimatedSpriteMode mode = AnimatedSpriteMode::kOneShot;
};

class AnimatedSprite
{
public:
  using Mode = AnimatedSpriteMode;
  using Options = AnimatedSpriteOptions;

  AnimatedSprite() = default;
  explicit AnimatedSprite(const Options&& options);

  void
  draw(RenderBuffer& rb, RenderPass& rp, TimeOffset t, const Bounds2f& rect, const Vec4f& tint = Vec4f::Ones()) const;

  void draw(RenderPass& rp, TimeOffset t, const Bounds2f& rect, const Vec4f& tint = Vec4f::Ones()) const;

  void setFrames(TileSetHandle frames) { options_.frames = frames; }

  void setFrameRate(Rate rate) { options_.frames_per_second = rate; }

  void setMode(Mode mode) { options_.mode = mode; }

private:
  Options options_;
};


}  // namespace sde::graphics
