// C++ Standard Library
#include <cmath>
#include <ostream>

// EnTT
#include <entt/entt.hpp>

// SDE
#include "sde/app.hpp"
#include "sde/audio/assets.hpp"
#include "sde/audio/mixer.hpp"
// #include "sde/audio/sound.hpp"
// #include "sde/audio/sound_data.hpp"
#include "sde/game/assets.hpp"
#include "sde/game/resources.hpp"
// #include "sde/game/script.hpp"
// #include "sde/geometry_utils.hpp"
// #include "sde/graphics/assets.hpp"
#include "sde/graphics/colors.hpp"
#include "sde/graphics/image.hpp"
#include "sde/graphics/render_buffer.hpp"
#include "sde/graphics/render_target.hpp"
#include "sde/graphics/renderer.hpp"
// #include "sde/graphics/shader.hpp"
// #include "sde/graphics/shapes.hpp"
#include "sde/graphics/sprite.hpp"
// #include "sde/graphics/texture.hpp"
// #include "sde/graphics/tile_map.hpp"
// #include "sde/graphics/tile_set.hpp"
#include "sde/graphics/type_setter.hpp"
// #include "sde/graphics/window.hpp"
#include "sde/logging.hpp"
// #include "sde/resource_cache_from_disk.hpp"
// #include "sde/view.hpp"

// RED
#include "red/player_character.hpp"

// clang-format off

using namespace sde;


int main(int argc, char** argv)
{
  using namespace sde::audio;
  using namespace sde::graphics;

  SDE_LOG_INFO("starting...");

  auto icon_or_error = Image::load("/home/brian/dev/assets/icons/red.png");
  SDE_ASSERT_TRUE(icon_or_error.has_value());

  auto app_or_error = App::create(
    {
      .initial_size = {1000, 500},
      .icon = std::addressof(*icon_or_error),
      //.cursor = std::addressof(*icon_or_error),  // <-- this works, but need a better image
    });
  SDE_ASSERT_TRUE(app_or_error.has_value());


  game::Assets assets;
  game::Resources resources{game::Resources::create().value()};

  auto audio_mixer_or_error = audio::Mixer::create();
  SDE_ASSERT_TRUE(audio_mixer_or_error.has_value());

  auto background_track_1_or_error = assets.sounds_from_disk.create("/home/brian/dev/assets/sounds/tracks/OldTempleLoop.wav");
  SDE_ASSERT_TRUE(background_track_1_or_error.has_value());

  auto background_track_2_or_error = assets.sounds_from_disk.create("/home/brian/dev/assets/sounds/tracks/forest.wav");
  SDE_ASSERT_TRUE(background_track_2_or_error.has_value());

  if (auto listener_or_err = ListenerTarget::create(*audio_mixer_or_error, 0UL); listener_or_err.has_value())
  {
    listener_or_err->set(*background_track_1_or_error, TrackOptions{.gain=0.3F, .looped=true});
    listener_or_err->set(*background_track_2_or_error, TrackOptions{.gain=1.0F, .looped=true});
  }

  auto player_font_or_error = assets.fonts_from_disk.create("/home/brian/dev/assets/fonts/white_rabbit.ttf");
  SDE_ASSERT_TRUE(player_font_or_error.has_value());
  auto player_typeset_or_error = assets.graphics.type_sets.create(assets.graphics.textures, *player_font_or_error, TypeSetOptions{.height_px = 20});
  SDE_ASSERT_TRUE(player_typeset_or_error.has_value());

  auto text_shader_or_error = assets.shaders_from_disk.create("/home/brian/dev/assets/shaders/glsl/simple_text.glsl");
  SDE_ASSERT_TRUE(text_shader_or_error.has_value());

  TypeSetter type_setter{*player_typeset_or_error};

  RenderResources text_rendering_resources;
  text_rendering_resources.shader = (*text_shader_or_error);
  text_rendering_resources.buffer_group = 1;


  auto sprite_shader_or_error = assets.shaders_from_disk.create("/home/brian/dev/assets/shaders/glsl/simple_sprite.glsl");
  SDE_ASSERT_TRUE(sprite_shader_or_error.has_value());

  auto window_target_or_error = RenderTarget::create(app_or_error->window());
  SDE_ASSERT_TRUE(window_target_or_error.has_value());


  RenderResources sprite_rendering_resources;
  sprite_rendering_resources.shader = (*sprite_shader_or_error);
  sprite_rendering_resources.buffer_group = 0;

  RenderBuffer render_buffer;
  RenderAttributes screen_attributes;
  RenderAttributes world_attributes;

  entt::registry reg;
  PlayerCharacter character_script;

  app_or_error->spin([&](const auto& window)
  {
    // Handle screen zoom
    static constexpr float kScaleRate = 500.0;
    const float scroll_sensitivity = std::clamp(world_attributes.scaling, 1e-4F, 1e-2F);
    if (window.mouse_scroll.y() > 0)
    {
      world_attributes.scaling -= scroll_sensitivity * kScaleRate * sde::toSeconds(window.time_delta);
    }
    else if (window.mouse_scroll.y() < 0)
    {
      world_attributes.scaling += scroll_sensitivity * kScaleRate * sde::toSeconds(window.time_delta);
    }
    world_attributes.scaling = std::max(world_attributes.scaling, 1e-3F);
    world_attributes.time = window.time;
    world_attributes.time_delta = window.time_delta;


    character_script.update(reg, resources, assets, window);

    reg.view<State>().each([dt = toSeconds(window.time_delta)](State& state) { state.position += state.velocity * dt; });

    window_target_or_error->refresh(Black());
    if (auto render_pass_or_error = RenderPass::create(*window_target_or_error, render_buffer, resources.renderer, assets.graphics, world_attributes, sprite_rendering_resources); render_pass_or_error.has_value())
    {
      reg.view<Size, State, graphics::AnimatedSprite>().each(
        [&](const Size& size, const State& state, const graphics::AnimatedSprite& sprite)
        {
          const Vec2f min_corner{state.position - 0.5F * size.extent};
          const Vec2f max_corner{state.position + 0.5F * size.extent};
          sprite.draw(*render_pass_or_error, window.time, {min_corner, max_corner});
        });
    }

    if (auto render_pass_or_error = RenderPass::create(*window_target_or_error, render_buffer, resources.renderer, assets.graphics, world_attributes, text_rendering_resources); render_pass_or_error.has_value())
    {
      reg.view<Info, Size, State>().each(
        [&](const Info& info, const Size& size, const State& state)
        {
          if ((state.velocity.array() == 0.0F).all())
          {
            const float t = toSeconds(window.time);
            const Vec4f color{std::abs(std::cos(t * 3.0F)), std::abs(std::sin(t * 3.0F)), std::abs(std::cos(t * 2.0F)), 1.0F};
            type_setter.draw(*render_pass_or_error, info.name, state.position + sde::Vec2f{0.0, 0.45F + std::sin(5.0F * t) * 0.05F}, {0.075F}, color);
          }
          type_setter.draw(*render_pass_or_error, sde::format("pos: (%.3f, %.3f)", state.position.x(),  state.position.y()),  state.position + sde::Vec2f{0.0, -0.3F}, {0.025F}, Yellow(0.8));
          type_setter.draw(*render_pass_or_error, sde::format("vel: (%.3f, %.3f)", state.velocity.x(), state.velocity.y()), state.position + sde::Vec2f{0.0, -0.3F - 0.05}, {0.025F}, Yellow(0.8));
        });
    }

    return AppDirective::kContinue;
  });

  SDE_LOG_INFO("done.");
  return 0;
}

// clang-format on
