// C++ Standard Library
#include <ostream>

// SDE
#include "sde/game/script_impl.hpp"
#include "sde/graphics/colors.hpp"
#include "sde/graphics/render_buffer.hpp"
#include "sde/graphics/renderer.hpp"
#include "sde/graphics/sprite.hpp"
#include "sde/graphics/type_set.hpp"
#include "sde/graphics/type_setter.hpp"
#include "sde/logging.hpp"

// RED
#include "red/components.hpp"
#include "red/renderer.hpp"

using namespace sde;
using namespace sde::game;
using namespace sde::graphics;

class Renderer final : public ScriptRuntime
{
public:
  Renderer() : ScriptRuntime{"Renderer"} {}

private:
  float scaling_ = 1.0F;
  RenderBuffer render_buffer_;
  ShaderHandle sprite_shader_;
  FontHandle player_text_font_;
  TypeSetHandle player_text_type_set_;
  ShaderHandle player_text_shader_;
  RenderTargetHandle render_target_;

  bool onLoad(IArchive& ar, SharedAssets& assets) override
  {
    using namespace sde::serial;
    ar >> named{"scaling", scaling_};
    ar >> named{"render_target", render_target_};
    ar >> named{"sprite_shader", sprite_shader_};
    ar >> named{"player_text_font", player_text_font_};
    ar >> named{"player_text_type_set", player_text_type_set_};
    ar >> named{"player_text_shader", player_text_shader_};
    return true;
  }

  bool onSave(OArchive& ar, SharedAssets& assets) override
  {
    using namespace sde::serial;
    ar << named{"scaling", scaling_};
    ar << named{"render_target", render_target_};
    ar << named{"sprite_shader", sprite_shader_};
    ar << named{"player_text_font", player_text_font_};
    ar << named{"player_text_type_set", player_text_type_set_};
    ar << named{"player_text_shader", player_text_shader_};
    return true;
  }

  bool onInitialize(Systems& systems, SharedAssets& assets, AppState& app_state, const AppProperties& app) override
  {
    if (!assets.assign(render_target_))
    {
      SDE_LOG_ERROR("Missing sprite shader");
      return false;
    }

    if (!assets.assign(sprite_shader_, "/home/brian/dev/assets/shaders/glsl/simple_sprite.glsl"_path))
    {
      SDE_LOG_ERROR("Missing sprite shader");
      return false;
    }

    if (!assets.assign(player_text_shader_, "/home/brian/dev/assets/shaders/glsl/simple_text.glsl"_path))
    {
      SDE_LOG_ERROR("Missing text shader");
      return false;
    }

    if (!assets.assign(player_text_font_, "/home/brian/dev/assets/fonts/white_rabbit.ttf"_path))
    {
      SDE_LOG_ERROR("Missing font");
      return false;
    }

    if (!assets.assign(player_text_type_set_, player_text_font_, TypeSetOptions{.height_px = 100}))
    {
      SDE_LOG_ERROR("Failed to create player typeset");
      return false;
    }

    render_buffer_.reset();
    return true;
  }

  expected<void, ScriptError>
  onUpdate(Systems& systems, SharedAssets& assets, AppState& app_state, const AppProperties& app) override
  {
    using namespace sde::graphics;

    RenderResources render_resources;
    render_resources.target = render_target_;
    render_resources.shader = sprite_shader_;
    render_resources.buffer_group = 0;

    RenderUniforms uniforms;
    uniforms.scaling = scaling_;
    uniforms.time = app.time;
    uniforms.time_delta = app.time_delta;

    // Handle screen zoom
    if (app_state.enabled)
    {
      static constexpr float kScaleRate = 500.0;
      const float scroll_sensitivity = std::clamp(scaling_, 1e-4F, 1e-2F);
      if (app.mouse_scroll.y() > 0)
      {
        scaling_ = std::max(1e-3F, scaling_ - scroll_sensitivity * kScaleRate * sde::toSeconds(app.time_delta));
      }
      else if (app.mouse_scroll.y() < 0)
      {
        scaling_ = std::min(1e+3F, scaling_ + scroll_sensitivity * kScaleRate * sde::toSeconds(app.time_delta));
      }
    }

    assets.registry.view<Focused, Position>().each(
      [&](const Position& pos) { uniforms.world_from_camera.block<2, 1>(0, 2) = pos.center; });

    if (auto render_pass_or_error = RenderPass::create(
          render_buffer_, systems.renderer, assets.graphics, uniforms, render_resources, app.viewport_size);
        render_pass_or_error.has_value())
    {
      render_pass_or_error->clear(Black());
      assets.registry.view<Midground, Size, Position, AnimatedSprite>().each(
        [&](const Size& size, const Position& pos, const AnimatedSprite& sprite) {
          const Vec2f min_corner{pos.center - 0.5F * size.extent};
          const Vec2f max_corner{pos.center + 0.5F * size.extent};
          sprite.draw(*render_pass_or_error, app.time, {min_corner, max_corner});
        });

      assets.registry.view<Foreground, Size, Position, AnimatedSprite>().each(
        [&](const Size& size, const Position& pos, const AnimatedSprite& sprite) {
          const Vec2f min_corner{pos.center - 0.5F * size.extent};
          const Vec2f max_corner{pos.center + 0.5F * size.extent};
          sprite.draw(*render_pass_or_error, app.time, {min_corner, max_corner});
        });
    }

    render_resources.shader = player_text_shader_;
    if (auto render_pass_or_error = RenderPass::create(
          render_buffer_, systems.renderer, assets.graphics, uniforms, render_resources, app.viewport_size);
        render_pass_or_error.has_value())
    {
      TypeSetter type_setter{player_text_type_set_};
      assets.registry.view<Info, Size, Position, Dynamics>().each(
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
};

std::unique_ptr<sde::game::ScriptRuntime> createRenderer() { return std::make_unique<Renderer>(); }
