/**
 * @copyright 2024-present Brian Cairl
 *
 * @file app_properties.hpp
 */
#pragma once

// SDE
#include "sde/geometry.hpp"
#include "sde/graphics/window_fwd.hpp"
#include "sde/keyboard.hpp"
#include "sde/time.hpp"

namespace sde
{

struct AppProperties
{
  graphics::NativeWindowHandle window{nullptr};

  TimeOffset time = TimeOffset::zero();
  TimeOffset time_delta = TimeOffset::zero();

  Vec2i viewport_size = {640, 480};
  Vec2d mouse_position_px = {0.0, 0.0};
  Vec2d mouse_scroll = {0.0, 0.0};

  KeyStates keys;

  Vec2f getMousePositionViewport(Vec2i viewport_size) const
  {
    return {
      static_cast<float>(2.0 * mouse_position_px.x() / static_cast<double>(viewport_size.x()) - 1.0),
      static_cast<float>(1.0 - 2.0 * mouse_position_px.y() / static_cast<double>(viewport_size.y()))};
  }
};

}  // namespace sde
