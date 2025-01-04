#define SDE_SCRIPT_TYPE_NAME "renderer"

// C++ Standard Library
#include <optional>
#include <ostream>

// SDE
#include "sde/game/native_script_runtime.hpp"
#include "sde/graphics/colors.hpp"
#include "sde/graphics/render_buffer.hpp"
#include "sde/graphics/renderer.hpp"
#include "sde/graphics/sprite.hpp"
#include "sde/graphics/tile_map.hpp"
#include "sde/graphics/type_setter.hpp"
#include "sde/graphics/window.hpp"

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

// ImGui
#include <imgui.h>

// RED
#include "red/components/common.hpp"


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


struct renderer_state : native_script_data
{
  std::optional<Renderer2D> renderer;
  Renderer2DOptions renderer_options;

  float scaling = 1.0F;

  RenderBuffer render_buffer;
  ShaderHandle sprite_shader;
  FontHandle player_text_font;
  TypeSetHandle player_text_type_set;
  ShaderHandle player_text_shader;
  RenderTargetHandle render_target;
};

template <typename ArchiveT> bool serialize(renderer_state* self, ArchiveT& ar)
{
  using namespace sde::serial;
  ar& Field{"scaling", self->scaling};
  ar& Field{"renderer_options", self->renderer_options};
  ar& Field{"render_target", self->render_target};
  ar& Field{"sprite_shader", self->sprite_shader};
  ar& Field{"player_text_font", self->player_text_font};
  ar& Field{"player_text_type_set", self->player_text_type_set};
  ar& Field{"player_text_shader", self->player_text_shader};
  return true;
}

bool initialize(renderer_state* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  if (!graphics::Window::try_backend_initialization())
  {
    SDE_LOG_ERROR() << "Backend not initialized";
    return false;
  }

  if (auto renderer_or_error = Renderer2D::create(self->renderer_options); renderer_or_error.has_value())
  {
    self->renderer.emplace(std::move(renderer_or_error).value());
  }
  else
  {
    SDE_LOG_ERROR() << "Failed to create renderer: " << SDE_OSNV(renderer_or_error.error());
    return false;
  }

  if (!resources.assign(self->render_target))
  {
    SDE_LOG_ERROR() << "Missing sprite shader";
    return false;
  }

  if (!resources.assign(self->sprite_shader, "/home/brian/dev/assets/shaders/glsl/simple_sprite.glsl"_path))
  {
    SDE_LOG_ERROR() << "Missing sprite shader";
    return false;
  }

  if (!resources.assign(self->player_text_shader, "/home/brian/dev/assets/shaders/glsl/simple_text.glsl"_path))
  {
    SDE_LOG_ERROR() << "Missing text shader";
    return false;
  }

  if (!resources.assign(self->player_text_font, "/home/brian/dev/assets/fonts/white_rabbit.ttf"_path))
  {
    SDE_LOG_ERROR() << "Missing font";
    return false;
  }

  if (!resources.assign(self->player_text_type_set, self->player_text_font, TypeSetOptions{.height_px = 100}))
  {
    SDE_LOG_ERROR() << "Failed to create player typeset";
    return false;
  }

  self->render_buffer.reset();
  return true;
}

bool shutdown(renderer_state* self, sde::game::GameResources& resources, const sde::AppProperties& app) { return true; }

void ui(renderer_state* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  if (ImGui::GetCurrentContext() == nullptr)
  {
    return;
  }

  ImGui::Begin("renderer");
  ImGui::SliderFloat("scaling", &self->scaling, 0.0F, 1e0F);
  ImGui::End();

  // Handle screen zoom
  if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
  {
    static constexpr float kScaleRate = 500.0;
    const float scroll_sensitivity = std::clamp(self->scaling, 1e-4F, 1e-2F);
    if (app.mouse_scroll.y() > 0)
    {
      self->scaling = std::max(1e-3F, self->scaling - scroll_sensitivity * kScaleRate * sde::toSeconds(app.time_delta));
    }
    else if (app.mouse_scroll.y() < 0)
    {
      self->scaling = std::min(1e+3F, self->scaling + scroll_sensitivity * kScaleRate * sde::toSeconds(app.time_delta));
    }
  }
}

bool update(renderer_state* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  ui(self, resources, app);

  using namespace sde::graphics;

  if (auto render_target = resources(self->render_target); render_target)
  {
    render_target->reset(Black());
  }
  else
  {
    return false;
  }

  RenderResources render_resources;
  render_resources.target = self->render_target;
  render_resources.shader = self->sprite_shader;
  render_resources.buffer = 0;

  RenderUniforms uniforms;
  uniforms.scaling = self->scaling;
  uniforms.time = app.time;
  uniforms.time_delta = app.time_delta;

  auto& registry = resources.get<Registry>();

  registry.view<Focused, Position>().each(
    [&](const Position& pos) { uniforms.world_from_camera.block<2, 1>(0, 2) = pos.center; });

  if (auto render_pass_or_error = RenderPass::create(
        self->render_buffer, *self->renderer, resources.all(), uniforms, render_resources, app.viewport_size);
      render_pass_or_error.has_value())
  {
    registry.view<TransformQuery>().each(
      [&rp = *render_pass_or_error](auto& query) { query.world_from_viewport = rp.getWorldFromViewportMatrix(); });

    registry.view<Position, TileMap>().each([&](const Position& pos, const TileMap& tile_map) {
      tile_map.draw(*render_pass_or_error, resources.all(), pos.center);
    });

    registry.view<Midground, Size, Position, AnimatedSprite>().each(
      [&](const Size& size, const Position& pos, const AnimatedSprite& sprite) {
        const Vec2f min_corner{pos.center - 0.5F * size.extent};
        const Vec2f max_corner{pos.center + 0.5F * size.extent};
        sprite.draw(*render_pass_or_error, resources.all(), app.time, {min_corner, max_corner});
      });

    registry.view<Foreground, Size, Position, AnimatedSprite>().each(
      [&](const Size& size, const Position& pos, const AnimatedSprite& sprite) {
        const Vec2f min_corner{pos.center - 0.5F * size.extent};
        const Vec2f max_corner{pos.center + 0.5F * size.extent};
        sprite.draw(*render_pass_or_error, resources.all(), app.time, {min_corner, max_corner});
      });
  }
  else
  {
    return false;
  }

  render_resources.buffer = 1;
  render_resources.shader = self->sprite_shader;

  if (auto render_pass_or_error = RenderPass::create(
        self->render_buffer, *self->renderer, resources.all(), uniforms, render_resources, app.viewport_size);
      render_pass_or_error.has_value())
  {
    registry.view<Position, TileMap, DebugWireFrame>().each(
      [&](const Position& pos, const TileMap& tile_map, const DebugWireFrame& debug) {
        self->render_buffer.quads.push_back(
          {.rect = Rect2f{pos.center, pos.center + tile_map.mapSize()}, .color = debug.color});
      });

    registry.view<Size, Position, DebugWireFrame>().each(
      [&](const Size& size, const Position& pos, const DebugWireFrame& debug) {
        const Vec2f min_corner{pos.center - 0.5F * size.extent};
        const Vec2f max_corner{pos.center + 0.5F * size.extent};
        self->render_buffer.quads.push_back({.rect = Rect2f{min_corner, max_corner}, .color = debug.color});
      });
  }
  else
  {
    return false;
  }


  render_resources.buffer = 0;
  render_resources.shader = self->player_text_shader;

  if (auto render_pass_or_error = RenderPass::create(
        self->render_buffer, *self->renderer, resources.all(), uniforms, render_resources, app.viewport_size);
      render_pass_or_error.has_value())
  {
    TypeSetter type_setter{self->player_text_type_set};
    registry.view<Info, Size, Position, Dynamics>().each(
      [&](const Info& info, const Size& size, const Position& pos, const Dynamics& state) {
        if (state.velocity.x() == 0.0F and state.velocity.y() == 0.0F)
        {
          const float t = toSeconds(app.time);
          const Vec4f color{
            std::abs(std::cos(t * 3.0F)), std::abs(std::sin(t * 3.0F)), std::abs(std::cos(t * 2.0F)), 1.0F};
          type_setter.draw(
            *render_pass_or_error,
            resources.all(),
            info.name,
            pos.center + sde::Vec2f{0.0, 0.45F + std::sin(5.0F * t) * 0.05F},
            {0.075F},
            color);
        }
        type_setter.draw(
          *render_pass_or_error,
          resources.all(),
          sde::format("pos: (%.3f, %.3f)", pos.center.x(), pos.center.y()),
          pos.center + sde::Vec2f{0.0, -0.3F},
          {0.025F},
          Yellow(0.8));
        type_setter.draw(
          *render_pass_or_error,
          resources.all(),
          sde::format("vel: (%.3f, %.3f)", state.velocity.x(), state.velocity.y()),
          pos.center + sde::Vec2f{0.0, -0.3F - 0.05},
          {0.025F},
          Yellow(0.8));
      });
  }
  else
  {
    return false;
  }


  return true;
}


SDE_NATIVE_SCRIPT__REGISTER_AUTO(renderer_state);