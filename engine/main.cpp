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
#include "red/renderer.hpp"

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

  auto background_track_1_or_error = assets.sounds_from_disk.create("/home/brian/dev/assets/sounds/tracks/OldTempleLoop.wav");
  SDE_ASSERT_TRUE(background_track_1_or_error.has_value());

  auto background_track_2_or_error = assets.sounds_from_disk.create("/home/brian/dev/assets/sounds/tracks/forest.wav");
  SDE_ASSERT_TRUE(background_track_2_or_error.has_value());

  if (auto listener_or_err = ListenerTarget::create(resources.mixer, 0UL); listener_or_err.has_value())
  {
    listener_or_err->set(*background_track_1_or_error, TrackOptions{.gain=0.3F, .looped=true});
    listener_or_err->set(*background_track_2_or_error, TrackOptions{.gain=1.0F, .looped=true});
  }

  entt::registry reg;
  PlayerCharacter character_script;
  Renderer renderer_script;

  app_or_error->spin([&](const auto& window)
  {
    character_script.update(reg, resources, assets, window);
    reg.view<State>().each([dt = toSeconds(window.time_delta)](State& state) { state.position += state.velocity * dt; });
    renderer_script.update(reg, resources, assets, window);
    return AppDirective::kContinue;
  });

  SDE_LOG_INFO("done.");
  return 0;
}

// clang-format on
