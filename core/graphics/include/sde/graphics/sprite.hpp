/**
 * @copyright 2024-present Brian Cairl
 *
 * @file sprite.hpp
 */
#pragma once

// SDE
#include "sde/geometry_types.hpp"
#include "sde/graphics/renderer_fwd.hpp"
#include "sde/graphics/texture_handle.hpp"
#include "sde/graphics/tile_set_handle.hpp"

namespace sde::graphics
{

class Sprite
{
public:
  Sprite(const TextureHandle& tile_atlas, const Bounds2f& tile_bounds);

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

class AnimatedSprite
{
public:
  using Mode = AnimatedSpriteMode;

  AnimatedSprite(
    const TileSetHandle& frames_handle,
    float frames_per_second,
    AnimatedSpriteMode looped = AnimatedSpriteMode::kLooped);

  void draw(RenderPass& rp, const Bounds2f& rect, const Vec4f& tint = Vec4f::Ones()) const;

  void update(float t);

  void setMode(Mode mode) { mode_ = mode; }

private:
  TileSetHandle frames_handle_;
  float frames_per_second_;
  std::size_t frame_ = 0UL;
  Mode mode_ = Mode::kLooped;
};


}  // namespace sde::graphics
