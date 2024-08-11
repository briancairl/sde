// C++ Standard Library
#include <cmath>
#include <ostream>

// SDE
#include "sde/app.hpp"
#include "sde/game/scene.hpp"
#include "sde/logging.hpp"
// #include "sde/view.hpp"

// JSON
#include <nlohmann/json.hpp>

// RED
//#include "red/background_music.hpp"
//#include "red/components.hpp"
#include "red/components.hpp"
#include "red/drag_and_drop_asset_loader.hpp"
#include "red/imgui_end.hpp"
#include "red/imgui_start.hpp"
#include "red/player_character.hpp"
#include "red/renderer.hpp"
#include "red/texture_viewer.hpp"
#include "red/tile_map_editor.hpp"
#include "red/tile_set_editor.hpp"
// #include "red/weather.hpp"
// #include "red/world.hpp"


using namespace sde;

int main(int argc, char** argv)
{
  if (argc < 2)
  {
    SDE_LOG_ERROR_FMT("%s <dir>", argv[0]);
    return 1;
  }
  else
  {
    SDE_LOG_INFO_FMT("%s %s", argv[0], argv[1]);
  }

  game::ScriptRuntimeLoader::add("renderer", [](const auto& manifest) { return createRenderer(); });
  game::ScriptRuntimeLoader::add("imgui_start", [](const auto& manifest) { return _ImGuiStart(); });
  game::ScriptRuntimeLoader::add("imgui_end", [](const auto& manifest) { return _ImGuiEnd(); });
  game::ScriptRuntimeLoader::add("player_character", [](const auto& manifest) { return _PlayerCharacter(); });
  game::ScriptRuntimeLoader::add("tile_set_editor", [](const auto& manifest) { return _TileSetEditor(); });
  game::ScriptRuntimeLoader::add("tile_map_editor", [](const auto& manifest) { return _TileMapEditor(); });
  game::ScriptRuntimeLoader::add("texture_viewer", [](const auto& manifest) { return _TextureViewer(); });
  game::ScriptRuntimeLoader::add("drag_and_drop", [](const auto& manifest) { return _DragAndDropAssetLoader(); });

  SDE_LOG_INFO("starting...");

  auto app_or_error = App::create({.initial_size = {1000, 500}});
  SDE_ASSERT_TRUE(app_or_error.has_value());

  // game::Systems systems{game::Systems::create().value()};

  game::Scene scene;

  addComponentsToScene(scene);

  if (!scene.load(argv[1]))
  {
    scene.addScript("drag_and_drop", game::ScriptRuntimeLoader::load("drag_and_drop", {}));
    scene.addScript("renderer", game::ScriptRuntimeLoader::load("renderer", {}));
    scene.addScript("imgui_start", game::ScriptRuntimeLoader::load("imgui_start", {}));
    scene.addScript("player_character", game::ScriptRuntimeLoader::load("player_character", {}));
    scene.addScript("tile_set_editor", game::ScriptRuntimeLoader::load("tile_set_editor", {}));
    scene.addScript("tile_map_editor", game::ScriptRuntimeLoader::load("tile_map_editor", {}));
    scene.addScript("texture_viewer", game::ScriptRuntimeLoader::load("texture_viewer", {}));
    scene.addScript("imgui_end", game::ScriptRuntimeLoader::load("imgui_end", {}));
  }

  // scripts.emplace_back("bg_music", createBackgroundMusic());
  // scripts.emplace_back("player", createPlayerCharacter());
  // scripts.emplace_back("weather", createWeather());
  // // scripts.emplace_back("world", createWorld());

  // if (auto ifs_or_error = serial::file_istream::create("/tmp/assets.bin"); ifs_or_error.has_value())
  // {
  //   serial::binary_iarchive iar{*ifs_or_error};
  //   iar >> serial::named{"assets", sde::_R(assets)};
  //   SDE_ASSERT_OK(assets.refresh());
  // }

  // for (auto& [script_name, script] : scripts)
  // {
  //   if (auto ifs_or_error = serial::file_istream::create("/tmp/" + script_name + ".bin"); ifs_or_error.has_value())
  //   {
  //     serial::binary_iarchive iar{*ifs_or_error};
  //     script->load(iar, assets);
  //   }
  // }

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
    // assets.registry.view<Position, Dynamics>().each(
    //   [dt = toSeconds(app_properties.time_delta)](Position& pos, const Dynamics& state) {
    //     pos.center += state.velocity * dt;
    //   });

    if (scene.tick(app_state, app_properties))
    {
      return AppDirective::kContinue;
    }
    return AppDirective::kClose;
  });

  SDE_LOG_INFO("done.");


  SDE_ASSERT_OK(scene.save(argv[1]));

  // if (auto ofs_or_error = serial::file_ostream::create("/tmp/assets.bin"); ofs_or_error.has_value())
  // {
  //   serial::binary_oarchive oar{*ofs_or_error};
  //   oar << serial::named{"assets", sde::_R(assets)};
  // }

  // for (auto& [script_name, script] : scripts)
  // {
  //   if (auto ofs_or_error = serial::file_ostream::create("/tmp/" + script_name + ".bin"); ofs_or_error.has_value())
  //   {
  //     serial::binary_oarchive oar{*ofs_or_error};
  //     script->save(oar, assets);
  //   }
  // }

  return 0;
}
