// C++ Standard Library
#include <cmath>
#include <ostream>

// EnTT
#include <entt/entt.hpp>

// SDE
#include "sde/app.hpp"
#include "sde/audio/assets.hpp"
#include "sde/audio/mixer.hpp"
#include "sde/game/assets.hpp"
#include "sde/game/assets_io.hpp"
#include "sde/game/script.hpp"
#include "sde/game/systems.hpp"
#include "sde/graphics/image.hpp"
#include "sde/logging.hpp"
#include "sde/serialization_binary_file.hpp"
// #include "sde/view.hpp"

// RED
#include "red/components.hpp"
#include "red/player_character.hpp"
#include "red/renderer.hpp"
#include "red/weather.hpp"


using namespace sde;

int main(int argc, char** argv)
{
  using namespace sde::audio;
  using namespace sde::graphics;

  SDE_LOG_INFO("starting...");


  auto app_or_error = App::create({.initial_size = {1000, 500}});
  SDE_ASSERT_TRUE(app_or_error.has_value());

  game::Systems systems{game::Systems::create().value()};

  game::Assets assets{systems};

  entt::registry reg;
  auto character_script = createPlayerCharacter();
  auto renderer_script = createRenderer();
  auto weather_script = createWeather();

  if (auto ifs_or_error = serial::file_istream::create("/tmp/game.bin"); ifs_or_error.has_value())
  {
    serial::binary_iarchive iar{*ifs_or_error};
    iar >> serial::named{"assets", assets};
    if (auto ok_or_error = assets.refresh())
    {
      character_script->load(iar);
      renderer_script->load(iar);
      weather_script->load(iar);
    }
    else
    {
      SDE_FAIL("failed asset resolution");
    }
  }


  // auto icon_or_error = assets.graphics.images.find_or_create(
  //   [](const auto& value, const auto& path, auto&&...) -> bool { return value.path == path; },
  //   "/home/brian/dev/assets/icons/red.png",
  //   ImageOptions{ImageChannels::kRGBA});
  // SDE_ASSERT_TRUE(icon_or_error.has_value());

  // auto cursor_or_error = assets.graphics.images.find_or_create(
  //   [](const auto& value, const auto& path, auto&&...) -> bool { return value.path == path; },
  //   "/home/brian/dev/assets/icons/sword.png",
  //   ImageOptions{ImageChannels::kRGBA});
  // SDE_ASSERT_TRUE(cursor_or_error.has_value());

  // app_or_error->window().setIcon(icon_or_error->value->ref());
  // app_or_error->window().setCursor(cursor_or_error->value->ref());


  auto background_track_1_or_error =
    assets.audio.sounds.create("/home/brian/dev/assets/sounds/tracks/OldTempleLoop.wav");
  SDE_ASSERT_TRUE(background_track_1_or_error.has_value());

  auto background_track_2_or_error = assets.audio.sounds.create("/home/brian/dev/assets/sounds/tracks/forest.wav");
  SDE_ASSERT_TRUE(background_track_2_or_error.has_value());

  if (auto listener_or_err = ListenerTarget::create(systems.mixer, 0UL); listener_or_err.has_value())
  {
    listener_or_err->set(*background_track_1_or_error, TrackOptions{.volume = 0.2F, .looped = true});
    listener_or_err->set(*background_track_2_or_error, TrackOptions{.volume = 0.4F, .looped = true});
  }

  app_or_error->spin([&](const auto& window) {
    reg.view<Position, Dynamics>().each(
      [dt = toSeconds(window.time_delta)](Position& pos, const Dynamics& state) { pos.center += state.velocity * dt; });

    if (!character_script->update(reg, systems, assets, window))
    {
      SDE_LOG_ERROR("character_script->update(...) failed");
      return AppDirective::kClose;
    }

    if (!renderer_script->update(reg, systems, assets, window))
    {
      SDE_LOG_ERROR("renderer_script->update(...) failed");
      return AppDirective::kClose;
    }

    if (!weather_script->update(reg, systems, assets, window))
    {
      SDE_LOG_ERROR("weather_script->update(...) failed");
      return AppDirective::kClose;
    }

    return AppDirective::kContinue;
  });

  SDE_LOG_INFO("done.");

  if (auto ofs_or_error = serial::file_ostream::create("/tmp/game.bin"); ofs_or_error.has_value())
  {
    serial::binary_oarchive oar{*ofs_or_error};
    oar << serial::named{"assets", assets};
    character_script->save(oar);
    renderer_script->save(oar);
    weather_script->save(oar);
  }

  return 0;
}
