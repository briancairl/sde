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
#include "sde/game/resources.hpp"
#include "sde/graphics/image.hpp"
#include "sde/logging.hpp"
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

  auto icon_or_error = Image::load("/home/brian/dev/assets/icons/red.png");
  SDE_ASSERT_TRUE(icon_or_error.has_value());

  auto app_or_error = App::create({
    .initial_size = {1000, 500}, .icon = std::addressof(*icon_or_error),
    //.cursor = std::addressof(*icon_or_error),  // <-- this works, but need a better image
  });
  SDE_ASSERT_TRUE(app_or_error.has_value());


  game::Assets assets;
  game::Resources resources{game::Resources::create().value()};

  auto background_track_1_or_error = assets.audio.sounds.load("/home/brian/dev/assets/sounds/tracks/OldTempleLoop.wav");
  SDE_ASSERT_TRUE(background_track_1_or_error.has_value());

  auto background_track_2_or_error = assets.audio.sounds.load("/home/brian/dev/assets/sounds/tracks/forest.wav");
  SDE_ASSERT_TRUE(background_track_2_or_error.has_value());

  if (auto listener_or_err = ListenerTarget::create(resources.mixer, 0UL); listener_or_err.has_value())
  {
    listener_or_err->set(*background_track_1_or_error, TrackOptions{.volume = 0.2F, .looped = true});
    listener_or_err->set(*background_track_2_or_error, TrackOptions{.volume = 0.4F, .looped = true});
  }

  entt::registry reg;
  PlayerCharacter character_script;
  Renderer renderer_script;
  Weather weather_script;

  app_or_error->spin([&](const auto& window) {
    character_script.update(reg, resources, assets, window);
    reg.view<Position, Dynamics>().each(
      [dt = toSeconds(window.time_delta)](Position& pos, const Dynamics& state) { pos.center += state.velocity * dt; });
    renderer_script.update(reg, resources, assets, window);
    weather_script.update(reg, resources, assets, window);
    return AppDirective::kContinue;
  });

  SDE_LOG_INFO("done.");
  return 0;
}
