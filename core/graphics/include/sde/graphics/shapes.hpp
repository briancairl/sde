/**
 * @copyright 2024-present Brian Cairl
 *
 * @file shapes.hpp
 */
#pragma once

// SDE
#include "sde/geometry_types.hpp"

namespace sde::graphics
{

struct Rect
{
  Vec2f min = Vec2f::Zero();
  Vec2f max = Vec2f::Zero();
};

struct Quad
{
  Rect rect = {};
  Vec4f color = Vec4f::Ones();
};

struct TexturedQuad
{
  Rect rect;
  Rect rect_texture;
  Vec4f color = Vec4f::Ones();
  std::size_t texture_unit;
};

struct Circle
{
  Vec2f center = {};
  float radius = 1.0F;
  Vec4f color = Vec4f::Ones();
};

}  // namespace sde::graphics
