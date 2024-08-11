// C++ Standard Library
#include <optional>
#include <ostream>

// SDE
#include "sde/game/script_impl.hpp"
#include "sde/geometry_utils.hpp"
#include "sde/graphics/colors.hpp"
#include "sde/graphics/render_buffer.hpp"
#include "sde/graphics/renderer.hpp"
#include "sde/graphics/sprite.hpp"
#include "sde/graphics/tile_map.hpp"
#include "sde/graphics/type_set.hpp"
#include "sde/graphics/type_setter.hpp"
#include "sde/logging.hpp"

// ImGui
#include <imgui.h>

// RED
#include "red/components.hpp"
#include "red/renderer.hpp"

using namespace sde;
using namespace sde::game;
using namespace sde::graphics;

namespace sde::serial
{

template <typename Archive> struct load<Archive, VertexBufferOptions> : load<Archive, Resource<VertexBufferOptions>>
{};

template <typename Archive> struct save<Archive, VertexBufferOptions> : save<Archive, Resource<VertexBufferOptions>>
{};

}  // sde::serial

class Renderer final : public ScriptRuntime
{
public:
  Renderer() : ScriptRuntime{"Renderer"} {}

private:
  std::optional<Renderer2D> renderer_;

  float scaling_ = 1.0F;
  Renderer2DOptions renderer_options_;
  RenderBuffer render_buffer_;
  ShaderHandle sprite_shader_;
  FontHandle player_text_font_;
  TypeSetHandle player_text_type_set_;
  ShaderHandle player_text_shader_;
  RenderTargetHandle render_target_;

  bool onLoad(IArchive& ar, SharedAssets& assets) override
  {
    using namespace sde::serial;
    ar >> Field{"scaling", scaling_};
    ar >> Field{"renderer_options", renderer_options_};
    ar >> Field{"render_target", render_target_};
    ar >> Field{"sprite_shader", sprite_shader_};
    ar >> Field{"player_text_font", player_text_font_};
    ar >> Field{"player_text_type_set", player_text_type_set_};
    ar >> Field{"player_text_shader", player_text_shader_};
    return true;
  }

  bool onSave(OArchive& ar, const SharedAssets& assets) const override
  {
    using namespace sde::serial;
    ar << Field{"scaling", scaling_};
    ar << Field{"renderer_options", renderer_options_};
    ar << Field{"render_target", render_target_};
    ar << Field{"sprite_shader", sprite_shader_};
    ar << Field{"player_text_font", player_text_font_};
    ar << Field{"player_text_type_set", player_text_type_set_};
    ar << Field{"player_text_shader", player_text_shader_};
    return true;
  }

  bool onInitialize(SharedAssets& assets, AppState& app_state, const AppProperties& app) override
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

    if (auto renderer_or_error = Renderer2D::create(renderer_options_); renderer_or_error.has_value())
    {
      renderer_.emplace(std::move(renderer_or_error).value());
    }
    else
    {
      SDE_LOG_ERROR("Failed to create renderer");
      return false;
    }
    return true;
  }

  expected<void, ScriptError> onUpdate(SharedAssets& assets, AppState& app_state, const AppProperties& app) override
  {
    // if (auto ok_or_error = onEdit(assets);
    //     !ok_or_error.has_value() and (ok_or_error.error() == ScriptError::kCriticalUpdateFailure))
    // {
    //   return ok_or_error;
    // }

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
          render_buffer_, *renderer_, assets.graphics, uniforms, render_resources, app.viewport_size);
        render_pass_or_error.has_value())
    {
      render_pass_or_error->clear(Black());

      assets.registry.view<TransformQuery>().each(
        [&rp = *render_pass_or_error](auto& query) { query.world_from_viewport = rp.getWorldFromViewportMatrix(); });

      assets.registry.view<Position, TileMap>().each(
        [&](const Position& pos, const TileMap& tile_map) { tile_map.draw(*render_pass_or_error, pos.center); });

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

    render_resources.buffer_group = 1;
    render_resources.shader = sprite_shader_;

    if (auto render_pass_or_error = RenderPass::create(
          render_buffer_, *renderer_, assets.graphics, uniforms, render_resources, app.viewport_size);
        render_pass_or_error.has_value())
    {
      assets.registry.view<Position, TileMap, DebugWireFrame>().each(
        [&](const Position& pos, const TileMap& tile_map, const DebugWireFrame& debug) {
          render_buffer_.quads.push_back(
            {.rect = Rect2f{pos.center, pos.center + tile_map.mapSize()}, .color = debug.color});
        });

      assets.registry.view<Size, Position, DebugWireFrame>().each(
        [&](const Size& size, const Position& pos, const DebugWireFrame& debug) {
          const Vec2f min_corner{pos.center - 0.5F * size.extent};
          const Vec2f max_corner{pos.center + 0.5F * size.extent};
          render_buffer_.quads.push_back({.rect = Rect2f{min_corner, max_corner}, .color = debug.color});
        });
    }

    render_resources.buffer_group = 0;
    render_resources.shader = player_text_shader_;

    if (auto render_pass_or_error = RenderPass::create(
          render_buffer_, *renderer_, assets.graphics, uniforms, render_resources, app.viewport_size);
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

  expected<void, ScriptError> onEdit(SharedAssets& assets)
  {
    if (!assets->contains<ImGuiContext*>())
    {
      return make_unexpected(ScriptError::kNonCriticalUpdateFailure);
    }

    static Renderer2DOptions s__renderer_options{renderer_options_};

    ImGui::SetCurrentContext(assets->get<ImGuiContext*>());
    ImGui::Begin("renderer");
    {
      ImGui::BeginChild("reset", ImVec2{0, 40}, true);
      {
        if (renderer_options_ == s__renderer_options)
        {
          ImGui::Text("renderer up to date");
        }
        else if (ImGui::Button("restart renderer with settings"))
        {
          renderer_options_ = s__renderer_options;
          renderer_.reset();
          if (auto renderer_or_error = Renderer2D::create(renderer_options_); renderer_or_error.has_value())
          {
            renderer_.emplace(std::move(renderer_or_error).value());
          }
          else
          {
            SDE_LOG_ERROR("Failed to reset renderer");
            return make_unexpected(ScriptError::kCriticalUpdateFailure);
          }
        }
      }
      ImGui::EndChild();

      {
        static int n_buffers = s__renderer_options.buffers.size();
        if (ImGui::InputInt("n_buffers", &n_buffers, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue))
        {
          n_buffers = std::clamp(n_buffers, 1, 10);
          s__renderer_options.buffers.resize(n_buffers);
        }
      }

      ImGui::BeginChild("buffers", ImVec2{0, 0}, true);
      {
        for (std::size_t i = 0; i < s__renderer_options.buffers.size(); ++i)
        {
          auto& options = s__renderer_options.buffers[i];
          ImGui::PushID(i);
          ImGui::Text("buffer[%lu]", i);
          {
            int max_triangle_count_per_render_pass = options.max_triangle_count_per_render_pass;
            ImGui::InputInt(
              "max_triangle_count", &max_triangle_count_per_render_pass, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue);
            options.max_triangle_count_per_render_pass = max_triangle_count_per_render_pass;
          }
          {
            bool is_dynamic = options.buffer_mode == VertexBufferMode::kDynamic;
            ImGui::Checkbox("dynamic", &is_dynamic);
            options.buffer_mode = (is_dynamic) ? VertexBufferMode::kDynamic : VertexBufferMode::kStatic;
          }
          ImGui::PopID();
          ImGui::Separator();
        }
      }
      ImGui::EndChild();
    }
    ImGui::End();
    return {};
  }
};

std::unique_ptr<ScriptRuntime> createRenderer() { return std::make_unique<Renderer>(); }
