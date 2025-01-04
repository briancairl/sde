// C++ Standard Library
#include <chrono>
#include <csignal>
#include <ostream>
#include <thread>

// SDE
#include "sde/graphics/colors.hpp"
#include "sde/graphics/debug.hpp"
#include "sde/graphics/image.hpp"
#include "sde/graphics/render_buffer.hpp"
#include "sde/graphics/render_target.hpp"
#include "sde/graphics/renderer.hpp"
#include "sde/graphics/shader.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/graphics/window.hpp"
#include "sde/logging.hpp"

using namespace sde;

bool running = true;

void sigint_handler(int signum)
{
  SDE_LOG_INFO() << "exiting: " << signum;
  running = false;
}

int main(int argc, char** argv)
{
  std::signal(SIGINT, sigint_handler);

  auto window_or_error = graphics::Window::create({.title = "playground", .initial_size = {640, 480}});
  SDE_ASSERT_OK(window_or_error);

  graphics::enable_native_debug_logs();
  graphics::enable_native_error_logs();

  graphics::ImageCache images;
  graphics::ShaderCache shaders;
  graphics::TextureCache textures;
  graphics::RenderTargetCache render_targets;
  auto deps = ResourceDependencies{images, shaders, textures, render_targets};

  auto icon_or_error = images.create(deps, "/home/brian/dev/assets/icons/red.png"_path);
  SDE_ASSERT_OK(icon_or_error);
  window_or_error->setWindowIcon(icon_or_error->value->ref());

  auto cursor_or_error = images.create(deps, "/home/brian/dev/assets/icons/sword.png"_path);
  SDE_ASSERT_OK(cursor_or_error);
  window_or_error->setCursorIcon(cursor_or_error->value->ref());

  auto sprite_shader_or_error = shaders.create(deps, "/home/brian/dev/assets/shaders/glsl/simple_sprite.glsl"_path);
  SDE_ASSERT_OK(sprite_shader_or_error);

  auto texture_or_error = textures.create(deps, cursor_or_error->handle);
  SDE_ASSERT_OK(texture_or_error);

  auto render_target_or_error = render_targets.create(deps);
  SDE_ASSERT_OK(render_target_or_error);

  auto renderer_or_error = graphics::Renderer2D::create(
    {.buffers = {graphics::VertexBufferOptions{
       .max_triangle_count_per_render_pass = 1000UL, .draw_mode = graphics::VertexDrawMode::kFilled}}});
  SDE_ASSERT_OK(renderer_or_error);

  graphics::RenderResources render_resources;
  render_resources.target = render_target_or_error->handle;
  render_resources.shader = sprite_shader_or_error->handle;
  render_resources.buffer = 0;

  graphics::RenderUniforms render_uniforms;
  render_uniforms.scaling = 1.0F;
  render_uniforms.world_from_camera = Mat3f::Identity();
  render_uniforms.time = TimeOffset::zero();
  render_uniforms.time_delta = TimeOffset::zero();

  graphics::RenderBuffer render_buffer;

  while (window_or_error->poll() and running)
  {
    render_target_or_error->value->reset(graphics::Black());
    if (auto render_pass_or_error = graphics::RenderPass::create(
          render_buffer, *renderer_or_error, deps, render_uniforms, render_resources, window_or_error->size());
        render_pass_or_error.has_value())
    {
      render_buffer.quads.push_back({.rect = {{-1.0f, -1.0f}, {+0.0f, +0.0f}}, .color = Vec4f{0.5F, 0.6F, 0.7F, 0.9F}});

      render_buffer.quads.push_back({.rect = {{-0.0f, -0.0f}, {+1.0f, +1.0f}}, .color = Vec4f{0.9F, 0.7F, 0.5F, 0.9F}});

      render_buffer.textured_quads.push_back(
        {.rect = {{-1.0f, +1.0f}, {-0.0f, -0.0f}},
         .rect_texture = {{-0.0f, +0.0f}, {+1.0f, +1.0f}},
         .color = Vec4f{0.9F, 0.7F, 0.5F, 0.9F},
         .texture_unit = render_pass_or_error->assign(texture_or_error->handle).value()});

      render_buffer.circles.push_back(
        {.center = {0.5f, -0.5f}, .radius = 0.5F, .color = Vec4f{0.9F, 0.9F, 0.4F, 0.9F}});
    }
    else
    {
      SDE_LOG_ERROR() << "rendering failed";
      return 1;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds{static_cast<std::size_t>(1000.0F / 60.0F)});
  }

  return 0;
}
