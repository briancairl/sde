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

struct Line
{
  Vec2f tail = Vec2f::Zero();
  Vec2f head = Vec2f::Zero();
};

std::ostream& operator<<(std::ostream& os, const Line& line);

struct Rect
{
  Vec2f min = Vec2f::Zero();
  Vec2f max = Vec2f::Zero();
};

std::ostream& operator<<(std::ostream& os, const Rect& rect);

struct Quad
{
  Rect rect = {};
  Vec4f color = Vec4f::Ones();
};

std::ostream& operator<<(std::ostream& os, const Quad& quad);

struct Circle
{
  Vec2f center = {};
  float radius = 1.0F;
  Vec4f color = Vec4f::Ones();
};

std::ostream& operator<<(std::ostream& os, const Circle& circle);

struct TexturedQuad
{
  Rect rect;
  Rect rect_texture;
  Vec4f color = Vec4f::Ones();
  std::size_t texture_unit;
};

std::ostream& operator<<(std::ostream& os, const TexturedQuad& quad);

}  // namespace sde::graphics
