// C++ Standard Library
#include <iostream>

// SDE
#include "sde/graphics/platform.hpp"
#include "sde/graphics/renderer.hpp"
#include "sde/graphics/shader.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/logging.hpp"

int main(int argc, char** argv)
{
  auto app = sde::graphics::initialize({
    .initial_size = {1000, 500},
  });

  using namespace sde::graphics;

  ShaderCache shader_cache;

  auto shader_or_error = shader_cache.toShader(
    R"Shader(

  layout (location = 0) in vec2 vPosition;
  layout (location = 1) in vec2 vTexCoord;
  layout (location = 2) in vec4 vTintColor;

  out vec2 fTexCoord;
  out vec4 fTintColor;

  void main()
  {
    gl_Position = vec4(vPosition, 0, 1);
    fTexCoord = vTexCoord;
    fTintColor = vTintColor;
  }

  ---

  out vec4 FragColor;

  in vec2 fTexCoord;
  in vec4 vTintColor;

  uniform sampler2D fTextureID;

  void main()
  {
    FragColor = texture(fTextureID, fTexCoord);
  }

)Shader");

  SDE_ASSERT_TRUE(shader_or_error.has_value());

  TextureCache texture_cache;

  auto renderer = Renderer2D::create(shader_cache, *shader_or_error);

  app.spin([&](const auto& window_properties) {
    renderer->submit(Quad{.min = {0.1F, 0.1F}, .min = {0.5F, 0.5F}, .color = {0.1F, 0.1F, 0.1F, 0.1F}});

    renderer->update(texture_cache);
  });
  return 0;
}
