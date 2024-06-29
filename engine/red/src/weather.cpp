// C++ Standard Library
#include <cmath>
#include <ostream>

// EnTT
#include <entt/entt.hpp>

// SDE
#include "sde/game/assets.hpp"
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
    assets.textures_from_disk.create("/home/brian/dev/assets/sprites/weather/weather_effects/light_rain.png");
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

  float x_start = -4.0F;
  for (int xi = 0; xi < 20; ++xi)
  {
    const auto time_offset = Seconds(static_cast<float>(rand()));
    float y_start = -2.0F;
    for (int yi = 0; yi < 4; ++yi)
    {
      const auto id = registry.create();
      registry.emplace<Foreground>(id);
      registry.emplace<Size>(id, Size{{0.25F, 1.0F}});
      registry.emplace<Position>(id, Position{{x_start, y_start}});

      auto& sprite = registry.emplace<graphics::AnimatedSprite>(id);
      sprite.setMode(graphics::AnimatedSprite::Mode::kLooped);
      sprite.setFrames(rain_);
      sprite.setFrameRate(Hertz(15.F));
      sprite.setTimeOffset(time_offset);
      y_start += 1.0F;
    }
    x_start += 0.25F;
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
