// C++ Standard Library
#include <iostream>

// SDE
#include "sde/graphics/image.hpp"
#include "sde/graphics/platform.hpp"
#include "sde/graphics/renderer.hpp"
#include "sde/graphics/shader.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/logging.hpp"

int main(int argc, char** argv)
{
  SDE_LOG_INFO("starting...");

  auto app = sde::graphics::initialize({
    .initial_size = {1000, 500},
  });

  using namespace sde::graphics;

  auto image_or_error = Image::load("/home/brian/Pictures/nokron_background.png");

  SDE_ASSERT_TRUE(image_or_error.has_value());

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
  in vec4 fTintColor;

  uniform sampler2D fTextureID;

  void main()
  {
    vec4 color = texture2D(fTextureID, fTexCoord);
    float use = float((color.x + color.y + color.z + color.w) < 1e-2);
    FragColor = vec4(fTintColor.x * color.x, fTintColor.y * color.y, fTintColor.z * color.z, fTintColor.w * color.w) * use + (1-use) * fTintColor;
  }

)Shader");

  SDE_ASSERT_TRUE(shader_or_error.has_value());

  TextureCache texture_cache;

  auto texture_or_error = texture_cache.toTexture(*image_or_error);

  SDE_ASSERT_TRUE(texture_or_error.has_value());

  auto renderer = Renderer2D::create();

  renderer->set(*shader_or_error);

  app.spin([&](const auto& window_properties) {
    renderer->submit(Quad{.rect = {.min = {0.7F, 0.7F}, .max = {0.8F, 0.8F}}, .color = {1.0F, 0.0F, 1.0F, 1.0F}});
    renderer->submit(Quad{.rect = {.min = {0.1F, 0.1F}, .max = {0.5F, 0.5F}}, .color = {1.0F, 1.0F, 1.0F, 1.0F}});
    renderer->submit(TexturedQuad{
      .rect = {.min = {0.1F, 0.1F}, .max = {0.5F, 0.5F}},
      .texrect = {.min = {0.1F, 0.1F}, .max = {0.5F, 0.5F}},
      .color = {1.0F, 1.0F, 1.0F, 1.0F},
      .texture = *texture_or_error});
    renderer->update(shader_cache, texture_cache);
  });

  SDE_LOG_INFO("done.");
  return 0;
}
