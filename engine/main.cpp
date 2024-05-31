// C++ Standard Library
#include <cmath>
#include <iostream>

// SDE
#include "sde/geometry_utils.hpp"
#include "sde/graphics/image.hpp"
#include "sde/graphics/platform.hpp"
#include "sde/graphics/render_target.hpp"
#include "sde/graphics/renderer.hpp"
#include "sde/graphics/shader.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/graphics/tile_map.hpp"
#include "sde/graphics/tile_set.hpp"
#include "sde/logging.hpp"
#include "sde/view.hpp"

// clang-format off

static const auto* kShader1 = R"Shader1(

  layout (location = 0) in vec2 vPosition;
  layout (location = 1) in vec2 vTexCoord;
  layout (location = 2) in float vTexUnit;
  layout (location = 3) in vec4 vTintColor;

  out vec2 fTexCoord;
  out vec4 fTintColor;
  out float fTexUnit;

  uniform mat3 uCameraTransform;

  void main()
  {
    gl_Position = vec4(uCameraTransform * vec3(vPosition, 1), 1);
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

  uniform mat3 uCameraTransform;
  uniform float uTime;
  uniform float uTimeDelta;

  void main()
  {
    gl_Position = vec4(uCameraTransform * vec3(vPosition, 1), 1);

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

  auto app = sde::graphics::Window::initialize({
    .initial_size = {1000, 500},
  });

  using namespace sde::graphics;

  auto image_or_error = Image::load("/home/brian/Pictures/nokron_background.png", {.flags = {.flip_vertically = true}});

  SDE_ASSERT_TRUE(image_or_error.has_value());

  ShaderCache shader_cache;

  auto shader1_or_error = shader_cache.create(kShader1);
  SDE_ASSERT_TRUE(shader1_or_error.has_value());

  auto shader2_or_error = shader_cache.create(kShader2);
  SDE_ASSERT_TRUE(shader2_or_error.has_value());

  TextureCache texture_cache;

  auto texture_or_error = texture_cache.upload(*image_or_error);

  SDE_ASSERT_TRUE(texture_or_error.has_value());

  auto tile_set_or_error =
    TileSet::slice(*texture_or_error, texture_cache, {64, 64}, sde::toBounds(sde::Vec2i{8 * 64, 8 * 64}));

  SDE_ASSERT_TRUE(tile_set_or_error.has_value());

  auto draw_texture_or_error = texture_cache.create<std::uint8_t>(TextureShape{{500, 500}}, TextureLayout::kRGB);

  auto renderer_or_error = Renderer2D::create(&shader_cache, &texture_cache);
  SDE_ASSERT_TRUE(renderer_or_error.has_value());

  RenderResources default_resources;
  default_resources.shader = *shader1_or_error;
  default_resources.textures[0] = (*texture_or_error);
  default_resources.textures[1] = (*draw_texture_or_error);
  default_resources.textures[4] = (*texture_or_error);

  std::vector<Quad> layer_base_quads;
  std::vector<TileMap> layer_base_tile_maps;
  std::vector<TexturedQuad> layer_base_textured_quads;

  layer_base_quads.push_back({
    .rect = {
      .min = {0.7F, 0.7F},
      .max = {0.8F, 0.8F}
    },
    .color = {1.0F, 0.0F, 1.0F, 1.0F}
  });

  layer_base_quads.push_back({
    .rect = {
      .min = {0.1F, 0.1F},
      .max = {0.5F, 0.5F}
    },
    .color = {1.0F, 1.0F, 1.0F, 1.0F}
  });


  layer_base_textured_quads.push_back({
    .rect = {
      .min = {0.1F, 0.1F},
      .max = {0.3F, 0.3F}
    },
    .rect_texture = {
      .min = {0.0F, 0.0F},
      .max = {1.0F, 1.0F}
    },
    .color = {1.0F, 1.0F, 1.0F, 1.0F},
    .texture_unit = 1
  });

  layer_base_textured_quads.push_back({
    .rect = {
      .min = { 0.8F, 0.8F},
      .max = { 1.8F, 1.8F}
    },
    .rect_texture = {
      .min = {0.0F, 0.0F},
      .max = {1.0F, 1.0F}
    },
    .color = {1.0F, 0.0F, 1.0F, 0.9F},
    .texture_unit = 1
  });

  layer_base_tile_maps.push_back(
    []
    {
      TileMap tile_map;
      tile_map.position = {0.7F, -0.8F};
      tile_map.tile_size = {0.25F, 0.25F};
      tile_map.texture_unit = 0;
      tile_map.tiles <<
        0, 1, 2, 3, 4, 5, 6,24,
        8, 9,10,11,12,13,14,15,
        0, 1, 2, 3, 4, 5, 6, 7,
        0, 1, 2, 3, 4, 5, 6, 7,
        0, 1, 2, 3, 4, 5, 6, 7,
        0, 1, 2, 3, 4, 5, 6, 7,
        0, 1, 2, 3, 4, 5, 6, 7,
        0, 1, 2, 3, 4, 5, 6, 7;
      return tile_map;
    }()
  );

  layer_base_tile_maps.push_back(
    [position = getNextRightPosition(layer_base_tile_maps.back())]
    {
      TileMap tile_map;
      tile_map.position = position;
      tile_map.tile_size = {0.25F, 0.25F};
      tile_map.texture_unit = 0;
      tile_map.color[1] = 0.0;
      tile_map.tiles <<
        0, 1, 2, 3, 4, 5, 6,24,
        8, 9,10,11,12,13,14,15,
        0, 1, 2, 3, 4, 5, 6, 7,
        0, 1, 2, 3, 4, 5, 6, 7,
        0, 1, 2, 3, 4, 5, 6, 7,
        0, 1, 2, 3, 4, 5, 6, 7,
        0, 1, 2, 3, 4,55, 6, 7,
        0, 1, 2, 3, 4, 5, 6, 7;
      return tile_map;
    }()
  );

  RenderResources lighting_resources;
  lighting_resources.shader = *shader2_or_error;

  auto texture_target_or_error = RenderTarget::create(*draw_texture_or_error, texture_cache);
  SDE_ASSERT_TRUE(texture_target_or_error.has_value());

  auto window_target_or_error = RenderTarget::create(app.handle());
  SDE_ASSERT_TRUE(window_target_or_error.has_value());

  std::vector<Circle> layer_lighting_circles;

  RenderAttributes attributes;

  app.spin([&](const auto& window_properties)
  {
    const auto time = std::chrono::duration_cast<std::chrono::duration<float>>(window_properties.time).count();
    const auto time_delta = std::chrono::duration_cast<std::chrono::duration<float>>(window_properties.time_delta).count();

    static constexpr float kMoveRate = 0.5;

    if (window_properties.keys.isDown(KeyCode::kA))
    {
      attributes.world_from_camera(0, 2) -= time_delta * kMoveRate;
    }

    if (window_properties.keys.isDown(KeyCode::kD))
    {
      attributes.world_from_camera(0, 2) += time_delta * kMoveRate;
    }

    if (window_properties.keys.isDown(KeyCode::kS))
    {
      attributes.world_from_camera(1, 2) -= time_delta * kMoveRate;
    }

    if (window_properties.keys.isDown(KeyCode::kW))
    {
      attributes.world_from_camera(1, 2) += time_delta * kMoveRate;
    }

    static constexpr float kScaleRate = 1.5;
    const float scroll_sensitivity = std::clamp(attributes.scaling, 1e-4F, 1e-2F);
    if (window_properties.mouse_scroll.y() > 0)
    {
      attributes.scaling -= scroll_sensitivity * kScaleRate * time;
    }
    else if (window_properties.mouse_scroll.y() < 0)
    {
      attributes.scaling += scroll_sensitivity * kScaleRate * time;
    }
    attributes.scaling = std::max(attributes.scaling, 1e-3F);

    attributes.time = time;
    attributes.time_delta = time_delta;


    if (auto render_pass_or_error = RenderPass::create(*texture_target_or_error, *renderer_or_error, attributes, lighting_resources); render_pass_or_error.has_value())
    {
      //render_pass_or_error->submit(sde::make_const_view(layer_base_quads));
      // layer_lighting_circles.push_back({
      //   .center = sde::transform(attributes.getWorldFromViewportMatrix(*window_target_or_error), window_properties.getMousePositionViewport(window_target_or_error->getLastSize())), 
      //   .radius = 1.5F,
      //   .color = {1.0F, 1.0F, 0.5F, 1.0F}
      // });

      // // layer_lighting_circles.push_back({
      // //   .center = {-0.4F, -0.4F},
      // //   .radius = 0.4F,
      // //   .color = {1.0F, 1.0F, 1.0F, 0.5F}
      // // });

      // render_pass_or_error->submit(sde::make_const_view(layer_lighting_circles));
      // layer_lighting_circles.clear();
    }

    if (auto render_pass_or_error = RenderPass::create(*window_target_or_error, *renderer_or_error, attributes, default_resources); render_pass_or_error.has_value())
    {
      render_pass_or_error->submit(sde::make_const_view(layer_base_quads));
      //render_pass_or_error->submit(sde::make_const_view(layer_base_textured_quads));
      render_pass_or_error->submit(sde::make_const_view(layer_base_tile_maps), *tile_set_or_error);
    }

    SDE_LOG_DEBUG_FMT("%f : %f", attributes.time, attributes.time_delta);

    return WindowDirective::kContinue;
  });

  SDE_LOG_INFO("done.");
  return 0;
}

// clang-format on
