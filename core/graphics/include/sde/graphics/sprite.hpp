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
#include "sde/graphics/tile_set.hpp"
#include "sde/graphics/tile_set_handle.hpp"

namespace sde::graphics
{

class Sprite
{
public:
  Sprite(const TextureHandle& atlas_handle, const Bounds2f& texbounds);

  void draw(RenderPass& rp, const Bounds2f& rect, const Vec4f& tint = Vec4f::Ones()) const;

  TextureHandle atlas() const { return atlas_handle_; }

  const Bounds2f& texbounds() const { return texbounds_; }

private:
  TextureHandle atlas_handle_;
  Bounds2f texbounds_;
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

  void draw(RenderPass& rp, const TileSetCache& tileset_cache, const Bounds2f& rect, const Vec4f& tint = Vec4f::Ones())
    const;

  void update(float t);

private:
  TileSetHandle frames_handle_;
  float frames_per_second_;
  std::size_t frame_ = 0UL;
  Mode mode_ = Mode::kLooped;
};


}  // namespace sde::graphics