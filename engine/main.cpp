#include <iostream>

#include "sde/graphics/platform.hpp"
#include "sde/graphics/renderer.hpp"

int main(int argc, char** argv)
{
  auto app = sde::graphics::initialize({
    .initial_width = 1000,
    .initial_height = 500,
  });

  app.spin([](const auto& window_properties) {

  });
  return 0;
}
