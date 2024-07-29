/**
 * @copyright 2024-present Brian Cairl
 *
 * @file shapes.hpp
 */
#pragma once


// SDE
#include "sde/geometry.hpp"
#include "sde/resource.hpp"

namespace sde::graphics
{

struct Line : Resource<Line>
{
  Vec2f tail = Vec2f::Zero();
  Vec2f head = Vec2f::Zero();

  auto field_list() { return FieldList(Field{"tail", tail}, Field{"head", head}); }
};

struct Quad : Resource<Quad>
{
  Bounds2f rect = {};
  Vec4f color = Vec4f::Ones();

  const Bounds2f& bounds() const { return rect; }

  auto field_list() { return FieldList(Field{"rect", rect}, Field{"color", color}); }
};

struct Circle : Resource<Circle>
{
  Vec2f center = {};
  float radius = 1.0F;
  Vec4f color = Vec4f::Ones();

  const Bounds2f bounds() const
  {
    const Vec2f extents{radius, radius};
    return Bounds2f{center - extents, center + extents};
  }

  auto field_list() { return FieldList(Field{"center", center}, Field{"radius", radius}, Field{"color", color}); }
};

struct TexturedQuad : Resource<TexturedQuad>
{
  Bounds2f rect;
  Bounds2f rect_texture;
  Vec4f color = Vec4f::Ones();
  std::size_t texture_unit;

  const Bounds2f& bounds() const { return rect; }

  auto field_list()
  {
    return FieldList(
      Field{"rect", rect},
      Field{"rect_texture", rect_texture},
      Field{"color", color},
      Field{"texture_unit", texture_unit});
  }
};

}  // namespace sde::graphics
