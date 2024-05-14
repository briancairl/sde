// C++ Standard Library
#include <iostream>

// SDE
#include "sde/graphics/platform.hpp"
#include "sde/graphics/renderer.hpp"

int main(int argc, char** argv)
{
  auto app = sde::graphics::initialize({
    .initial_size = {1000, 500},
  });

  auto renderer = sde::graphics::Renderer2D::create();

  app.spin([&renderer](const auto& window_properties) { renderer->update(); });
  return 0;
}
