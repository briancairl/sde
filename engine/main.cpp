// C++ Standard Library
#include <ostream>

// SDE
#include "sde/graphics/image.hpp"
#include "sde/graphics/platform.hpp"
#include "sde/graphics/renderer.hpp"
#include "sde/graphics/shader.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/logging.hpp"

static const auto* kShader1 = R"Shader1(

  layout (location = 0) in vec2 vPosition;
  layout (location = 1) in vec2 vTexCoord;
  layout (location = 2) in float vTexUnit;
  layout (location = 3) in vec4 vTintColor;

  out vec2 fTexCoord;
  out vec4 fTintColor;
  out float fTexUnit;

  uniform mat3 uTransform;

  void main()
  {
    gl_Position = vec4(uTransform * vec3(vPosition, 0), 1);
    fTexUnit = vTexUnit;
    fTexCoord = vTexCoord;
    fTintColor = vTintColor;
  }

  ---

  out vec4 FragColor;

  in float fTexUnit;
  in vec2 fTexCoord;
  in vec4 fTintColor;

  uniform sampler2D[16] uTexture;

  void main()
  {
    int texture_unit = int(fTexUnit);
    bool texture_enabled = bool(texture_unit >= 0);
    vec4 texture_color_sampled = texture2D(uTexture[texture_unit], fTexCoord);
    FragColor = (float(!texture_enabled) * fTintColor) + float(texture_enabled) * (fTintColor * texture_color_sampled);
  }
)Shader1";

static const auto* kShader2 = R"Shader2(

  layout (location = 0) in vec2 vPosition;
  layout (location = 1) in vec2 vTexCoord;
  layout (location = 2) in float vTexUnit;
  layout (location = 3) in vec4 vTintColor;

  out vec2 fTexCoord;
  out vec4 fTintColor;

  uniform mat3 uTransform;
  uniform float uTime;
  uniform float uTimeDelta;

  void main()
  {
    gl_Position = vec4(uTransform * vec3(vPosition, 0), 1);

    float w = (2000.0 * (uTimeDelta + 0.01) + 200.0) * uTime;

    fTexCoord = vTexCoord * (1 + 5e-2 * sin(w));
    fTintColor = vTintColor;
  }

  ---

  out vec4 FragColor;

  in vec2 fTexCoord;
  in vec4 fTintColor;

  void main()
  {
    float fade = pow(1.0 - sqrt(pow(fTexCoord[0], 2) + pow(fTexCoord[1], 2)), 2);
    FragColor = fade * fTintColor;
  }
)Shader2";

int main(int argc, char** argv)
{
  SDE_LOG_INFO("starting...");

  auto app = sde::graphics::initialize({
    .initial_size = {1000, 500},
  });

  using namespace sde::graphics;

  auto image_or_error = Image::load("/home/brian/Pictures/nokron_background.png", {.flags = {.flip_vertically = true}});

  SDE_ASSERT_TRUE(image_or_error.has_value());

  ShaderCache shader_cache;

  auto shader1_or_error = shader_cache.toShader(kShader1);
  SDE_ASSERT_TRUE(shader1_or_error.has_value());

  auto shader2_or_error = shader_cache.toShader(kShader2);
  SDE_ASSERT_TRUE(shader2_or_error.has_value());

  TextureCache texture_cache;

  auto texture_or_error = texture_cache.toTexture(*image_or_error);

  SDE_ASSERT_TRUE(texture_or_error.has_value());

  Renderer2D renderer;

  Layer layer_base;
  layer_base.resources.shader = *shader1_or_error;
  layer_base.resources.textures[0] = (*texture_or_error);
  layer_base.resources.textures[4] = (*texture_or_error);

  Layer layer_lighting;
  layer_lighting.resources.shader = *shader2_or_error;

  // clang-format off
  app.spin([&](const auto& window_properties) {

    layer_base.settings.time = std::chrono::duration_cast<std::chrono::duration<float>>(window_properties.time).count();
    layer_base.settings.time_delta = std::chrono::duration_cast<std::chrono::duration<float>>(window_properties.time_delta).count();
    layer_base.settings.scaling = 1.0F;
    layer_base.settings.setAspectRatio(window_properties.size);

    layer_lighting.settings = layer_base.settings;

    layer_base.quads.push_back({
      .rect = {
        .min = {0.7F, 0.7F},
        .max = {0.8F, 0.8F}
      },
      .color = {1.0F, 0.0F, 1.0F, 1.0F}
    });

    layer_base.quads.push_back({
      .rect = {
        .min = {0.1F, 0.1F},
        .max = {0.5F, 0.5F}
      },
      .color = {1.0F, 1.0F, 1.0F, 1.0F}
    });

    layer_base.textured_quads.push_back({
      .rect = {
        .min = {0.1F, 0.1F},
        .max = {0.3F, 0.3F}
      },
      .rect_texture = {
        .min = {0.0F, 0.0F},
        .max = {1.0F, 1.0F}
      },
      .color = {1.0F, 1.0F, 1.0F, 1.0F},
      .texture_unit = 0});

    layer_base.textured_quads.push_back({
      .rect = {
        .min = {-0.1F, -0.1F},
        .max = {-0.3F, -0.3F}
      },
      .rect_texture = {
        .min = {0.0F, 0.0F},
        .max = {1.0F, 1.0F}
      },
      .color = {1.0F, 0.0F, 1.0F, 0.1F},
      .texture_unit = 4
    });

    layer_lighting.circles.push_back({
      .center = {0.4F, 0.4F}, 
      .radius = 1.5F,
      .color = {1.0F, 1.0F, 0.5F, 1.0F}
    });

    layer_lighting.circles.push_back({
      .center = {-0.4F, -0.4F},
      .radius = 0.4F,
      .color = {1.0F, 1.0F, 1.0F, 0.5F}
    });

    renderer.submit(shader_cache, texture_cache, layer_base);
    renderer.submit(shader_cache, texture_cache, layer_lighting);

    SDE_LOG_DEBUG_FMT("%f : %f", layer_base.settings.time, layer_base.settings.time_delta);

    return WindowDirective::kContinue;
  });
  // clang-format on

  SDE_LOG_INFO("done.");
  return 0;
}
