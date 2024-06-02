/**
 * @copyright 2024-present Brian Cairl
 *
 * @file shapes.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>

// SDE
#include "sde/geometry_types.hpp"

namespace sde::graphics
{

using Point = Vec2f;

struct Line
{
  Point tail = Point::Zero();
  Point head = Point::Zero();
};

std::ostream& operator<<(std::ostream& os, const Line& line);

using Rect = Bounds2f;

std::ostream& operator<<(std::ostream& os, const Rect& rect);

struct Quad
{
  Rect rect = {};
  Vec4f color = Vec4f::Ones();

  const Bounds2f& bounds() const { return rect; }
};

std::ostream& operator<<(std::ostream& os, const Quad& quad);

struct Circle
{
  Point center = {};
  float radius = 1.0F;
  Vec4f color = Vec4f::Ones();

  const Bounds2f bounds() const
  {
    const Point extents{radius, radius};
    return Bounds2f{center - extents, center + extents};
  }
};

std::ostream& operator<<(std::ostream& os, const Circle& circle);

struct TexturedQuad
{
  Rect rect;
  Rect rect_texture;
  Vec4f color = Vec4f::Ones();
  std::size_t texture_unit;

  const Bounds2f& bounds() const { return rect; }
};

std::ostream& operator<<(std::ostream& os, const TexturedQuad& quad);

}  // namespace sde::graphics
