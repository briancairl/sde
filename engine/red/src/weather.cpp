// C++ Standard Library
#include <cmath>
#include <ostream>

// EnTT
#include <entt/entt.hpp>

// SDE
#include "sde/game/assets.hpp"
#include "sde/game/resources.hpp"
#include "sde/game/script.hpp"
#include "sde/graphics/sprite.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/graphics/tile_set.hpp"
#include "sde/logging.hpp"

// RED
#include "red/weather.hpp"

using namespace sde;
using namespace sde::graphics;


bool Weather::onInitialize(
  entt::registry& registry,
  sde::game::Resources& resources,
  game::Assets& assets,
  const AppProperties& app)
{
  using namespace sde::graphics;

  auto atlas_or_error =
    assets.graphics.textures.load("/home/brian/dev/assets/sprites/weather/weather_effects/light_rain.png");
  if (!atlas_or_error.has_value())
  {
    return false;
  }

  auto rain_frames_or_error = assets.graphics.tile_sets.create(
    assets.graphics.textures,
    atlas_or_error->handle,
    TileSetSliceUniform{
      .tile_size_px = {32, 128},
      .tile_orientation_x = TileOrientation::kNormal,
      .tile_orientation_y = TileOrientation::kNormal,
      .direction = TileSliceDirection::kColWise});
  if (!rain_frames_or_error.has_value())
  {
    return false;
  }
  rain_ = rain_frames_or_error->handle;

  auto sound_or_error = assets.audio.sounds.load("/home/brian/dev/assets/sounds/fx/rain1_mono.wav");
  if (!sound_or_error.has_value())
  {
    return false;
  }
  rain_sound_ = sound_or_error->handle;


  float x_start = -4.0F;
  for (int xi = 0; xi < 32; ++xi)
  {
    const auto time_offset = Seconds(static_cast<float>(rand()));
    float y_start = -4.0F;
    for (int yi = 0; yi < 16; ++yi)
    {
      const auto id = registry.create();
      registry.emplace<Foreground>(id);
      registry.emplace<Size>(id, Size{{0.25F, 1.0F}});
      registry.emplace<Position>(id, Position{{x_start, y_start}});

      auto& sprite = registry.emplace<graphics::AnimatedSprite>(id);
      sprite.setMode(graphics::AnimatedSprite::Mode::kLooped);
      sprite.setFrames(rain_);
      sprite.setFrameRate(Hertz(15.F));
      const float alpha = (1.0F - std::abs(x_start) / 4.0F) * (1.0F - std::abs(y_start) / 4.0F);
      sprite.setTintColor(Vec4f{1.0F, 1.0F, 1.0F, alpha});
      sprite.setTimeOffset(time_offset);
      y_start += 1.0F;
    }
    x_start += 0.25F;
  }

  if (auto listener_or_err = audio::ListenerTarget::create(resources.mixer, kPlayerListener);
      listener_or_err.has_value())
  {
    listener_or_err->set(
      *sound_or_error->value,
      audio::TrackOptions{
        .position = Vec3f{0.0F, 0.0F, -0.5F},
        .velocity = Vec3f::Zero(),
        .orientation = Vec3f::UnitZ(),
        .volume = 0.5F,
        .cutoff_distance = 8.0F,
        .looped = true});
  }

  return true;
}

expected<void, game::ScriptError> Weather::onUpdate(
  entt::registry& registry,
  sde::game::Resources& resources,
  const game::Assets& assets,
  const AppProperties& app)
{
  return {};
}
