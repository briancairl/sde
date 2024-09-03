/**
 * @copyright 2024-present Brian Cairl
 *
 * @file render_buffer.hpp
 */
#pragma once

// SDE
#include "sde/graphics/shapes.hpp"
#include "sde/vector.hpp"

namespace sde::graphics
{

struct RenderBuffer
{
  sde::vector<Circle> circles;
  sde::vector<Quad> quads;
  sde::vector<TexturedQuad> textured_quads;

  void reset()
  {
    circles.clear();
    quads.clear();
    textured_quads.clear();
  }
};

}  // namespace sde::graphics
