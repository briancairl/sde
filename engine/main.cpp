// C++ Standard Library
#include <cmath>
#include <iostream>

// SDE
#include "sde/geometry_utils.hpp"
#include "sde/graphics/assets.hpp"
#include "sde/graphics/colors.hpp"
#include "sde/graphics/image.hpp"
#include "sde/graphics/platform.hpp"
#include "sde/graphics/render_target.hpp"
#include "sde/graphics/renderer.hpp"
#include "sde/graphics/shader.hpp"
#include "sde/graphics/shapes.hpp"
#include "sde/graphics/sprite.hpp"
#include "sde/graphics/text.hpp"
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

static const auto* kTextShader = R"TextShader(

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
    fTexCoord = vTexCoord;
    fTintColor = vTintColor;
    fTexUnit = vTexUnit;
  }

  ---
  out vec4 FragColor;

  in float fTexUnit;
  in vec2 fTexCoord;
  in vec4 fTintColor;

  uniform sampler2D[16] uTexture;

  void main()
  {
    int u = int(fTexUnit);
    vec4 s = texture2D(uTexture[u], fTexCoord);
    vec4 c = vec4(sin(3.0 * fTexCoord[0]), cos(3.0 * fTexCoord[1]), fTintColor[2], fTintColor[3]) * s[0];
    FragColor = c;
  }

)TextShader";

int main(int argc, char** argv)
{
  SDE_LOG_INFO("starting...");

  auto app = sde::graphics::Window::initialize({
    .initial_size = {1000, 500},
  });

  using namespace sde::graphics;

  auto image_or_error = Image::load("/home/brian/Pictures/nokron_background.png", {.flags = {.flip_vertically = true}});

  Assets assets;

  auto renderer_or_error = Renderer2D::create();
  SDE_ASSERT_TRUE(renderer_or_error.has_value());

  SDE_ASSERT_TRUE(image_or_error.has_value());

  auto shader1_or_error = assets.shaders.create(kShader1);
  SDE_ASSERT_TRUE(shader1_or_error.has_value());

  auto shader2_or_error = assets.shaders.create(kShader2);
  SDE_ASSERT_TRUE(shader2_or_error.has_value());

  auto text_shader_or_error = assets.shaders.create(kTextShader);
  SDE_ASSERT_TRUE(text_shader_or_error.has_value());

  auto texture_or_error = assets.textures.create(*image_or_error);

  SDE_ASSERT_TRUE(texture_or_error.has_value());

  auto tile_set_or_error =
    assets.tile_sets.create(*texture_or_error, sde::Vec2i{64, 64}, sde::toBounds(sde::Vec2i{8 * 64, 8 * 64}));

  SDE_ASSERT_TRUE(tile_set_or_error.has_value());

  auto draw_texture_or_error = assets.textures.create(sde::Type<std::uint8_t>, TextureShape{{500, 500}}, TextureLayout::kRGB);

  auto font_or_error = assets.fonts.create("/home/brian/Downloads/coffee_fills/font.ttf");
  SDE_ASSERT_TRUE(font_or_error.has_value());

  auto glyphs_or_error = assets.glyph_sets.create(assets.textures, *font_or_error, GlyphSetOptions{.height_px = 100});
  SDE_ASSERT_TRUE(glyphs_or_error.has_value());

  RenderResources default_resources;
  default_resources.shader = shader1_or_error->handle;
  default_resources.textures[0] = texture_or_error->handle;
  default_resources.textures[1] = draw_texture_or_error->handle;
  default_resources.textures[4] = texture_or_error->handle;

  std::vector<Quad> layer_base_quads;
  std::vector<TexturedQuad> layer_base_textured_quads;

  layer_base_quads.push_back({
    .rect = Rect{Point{0.7F, 0.7F}, Point{0.8F, 0.8F}},
    .color = Magenta()
  });

  layer_base_quads.push_back({
    .rect = Rect{Point{0.0F, 0.0F}, Point{0.5F, 0.5F}},
    .color = Red()
  });

  layer_base_textured_quads.push_back({
    .rect = Rect{Point{ 0.8F, 0.8F}, Point{ 1.8F, 1.8F }},
    .rect_texture = Rect{Point{0.0F, 0.0F}, Point{1.0F, 1.0F}},
    .color = Cyan(0.5F),
    .texture_unit = 0
  });

  auto tile_map = TileMap::create(*tile_set_or_error, {8, 8}, {0.0F, 0.0F}, {0.1F, 0.1F});

  for (int y = 0; y < 8; ++y)
  {
    for (int x = 0; x < 8; ++x)
    {
      tile_map[{x, y}] = y * 8 + x;
    }
  }

  RenderResources text_resources;
  text_resources.shader = text_shader_or_error->handle;
  text_resources.textures[0] = glyphs_or_error->value->glyph_atlas;

  RenderResources lighting_resources;
  lighting_resources.shader = shader2_or_error->handle;

  auto texture_target_or_error = RenderTarget::create(draw_texture_or_error->handle, assets.textures);
  SDE_ASSERT_TRUE(texture_target_or_error.has_value());

  auto window_target_or_error = RenderTarget::create(app.handle());
  SDE_ASSERT_TRUE(window_target_or_error.has_value());

  std::vector<Circle> layer_lighting_circles;

  RenderAttributes attributes;

  Sprite sprite{texture_or_error->handle, sde::Bounds2f{sde::Vec2f{0, 0}, sde::Vec2f{0.8, 0.5}}};

  AnimatedSprite animated_sprite{tile_set_or_error->handle, 15.0F, AnimatedSprite::Mode::kLooped};
  AnimatedSprite animated_sprite_once{tile_set_or_error->handle, 5.0F, AnimatedSprite::Mode::kOneShot};

  TypeSetter typesetter{glyphs_or_error->handle};

  app.spin([&](const auto& window)
  {
    const auto time = std::chrono::duration_cast<std::chrono::duration<float>>(window.time).count();
    const auto time_delta = std::chrono::duration_cast<std::chrono::duration<float>>(window.time_delta).count();

    static constexpr float kMoveRate = 0.5;

    if (window.keys.isDown(KeyCode::kA))
    {
      attributes.world_from_camera(0, 2) -= time_delta * kMoveRate;
    }

    if (window.keys.isDown(KeyCode::kD))
    {
      attributes.world_from_camera(0, 2) += time_delta * kMoveRate;
    }

    if (window.keys.isDown(KeyCode::kS))
    {
      attributes.world_from_camera(1, 2) -= time_delta * kMoveRate;
    }

    if (window.keys.isDown(KeyCode::kW))
    {
      attributes.world_from_camera(1, 2) += time_delta * kMoveRate;
    }

    static constexpr float kScaleRate = 1.5;
    const float scroll_sensitivity = std::clamp(attributes.scaling, 1e-4F, 1e-2F);
    if (window.mouse_scroll.y() > 0)
    {
      attributes.scaling -= scroll_sensitivity * kScaleRate * time;
    }
    else if (window.mouse_scroll.y() < 0)
    {
      attributes.scaling += scroll_sensitivity * kScaleRate * time;
    }
    attributes.scaling = std::max(attributes.scaling, 1e-3F);

    attributes.time = time;
    attributes.time_delta = time_delta;

    animated_sprite.update(time);
    animated_sprite_once.update(time);


    layer_lighting_circles.push_back({
      .center = sde::transform(attributes.getWorldFromViewportMatrix(*window_target_or_error), window.getMousePositionViewport(window_target_or_error->getLastSize())), 
      .radius = 1.5F,
      .color = Yellow()
    });

    texture_target_or_error->refresh(Black());
    if (auto render_pass_or_error = RenderPass::create(*texture_target_or_error, *renderer_or_error, assets, attributes, lighting_resources); render_pass_or_error.has_value())
    {
      render_pass_or_error->submit(sde::make_const_view(layer_base_quads));
      render_pass_or_error->submit(sde::make_const_view(layer_lighting_circles));
    }

    window_target_or_error->refresh(Black());
    if (auto render_pass_or_error = RenderPass::create(*window_target_or_error, *renderer_or_error, assets, attributes, default_resources); render_pass_or_error.has_value())
    {
      render_pass_or_error->submit(sde::make_const_view(layer_base_quads));
      render_pass_or_error->submit(sde::make_const_view(layer_base_textured_quads));
      tile_map.draw(*render_pass_or_error);
      sprite.draw(*render_pass_or_error, sde::Bounds2f{sde::Vec2f{0, 0}, sde::Vec2f{0.5, 0.5}}, Red(0.4));
      animated_sprite.draw(*render_pass_or_error, sde::Bounds2f{sde::Vec2f{0.8, 0.8}, sde::Vec2f{1.4, 1.4}}, Blue(0.9));
      animated_sprite_once.draw(*render_pass_or_error, sde::Bounds2f{sde::Vec2f{-0.8, -0.8}, sde::Vec2f{-0.4, -0.4}}, Green(0.9));

    }

    if (auto render_pass_or_error = RenderPass::create(*window_target_or_error, *renderer_or_error, assets, attributes, text_resources); render_pass_or_error.has_value())
    {
      typesetter.draw(*render_pass_or_error, "Poop is always funny :]", sde::Vec2f{0.0, 0.0}, 0.5F);
      typesetter.draw(*render_pass_or_error, "Poop is always funny :]", sde::Vec2f{0.5, 0.0}, 0.5F, Blue(0.3));
    }

    if (auto render_pass_or_error = RenderPass::create(*window_target_or_error, *renderer_or_error, assets, attributes, lighting_resources); render_pass_or_error.has_value())
    {
      render_pass_or_error->submit(sde::make_const_view(layer_lighting_circles));
    }

    // SDE_LOG_DEBUG_FMT("%f : %f", attributes.time, attributes.time_delta);

    layer_lighting_circles.clear();

    return WindowDirective::kContinue;
  });

  SDE_LOG_INFO("done.");
  return 0;
}

// clang-format on
