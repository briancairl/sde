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

namespace sde::graphics
{

class Sprite
{
public:
  Sprite(const TextureHandle& atlas, const Bounds2f& texbounds);

  void draw(RenderPass& rp, const Bounds2f& rect, const Vec4f& tint = Vec4f::Ones()) const;

  TextureHandle atlas() const { return atlas_; }

  const Bounds2f& texbounds() const { return texbounds_; }

private:
  TextureHandle atlas_;
  Bounds2f texbounds_;
};

}  // namespace sde::graphics
