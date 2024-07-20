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
#include "red/imgui.hpp"
#include "red/player_character.hpp"
#include "red/renderer.hpp"
#include "red/weather.hpp"
#include "red/world.hpp"


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

  std::vector<std::pair<std::string, sde::game::ScriptRuntime::UPtr>> scripts;
  scripts.emplace_back("imgui", createImGui());
  scripts.emplace_back("renderer", createRenderer());
  scripts.emplace_back("bg_music", createBackgroundMusic());
  scripts.emplace_back("player", createPlayerCharacter());
  scripts.emplace_back("weather", createWeather());
  // scripts.emplace_back("world", createWorld());

  if (auto ifs_or_error = serial::file_istream::create("/tmp/assets.bin"); ifs_or_error.has_value())
  {
    serial::binary_iarchive iar{*ifs_or_error};
    iar >> serial::named{"assets", sde::_R(assets)};
    SDE_ASSERT_OK(assets.refresh());
  }

  for (auto& [script_name, script] : scripts)
  {
    if (auto ifs_or_error = serial::file_istream::create("/tmp/" + script_name + ".bin"); ifs_or_error.has_value())
    {
      serial::binary_iarchive iar{*ifs_or_error};
      script->load(iar, assets);
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

  app_or_error->spin([&](auto& app_state, const auto& app_properties) {
    assets.registry.view<Position, Dynamics>().each(
      [dt = toSeconds(app_properties.time_delta)](Position& pos, const Dynamics& state) {
        pos.center += state.velocity * dt;
      });

    for (auto& [name, script] : scripts)
    {
      if (!script->update(systems, assets, app_state, app_properties))
      {
        SDE_LOG_ERROR_FMT("[%s]->update(...) failed: %s", name.c_str(), script->identity().data());
        return AppDirective::kClose;
      }
    }
    return AppDirective::kContinue;
  });

  SDE_LOG_INFO("done.");

  if (auto ofs_or_error = serial::file_ostream::create("/tmp/assets.bin"); ofs_or_error.has_value())
  {
    serial::binary_oarchive oar{*ofs_or_error};
    oar << serial::named{"assets", sde::_R(assets)};
  }

  for (auto& [script_name, script] : scripts)
  {
    if (auto ofs_or_error = serial::file_ostream::create("/tmp/" + script_name + ".bin"); ofs_or_error.has_value())
    {
      serial::binary_oarchive oar{*ofs_or_error};
      script->save(oar, assets);
    }
  }

  return 0;
}
