// C++ Standard Library
#include <ostream>

// SDE
#include "sde/game/assets.hpp"
#include "sde/game/resources.hpp"
#include "sde/graphics/colors.hpp"
#include "sde/graphics/renderer.hpp"
#include "sde/graphics/sprite.hpp"
#include "sde/graphics/type_set.hpp"
#include "sde/graphics/type_setter.hpp"
#include "sde/logging.hpp"

// RED
#include "red/renderer.hpp"

using namespace sde;

bool Renderer::onInitialize(entt::registry& registry, Resources& resources, Assets& assets, const AppProperties& app)
{
  auto sprite_shader_or_error = assets.graphics.shaders.load("/home/brian/dev/assets/shaders/glsl/simple_sprite.glsl");
  if (!sprite_shader_or_error.has_value())
  {
    SDE_LOG_ERROR("Missing sprite shader");
    return false;
  }

  auto player_text_shader_or_error =
    assets.graphics.shaders.load("/home/brian/dev/assets/shaders/glsl/simple_text.glsl");
  if (!player_text_shader_or_error.has_value())
  {
    SDE_LOG_ERROR("Missing text shader");
    return false;
  }

  auto player_font_or_error = assets.graphics.fonts.load("/home/brian/dev/assets/fonts/white_rabbit.ttf");
  if (!player_font_or_error.has_value())
  {
    SDE_LOG_ERROR("Missing front");
    return false;
  }

  auto player_typeset_or_error = assets.graphics.type_sets.create(
    assets.graphics.textures, *player_font_or_error, graphics::TypeSetOptions{.height_px = 20});
  if (!player_typeset_or_error.has_value())
  {
    SDE_LOG_ERROR("Failed to create player typeset");
    return false;
  }

  player_text_shader_ = player_text_shader_or_error->handle;
  player_text_type_set_ = player_typeset_or_error->handle;
  sprite_shader_ = sprite_shader_or_error->handle;
  scaling_ = 1.0F;
  render_buffer_.reset();
  return true;
}

expected<void, ScriptError>
Renderer::onUpdate(entt::registry& registry, Resources& resources, const Assets& assets, const AppProperties& app)
{
  // Handle screen zoom
  static constexpr float kScaleRate = 500.0;
  const float scroll_sensitivity = std::clamp(scaling_, 1e-4F, 1e-2F);
  if (app.mouse_scroll.y() > 0)
  {
    scaling_ -= scroll_sensitivity * kScaleRate * sde::toSeconds(app.time_delta);
  }
  else if (app.mouse_scroll.y() < 0)
  {
    scaling_ += scroll_sensitivity * kScaleRate * sde::toSeconds(app.time_delta);
  }

  using namespace sde::graphics;

  RenderResources render_resources;
  render_resources.target = RenderTargetHandle::null();
  render_resources.shader = sprite_shader_;
  render_resources.buffer_group = 0;

  RenderUniforms uniforms;
  uniforms.scaling = scaling_;
  uniforms.time = app.time;
  uniforms.time_delta = app.time_delta;

  registry.view<Focused, Position>().each(
    [&](const Position& pos) { uniforms.world_from_camera.block<2, 1>(0, 2) = pos.center; });

  if (auto render_pass_or_error = RenderPass::create(
        render_buffer_, resources.renderer, assets.graphics, uniforms, render_resources, app.viewport_size);
      render_pass_or_error.has_value())
  {
    render_pass_or_error->clear(Black());
    registry.view<Midground, Size, Position, AnimatedSprite>().each(
      [&](const Size& size, const Position& pos, const AnimatedSprite& sprite) {
        const Vec2f min_corner{pos.center - 0.5F * size.extent};
        const Vec2f max_corner{pos.center + 0.5F * size.extent};
        sprite.draw(*render_pass_or_error, app.time, {min_corner, max_corner});
      });

    registry.view<Foreground, Size, Position, AnimatedSprite>().each(
      [&](const Size& size, const Position& pos, const AnimatedSprite& sprite) {
        const Vec2f min_corner{pos.center - 0.5F * size.extent};
        const Vec2f max_corner{pos.center + 0.5F * size.extent};
        sprite.draw(*render_pass_or_error, app.time, {min_corner, max_corner});
      });
  }

  render_resources.shader = player_text_shader_;
  if (auto render_pass_or_error = RenderPass::create(
        render_buffer_, resources.renderer, assets.graphics, uniforms, render_resources, app.viewport_size);
      render_pass_or_error.has_value())
  {
    TypeSetter type_setter{player_text_type_set_};
    registry.view<Info, Size, Position, Dynamics>().each(
      [&](const Info& info, const Size& size, const Position& pos, const Dynamics& state) {
        if ((state.velocity.array() == 0.0F).all())
        {
          const float t = toSeconds(app.time);
          const Vec4f color{
            std::abs(std::cos(t * 3.0F)), std::abs(std::sin(t * 3.0F)), std::abs(std::cos(t * 2.0F)), 1.0F};
          type_setter.draw(
            *render_pass_or_error,
            info.name,
            pos.center + sde::Vec2f{0.0, 0.45F + std::sin(5.0F * t) * 0.05F},
            {0.075F},
            color);
        }
        type_setter.draw(
          *render_pass_or_error,
          sde::format("pos: (%.3f, %.3f)", pos.center.x(), pos.center.y()),
          pos.center + sde::Vec2f{0.0, -0.3F},
          {0.025F},
          Yellow(0.8));
        type_setter.draw(
          *render_pass_or_error,
          sde::format("vel: (%.3f, %.3f)", state.velocity.x(), state.velocity.y()),
          pos.center + sde::Vec2f{0.0, -0.3F - 0.05},
          {0.025F},
          Yellow(0.8));
      });
  }

  return {};
}
