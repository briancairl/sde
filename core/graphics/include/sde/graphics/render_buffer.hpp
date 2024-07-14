/**
 * @copyright 2024-present Brian Cairl
 *
 * @file render_buffer.hpp
 */
#pragma once

// C++ Standard Library
#include <vector>

// SDE
#include "sde/graphics/shapes.hpp"

namespace sde::graphics
{

struct RenderBuffer
{
  std::vector<Circle> circles;
  std::vector<Quad> quads;
  std::vector<TexturedQuad> textured_quads;

  void reset()
  {
    circles.clear();
    quads.clear();
    textured_quads.clear();
  }
};

}  // namespace sde::graphics
