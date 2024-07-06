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
#include "sde/game/systems.hpp"
#include "sde/graphics/image.hpp"
#include "sde/logging.hpp"
#include "sde/serialization_binary_file.hpp"
// #include "sde/view.hpp"

// RED
#include "red/player_character.hpp"
#include "red/renderer.hpp"
#include "red/weather.hpp"


using namespace sde;

int main(int argc, char** argv)
{
  using namespace sde::audio;
  using namespace sde::graphics;

  SDE_LOG_INFO("starting...");

  game::Assets assets;

  if (auto ifs_or_error = serial::file_istream::create("/tmp/game.bin"); ifs_or_error.has_value())
  {
    serial::binary_iarchive iar{*ifs_or_error};
    iar >> serial::named{"assets", assets};
  }

  auto icon_or_error =
    assets.graphics.images.create("/home/brian/dev/assets/icons/red.png", ImageOptions{ImageChannels::kRGBA});
  SDE_ASSERT_TRUE(icon_or_error.has_value());

  auto cursor_or_error =
    assets.graphics.images.create("/home/brian/dev/assets/icons/sword.png", ImageOptions{ImageChannels::kRGBA});
  SDE_ASSERT_TRUE(cursor_or_error.has_value());

  auto app_or_error = App::create(
    {.initial_size = {1000, 500}, .icon = icon_or_error->value->ref(), .cursor = cursor_or_error->value->ref()});
  SDE_ASSERT_TRUE(app_or_error.has_value());

  game::Systems systems{game::Systems::create().value()};

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

  entt::registry reg;
  PlayerCharacter character_script;
  Renderer renderer_script;
  Weather weather_script;

  app_or_error->spin([&](const auto& window) {
    character_script.update(reg, systems, assets, window);
    reg.view<Position, Dynamics>().each(
      [dt = toSeconds(window.time_delta)](Position& pos, const Dynamics& state) { pos.center += state.velocity * dt; });
    renderer_script.update(reg, systems, assets, window);
    weather_script.update(reg, systems, assets, window);
    return AppDirective::kContinue;
  });

  SDE_LOG_INFO("done.");

  if (auto ofs_or_error = serial::file_ostream::create("/tmp/game.bin"); ofs_or_error.has_value())
  {
    serial::binary_oarchive oar{*ofs_or_error};
    oar << serial::named{"assets", assets};
  }

  return 0;
}
