// C++ Standard Library
#include <cmath>
#include <ostream>
#include <vector>

// SDE
#include "sde/app.hpp"
#include "sde/audio/assets.hpp"
#include "sde/audio/mixer.hpp"
#include "sde/game/assets.hpp"
#include "sde/game/script.hpp"
#include "sde/game/systems.hpp"
#include "sde/geometry_io.hpp"
#include "sde/logging.hpp"
#include "sde/resource_cache_io.hpp"
#include "sde/resource_io.hpp"
#include "sde/serial/std/filesystem.hpp"
#include "sde/serial/std/vector.hpp"
#include "sde/serialization_binary_file.hpp"
// #include "sde/view.hpp"

// RED
#include "red/background_music.hpp"
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

  std::vector<sde::game::ScriptRuntime::UPtr> scripts;
  scripts.push_back(createBackgroundMusic());
  scripts.push_back(createPlayerCharacter());
  scripts.push_back(createRenderer());
  scripts.push_back(createWeather());

  if (auto ifs_or_error = serial::file_istream::create("/tmp/game.bin"); ifs_or_error.has_value())
  {
    serial::binary_iarchive iar{*ifs_or_error};
    iar >> serial::named{"assets", sde::_R(assets)};
    if (auto ok_or_error = assets.refresh())
    {
      for (auto& script : scripts)
      {
        script->load(iar, assets);
      }
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

  app_or_error->spin([&](const auto& window) {
    assets.registry.view<Position, Dynamics>().each(
      [dt = toSeconds(window.time_delta)](Position& pos, const Dynamics& state) { pos.center += state.velocity * dt; });

    for (auto& script : scripts)
    {
      if (!script->update(systems, assets, window))
      {
        SDE_LOG_ERROR_FMT("script->update(...) failed: %s", script->identity().data());
        return AppDirective::kClose;
      }
    }
    return AppDirective::kContinue;
  });

  SDE_LOG_INFO("done.");

  if (auto ofs_or_error = serial::file_ostream::create("/tmp/game.bin"); ofs_or_error.has_value())
  {
    serial::binary_oarchive oar{*ofs_or_error};
    oar << serial::named{"assets", sde::_R(assets)};
    for (auto& script : scripts)
    {
      script->save(oar, assets);
    }
  }

  return 0;
}
