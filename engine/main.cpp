// C++ Standard Library
#include <cmath>
#include <ostream>

// EnTT
#include <entt/entt.hpp>

// SDE
#include "sde/app.hpp"
#include "sde/audio/assets.hpp"
#include "sde/audio/mixer.hpp"
#include "sde/audio/sound.hpp"
#include "sde/audio/sound_data.hpp"
#include "sde/game/assets.hpp"
#include "sde/game/script.hpp"
#include "sde/geometry_utils.hpp"
#include "sde/graphics/assets.hpp"
#include "sde/graphics/colors.hpp"
#include "sde/graphics/image.hpp"
#include "sde/graphics/render_target.hpp"
#include "sde/graphics/renderer.hpp"
#include "sde/graphics/shader.hpp"
#include "sde/graphics/shapes.hpp"
#include "sde/graphics/sprite.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/graphics/tile_map.hpp"
#include "sde/graphics/tile_set.hpp"
#include "sde/graphics/type_setter.hpp"
#include "sde/graphics/window.hpp"
#include "sde/logging.hpp"
#include "sde/resource_cache_from_disk.hpp"
#include "sde/view.hpp"

// clang-format off

using namespace sde;

struct Info
{
  std::string name;
};

struct Size
{
  Vec2f extent;
};

struct State
{
  Vec2f position;
  Vec2f velocity;
  Vec2f looking;
};

struct Direction
{
  Vec2f looking;
};

class Character : public game::Script<Character>
{
  friend script;
private:
  static constexpr std::size_t kFront{0};
  static constexpr std::size_t kBack{1};
  static constexpr std::size_t kRight{2};
  static constexpr std::size_t kLeft{3};
  static constexpr std::size_t kFrontRight{4};
  static constexpr std::size_t kFrontLeft{5};
  static constexpr std::size_t kBackRight{6};
  static constexpr std::size_t kBackLeft{7};

  struct CharacterTextures
  {
    graphics::TextureHandle front_atlas;
    graphics::TextureHandle back_atlas;
    graphics::TextureHandle side_atlas;
    graphics::TextureHandle front_side_atlas;
    graphics::TextureHandle back_side_atlas;
  };

  void createMovementTileSets(game::Assets& assets, graphics::TileSetHandle* movement_tilsets, const CharacterTextures& character_textures, std::size_t cardinal_start_offset, std::size_t ordinal_start_offset)
  {
    using namespace sde::graphics;

    movement_tilsets[kFront] = [&]
    {
      auto frames_or_error = assets.graphics.tile_sets.create(
        assets.graphics.textures,
        character_textures.front_atlas,
        TileSetSliceUniform{
          .tile_size_px = {64, 64},
          .tile_orientation_x = TileOrientation::kNormal,
          .tile_orientation_y = TileOrientation::kNormal,
          .direction = TileSliceDirection::kRowWise,
          .start_offset = cardinal_start_offset,
          .stop_after = 6,
        });
      SDE_ASSERT_TRUE(frames_or_error.has_value());
      return frames_or_error->handle;
    }();

    movement_tilsets[kBack] = [&]
    {
      auto frames_or_error = assets.graphics.tile_sets.create(
        assets.graphics.textures,
        character_textures.back_atlas,
        TileSetSliceUniform{
          .tile_size_px = {64, 64},
          .tile_orientation_x = TileOrientation::kNormal,
          .tile_orientation_y = TileOrientation::kNormal,
          .direction = TileSliceDirection::kRowWise,
          .start_offset = cardinal_start_offset,
          .stop_after = 6,
        });
      SDE_ASSERT_TRUE(frames_or_error.has_value());
      return frames_or_error->handle;
    }();

    movement_tilsets[kRight] = [&]
    {
      auto frames_or_error = assets.graphics.tile_sets.create(
        assets.graphics.textures,
        character_textures.side_atlas,
        TileSetSliceUniform{
          .tile_size_px = {64, 64},
          .tile_orientation_x = TileOrientation::kNormal,
          .tile_orientation_y = TileOrientation::kNormal,
          .direction = TileSliceDirection::kRowWise,
          .start_offset = cardinal_start_offset,
          .stop_after = 6,
        });
      SDE_ASSERT_TRUE(frames_or_error.has_value());
      return frames_or_error->handle;
    }();

    movement_tilsets[kLeft] = [&]
    {
      auto frames_or_error = assets.graphics.tile_sets.create(
        assets.graphics.textures,
        character_textures.side_atlas,
        TileSetSliceUniform{
          .tile_size_px = {64, 64},
          .tile_orientation_x = TileOrientation::kFlipped,
          .tile_orientation_y = TileOrientation::kNormal,
          .direction = TileSliceDirection::kRowWise,
          .start_offset = cardinal_start_offset,
          .stop_after = 6,
        });
      SDE_ASSERT_TRUE(frames_or_error.has_value());
      return frames_or_error->handle;
    }();

    movement_tilsets[kFrontRight] = [&]
    {
      auto frames_or_error = assets.graphics.tile_sets.create(
        assets.graphics.textures,
        character_textures.front_side_atlas,
        TileSetSliceUniform{
          .tile_size_px = {64, 64},
          .tile_orientation_x = TileOrientation::kNormal,
          .tile_orientation_y = TileOrientation::kNormal,
          .direction = TileSliceDirection::kRowWise,
          .start_offset = ordinal_start_offset,
          .stop_after = 6,
        });
      SDE_ASSERT_TRUE(frames_or_error.has_value());
      return frames_or_error->handle;
    }();

    movement_tilsets[kFrontLeft] = [&]
    {
      auto frames_or_error = assets.graphics.tile_sets.create(
        assets.graphics.textures,
        character_textures.front_side_atlas,
        TileSetSliceUniform{
          .tile_size_px = {64, 64},
          .tile_orientation_x = TileOrientation::kFlipped,
          .tile_orientation_y = TileOrientation::kNormal,
          .direction = TileSliceDirection::kRowWise,
          .start_offset = ordinal_start_offset,
          .stop_after = 6,
        });
      SDE_ASSERT_TRUE(frames_or_error.has_value());
      return frames_or_error->handle;
    }();

    movement_tilsets[kBackRight] = [&]
    {
      auto frames_or_error = assets.graphics.tile_sets.create(
        assets.graphics.textures,
        character_textures.back_side_atlas,
        TileSetSliceUniform{
          .tile_size_px = {64, 64},
          .tile_orientation_x = TileOrientation::kNormal,
          .tile_orientation_y = TileOrientation::kNormal,
          .direction = TileSliceDirection::kRowWise,
          .start_offset = ordinal_start_offset,
          .stop_after = 6,
        });
      SDE_ASSERT_TRUE(frames_or_error.has_value());
      return frames_or_error->handle;
    }();

    movement_tilsets[kBackLeft] = [&]
    {
      auto frames_or_error = assets.graphics.tile_sets.create(
        assets.graphics.textures,
        character_textures.back_side_atlas,
        TileSetSliceUniform{
          .tile_size_px = {64, 64},
          .tile_orientation_x = TileOrientation::kFlipped,
          .tile_orientation_y = TileOrientation::kNormal,
          .direction = TileSliceDirection::kRowWise,
          .start_offset = ordinal_start_offset,
          .stop_after = 6,
        });
      SDE_ASSERT_TRUE(frames_or_error.has_value());
      return frames_or_error->handle;
    }();
  }

  bool onInitialize(entt::registry& registry, game::Assets& assets)
  {
    const CharacterTextures character_textures{
      .front_atlas = [&]
      {
        auto atlas_or_error = assets.textures_from_disk.create("/home/brian/dev/assets/sprites/red/Top Down/Front Movement.png");
        SDE_ASSERT_TRUE(atlas_or_error.has_value());
        return atlas_or_error->handle;
      }(),
      .back_atlas = [&]
      {
        auto atlas_or_error = assets.textures_from_disk.create("/home/brian/dev/assets/sprites/red/Top Down/Back Movement.png");
        SDE_ASSERT_TRUE(atlas_or_error.has_value());
        return atlas_or_error->handle;
      }(),
      .side_atlas = [&]
      {
        auto atlas_or_error = assets.textures_from_disk.create("/home/brian/dev/assets/sprites/red/Top Down/Side Movement.png");
        SDE_ASSERT_TRUE(atlas_or_error.has_value());
        return atlas_or_error->handle;
      }(),
      .front_side_atlas = [&]
      {
        auto atlas_or_error = assets.textures_from_disk.create("/home/brian/dev/assets/sprites/red/Top Down/FrontSide Movement.png");
        SDE_ASSERT_TRUE(atlas_or_error.has_value());
        return atlas_or_error->handle;
      }(),
      .back_side_atlas = [&]
      {
        auto atlas_or_error = assets.textures_from_disk.create("/home/brian/dev/assets/sprites/red/Top Down/BackSide Movement.png");
        SDE_ASSERT_TRUE(atlas_or_error.has_value());
        return atlas_or_error->handle;
      }()
    };

    createMovementTileSets(assets, idle_, character_textures, 18UL, 12UL);
    createMovementTileSets(assets, walk_, character_textures, 12UL, 12UL);
    createMovementTileSets(assets, run_, character_textures, 6UL, 6UL);

    id_ = registry.create();
    registry.emplace<Info>(id_, Info{{"bob"}});
    registry.emplace<Size>(id_, Size{{1.5F, 1.5F}});
    registry.emplace<State>(id_, State{Vec2f::Zero(), Vec2f::Zero(), {0, -1}});
    registry.emplace<graphics::AnimatedSprite>(id_).setMode(graphics::AnimatedSprite::Mode::kLooped);

    return true;
  }

  expected<void, game::ScriptError> onUpdate(entt::registry& registry, const game::Assets& assets, const AppProperties& app)
  {
    auto [size, state, sprite] = registry.get<Size, State, graphics::AnimatedSprite>(id_);

    static constexpr float kSpeedWalking = 0.5;
    static constexpr float kSpeedRunning = 1.0;

    // Handle character speed
    const float next_speed =
      app.keys.isDown(KeyCode::kLShift) ? kSpeedRunning : kSpeedWalking;

    state.velocity.setZero();

    // Handle movement controls
    if (app.keys.isDown(KeyCode::kA))
    {
      state.velocity.x() = -next_speed;
    }
    if (app.keys.isDown(KeyCode::kD))
    {
      state.velocity.x() = +next_speed;
    }
    if (app.keys.isDown(KeyCode::kS))
    {
      state.velocity.y() = -next_speed;
    }
    if (app.keys.isDown(KeyCode::kW))
    {
      state.velocity.y() = +next_speed;
    }

    graphics::TileSetHandle frames;

    // Handle next animation
    if ((state.velocity.x() > 0) and (state.velocity.y() > 0))
    {
      sprite.setFrames((next_speed == kSpeedWalking) ? walk_[kBackRight] : run_[kBackRight]);
    }
    else if ((state.velocity.x() < 0) and (state.velocity.y() > 0))
    {
      sprite.setFrames((next_speed == kSpeedWalking) ? walk_[kBackLeft] : run_[kBackLeft]);
    }
    else if ((state.velocity.x() > 0) and (state.velocity.y() < 0))
    {
      sprite.setFrames((next_speed == kSpeedWalking) ? walk_[kFrontRight] : run_[kFrontRight]);
    }
    else if ((state.velocity.x() < 0) and (state.velocity.y() < 0))
    {
      sprite.setFrames((next_speed == kSpeedWalking) ? walk_[kFrontLeft] : run_[kFrontLeft]);
    }
    else if (state.velocity.x() > 0)
    {
      sprite.setFrames((next_speed == kSpeedWalking) ? walk_[kRight] : run_[kRight]);
    }
    else if (state.velocity.x() < 0)
    {
      sprite.setFrames((next_speed == kSpeedWalking) ? walk_[kLeft] : run_[kLeft]);
    }
    else if (state.velocity.y() < 0)
    {
      sprite.setFrames((next_speed == kSpeedWalking) ? walk_[kFront] : run_[kFront]);
    }
    else if (state.velocity.y() > 0)
    {
      sprite.setFrames((next_speed == kSpeedWalking) ? walk_[kBack] : run_[kBack]);
    }
    else if ((state.looking.x() > 0) and (state.looking.y() > 0))
    {
      sprite.setFrames(idle_[kBackRight]);
    }
    else if ((state.looking.x() < 0) and (state.looking.y() > 0))
    {
      sprite.setFrames(idle_[kBackLeft]);
    }
    else if ((state.looking.x() > 0) and (state.looking.y() < 0))
    {
      sprite.setFrames(idle_[kFrontRight]);
    }
    else if ((state.looking.x() < 0) and (state.looking.y() < 0))
    {
      sprite.setFrames(idle_[kBackLeft]);
    }
    else if (state.looking.x() > 0)
    {
      sprite.setFrames(idle_[kRight]);
    }
    else if (state.looking.x() < 0)
    {
      sprite.setFrames(idle_[kLeft]);
    }
    else if (state.looking.y() < 0)
    {
      sprite.setFrames(idle_[kFront]);
    }
    else if (state.looking.y() > 0)
    {
      sprite.setFrames(idle_[kBack]);
    }

    // Set sprite stuff
    if ((state.velocity.array() != 0.0F).any())
    {
      state.looking = state.velocity;
      sprite.setFrameRate(Hertz(next_speed * 15.0F));
    }
    else
    {
      sprite.setFrameRate(Hertz(kSpeedWalking * 15.0F));
    }

    // Update position
    state.position += state.velocity * toSeconds(app.time_delta);

    return {};
  }

  entt::entity id_;
  graphics::TileSetHandle idle_[8UL];
  graphics::TileSetHandle walk_[8UL];
  graphics::TileSetHandle run_[8UL];
};

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


  auto renderer_or_error = Renderer2D::create();
  SDE_ASSERT_TRUE(renderer_or_error.has_value());


  RenderResources sprite_rendering_resources;
  sprite_rendering_resources.shader = (*sprite_shader_or_error);
  sprite_rendering_resources.buffer_group = 0;

  RenderAttributes attributes;

  sde::Vec2f position{0, 0};
  sde::Vec2f direction{0, -1};
  sde::Vec2f direction_looking{0, -1};



  AnimatedSprite animated_sprite;
  animated_sprite.setFrameRate(Hertz(5.0F));
  animated_sprite.setMode(AnimatedSprite::Mode::kLooped);

  entt::registry reg;
  Character character_script;

  app_or_error->spin([&](const auto& window)
  {
    // Handle screen zoom
    static constexpr float kScaleRate = 100.0;
    const float scroll_sensitivity = std::clamp(attributes.scaling, 1e-4F, 1e-2F);
    if (window.mouse_scroll.y() > 0)
    {
      attributes.scaling -= scroll_sensitivity * kScaleRate * sde::toSeconds(window.time_delta);
    }
    else if (window.mouse_scroll.y() < 0)
    {
      attributes.scaling += scroll_sensitivity * kScaleRate * sde::toSeconds(window.time_delta);
    }
    attributes.scaling = std::max(attributes.scaling, 1e-3F);
    attributes.time = window.time;
    attributes.time_delta = window.time_delta;


    character_script.update(reg, assets, window);

    window_target_or_error->refresh(Black());
    if (auto render_pass_or_error = RenderPass::create(*window_target_or_error, *renderer_or_error, assets.graphics, attributes, sprite_rendering_resources); render_pass_or_error.has_value())
    {
      reg.view<Size, State, graphics::AnimatedSprite>().each(
        [&](const Size& size, const State& state, const graphics::AnimatedSprite& sprite)
        {
          const Vec2f min_corner{state.position - 0.5F * size.extent};
          const Vec2f max_corner{state.position + 0.5F * size.extent};
          sprite.draw(*render_pass_or_error, window.time, {min_corner, max_corner});
        });
    }

    if (auto render_pass_or_error = RenderPass::create(*window_target_or_error, *renderer_or_error, assets.graphics, attributes, text_rendering_resources); render_pass_or_error.has_value())
    {
      reg.view<Info, Size, State>().each(
        [&](const Info& info, const Size& size, const State& state)
        {
          type_setter.draw(*render_pass_or_error, info.name, state.position + sde::Vec2f{0.0, 0.3F}, {0.075F});
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
