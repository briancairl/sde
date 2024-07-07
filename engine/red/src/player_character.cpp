// C++ Standard Library
#include <cmath>
#include <ostream>

// EnTT
#include <entt/entt.hpp>

// SDE
#include "sde/game/assets.hpp"
#include "sde/game/script.hpp"
#include "sde/game/systems.hpp"
#include "sde/graphics/sprite.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/graphics/tile_set.hpp"
#include "sde/logging.hpp"

// RED
#include "red/player_character.hpp"

using namespace sde;
using namespace sde::graphics;

namespace
{

constexpr std::size_t kFront{0};
constexpr std::size_t kBack{1};
constexpr std::size_t kRight{2};
constexpr std::size_t kLeft{3};
constexpr std::size_t kFrontRight{4};
constexpr std::size_t kFrontLeft{5};
constexpr std::size_t kBackRight{6};
constexpr std::size_t kBackLeft{7};

struct CharacterTextures
{
  TextureHandle front_atlas;
  TextureHandle back_atlas;
  TextureHandle side_atlas;
  TextureHandle front_side_atlas;
  TextureHandle back_side_atlas;
};

void createMovementTileSets(
  game::Assets& assets,
  TileSetHandle* movement_tilsets,
  const CharacterTextures& character_textures,
  std::size_t cardinal_start_offset,
  std::size_t ordinal_start_offset)
{
  movement_tilsets[kFront] = [&] {
    auto frames_or_error = assets.graphics.tile_sets.find_or_create(
      movement_tilsets[kFront],
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

  movement_tilsets[kBack] = [&] {
    auto frames_or_error = assets.graphics.tile_sets.find_or_create(
      movement_tilsets[kBack],
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

  movement_tilsets[kRight] = [&] {
    auto frames_or_error = assets.graphics.tile_sets.find_or_create(
      movement_tilsets[kRight],
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

  movement_tilsets[kLeft] = [&] {
    auto frames_or_error = assets.graphics.tile_sets.find_or_create(
      movement_tilsets[kLeft],
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

  movement_tilsets[kFrontRight] = [&] {
    auto frames_or_error = assets.graphics.tile_sets.find_or_create(
      movement_tilsets[kFrontRight],
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

  movement_tilsets[kFrontLeft] = [&] {
    auto frames_or_error = assets.graphics.tile_sets.find_or_create(
      movement_tilsets[kFrontLeft],
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

  movement_tilsets[kBackRight] = [&] {
    auto frames_or_error = assets.graphics.tile_sets.find_or_create(
      movement_tilsets[kBackRight],
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

  movement_tilsets[kBackLeft] = [&] {
    auto frames_or_error = assets.graphics.tile_sets.find_or_create(
      movement_tilsets[kBackLeft],
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

}  // namespace

bool PlayerCharacter::onInitialize(
  entt::registry& registry,
  game::Systems& systems,
  game::Assets& assets,
  const AppProperties& app)
{
  const CharacterTextures character_textures{
    .front_atlas =
      [&] {
        auto atlas_or_error =
          assets.graphics.textures.create("/home/brian/dev/assets/sprites/red/Top Down/Front Movement.png");
        SDE_ASSERT_TRUE(atlas_or_error.has_value());
        return atlas_or_error->handle;
      }(),
    .back_atlas =
      [&] {
        auto atlas_or_error =
          assets.graphics.textures.create("/home/brian/dev/assets/sprites/red/Top Down/Back Movement.png");
        SDE_ASSERT_TRUE(atlas_or_error.has_value());
        return atlas_or_error->handle;
      }(),
    .side_atlas =
      [&] {
        auto atlas_or_error =
          assets.graphics.textures.create("/home/brian/dev/assets/sprites/red/Top Down/Side Movement.png");
        SDE_ASSERT_TRUE(atlas_or_error.has_value());
        return atlas_or_error->handle;
      }(),
    .front_side_atlas =
      [&] {
        auto atlas_or_error =
          assets.graphics.textures.create("/home/brian/dev/assets/sprites/red/Top Down/FrontSide Movement.png");
        SDE_ASSERT_TRUE(atlas_or_error.has_value());
        return atlas_or_error->handle;
      }(),
    .back_side_atlas =
      [&] {
        auto atlas_or_error =
          assets.graphics.textures.create("/home/brian/dev/assets/sprites/red/Top Down/BackSide Movement.png");
        SDE_ASSERT_TRUE(atlas_or_error.has_value());
        return atlas_or_error->handle;
      }()};

  createMovementTileSets(assets, idle_, character_textures, 18UL, 12UL);
  createMovementTileSets(assets, walk_, character_textures, 12UL, 12UL);
  createMovementTileSets(assets, run_, character_textures, 6UL, 6UL);

  id_ = registry.create();
  registry.emplace<Focused>(id_);
  registry.emplace<Midground>(id_);
  registry.emplace<Info>(id_, Info{{"bob"}});
  registry.emplace<Size>(id_, Size{{1.5F, 1.5F}});
  registry.emplace<Position>(id_, Position{Vec2f::Zero()});
  registry.emplace<Dynamics>(id_, Dynamics{Vec2f::Zero(), {0, -1}});
  registry.emplace<graphics::AnimatedSprite>(id_).setMode(graphics::AnimatedSprite::Mode::kLooped);

  return true;
}

expected<void, game::ScriptError> PlayerCharacter::onUpdate(
  entt::registry& registry,
  game::Systems& systems,
  const game::Assets& assets,
  const AppProperties& app)
{
  auto [size, position, state, sprite] = registry.get<Size, Position, Dynamics, graphics::AnimatedSprite>(id_);

  static constexpr float kSpeedWalking = 0.5;
  static constexpr float kSpeedRunning = 1.0;

  // Handle character speed
  const float next_speed = app.keys.isDown(KeyCode::kLShift) ? kSpeedRunning : kSpeedWalking;

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

  TileSetHandle frames;

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

  if (auto listener_or_err = audio::ListenerTarget::create(systems.mixer, kPlayerListener); listener_or_err.has_value())
  {
    listener_or_err->set(audio::ListenerState{
      .position = Vec3f{position.center.x(), position.center.y(), 1.0F},
      .velocity = Vec3f{state.velocity.x(), state.velocity.y(), 0.0F},
      .orientation_at = Vec3f::UnitY(),
      .orientation_up = Vec3f::UnitZ(),
    });
  }

  return {};
}
