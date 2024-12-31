// C++ Standard Library
#include <cmath>
#include <ostream>

// EnTT
#include <entt/entt.hpp>

// SDE
#include "sde/game/assets.hpp"
#include "sde/game/script_impl.hpp"
#include "sde/graphics/sprite.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/graphics/tile_set.hpp"
#include "sde/logging.hpp"

// RED
#include "red/components/common.hpp"
#include "red/weather.hpp"

using namespace sde;
using namespace sde::game;
using namespace sde::graphics;
using namespace sde::audio;


class Weather final : public ScriptRuntime
{
public:
  Weather() : ScriptRuntime{"Weather"} {}

private:
  TextureHandle rain_frames_atlas_;
  TileSetHandle rain_frames_;
  SoundHandle rain_sound_;

  bool onLoad(IArchive& ar, SharedAssets& assets) override
  {
    using namespace sde::serial;
    ar >> Field{"rain_frames_atlas", rain_frames_atlas_};
    ar >> Field{"rain_frames", rain_frames_};
    ar >> Field{"rain_sound", rain_sound_};
    return true;
  }

  bool onSave(OArchive& ar, const SharedAssets& assets) const override
  {
    using namespace sde::serial;
    ar << Field{"rain_frames_atlas", rain_frames_atlas_};
    ar << Field{"rain_frames", rain_frames_};
    ar << Field{"rain_sound", rain_sound_};
    return true;
  }

  bool onInitialize(SharedAssets& assets, AppState& app_state, const AppProperties& app) override
  {
    if (!assets.assign(
          rain_frames_atlas_, "/home/brian/dev/assets/sprites/weather/weather_effects/light_rain.png"_path))
    {
      SDE_LOG_ERROR("Missing texture");
      return false;
    }

    static const TileSetSliceUniform kTileSlicing{
      .tile_size_px = {32, 128},
      .tile_orientation_x = TileOrientation::kNormal,
      .tile_orientation_y = TileOrientation::kNormal,
      .direction = TileSliceDirection::kColWise};

    if (!assets.assign(rain_frames_, rain_frames_atlas_, kTileSlicing))
    {
      SDE_LOG_ERROR("Missing text shader");
      return false;
    }

    if (!assets.assign(rain_sound_, "/home/brian/dev/assets/sounds/fx/rain1_mono.wav"_path))
    {
      SDE_LOG_ERROR("Failed to load rain");
      return false;
    }

    float x_start = -4.0F;
    for (int xi = 0; xi < 32; ++xi)
    {
      const auto time_offset = Seconds(static_cast<float>(rand()));
      float y_start = -4.0F;
      for (int yi = 0; yi < 16; ++yi)
      {
        const auto id = assets.registry.create();
        assets.registry.emplace<Foreground>(id);
        assets.registry.emplace<Size>(id, Size{.extent = {0.25F, 1.0F}});
        assets.registry.emplace<Position>(id, Position{.center = {x_start, y_start}});

        auto& sprite = assets.registry.emplace<AnimatedSprite>(id);
        sprite.setMode(AnimatedSprite::Mode::kLooped);
        sprite.setFrames(rain_frames_);
        sprite.setFrameRate(Hertz(15.F));
        const float alpha = (1.0F - std::abs(x_start) / 4.0F) * (1.0F - std::abs(y_start) / 4.0F);
        sprite.setTintColor(Vec4f{1.0F, 1.0F, 1.0F, alpha});
        sprite.setTimeOffset(time_offset);
        y_start += 1.0F;
      }
      x_start += 0.25F;
    }

    if (auto listener_or_err = audio::ListenerTarget::create(systems.mixer, kPlayerListener);
        !listener_or_err.has_value())
    {
      SDE_LOG_ERROR("Failed to create listener pass");
      return false;
    }
    else if (auto sound = assets.audio.sounds(rain_sound_))
    {
      listener_or_err->set(
        sound.get(),
        audio::TrackOptions{
          .position = Vec3f{0.0F, 0.0F, -0.5F},
          .velocity = Vec3f::Zero(),
          .orientation = Vec3f::UnitZ(),
          .volume = 0.5F,
          .cutoff_distance = 8.0F,
          .looped = true});
    }
    else
    {
      SDE_LOG_ERROR("Failed to locate sound");
      return false;
    }
    return true;
  }

  expected<void, ScriptError> onUpdate(SharedAssets& assets, AppState& app_state, const AppProperties& app) override
  {
    return {};
  }
};

std::unique_ptr<ScriptRuntime> createWeather() { return std::make_unique<Weather>(); }
