// C++ Standard Library
#include <cmath>
#include <ostream>

// SDE
#include "sde/app.hpp"
#include "sde/game/assets.hpp"
#include "sde/game/scene_graph.hpp"
#include "sde/logging.hpp"
#include "sde/vector.hpp"
// #include "sde/view.hpp"

// JSON
#include <nlohmann/json.hpp>

// RED
//#include "red/background_music.hpp"
//#include "red/components.hpp"
// #include "red/audio_manager.hpp"
// #include "red/components.hpp"
// #include "red/drag_and_drop_asset_loader.hpp"
// #include "red/imgui_end.hpp"
// #include "red/imgui_start.hpp"
// #include "red/player_character.hpp"
// #include "red/renderer.hpp"
// #include "red/texture_viewer.hpp"
// #include "red/tile_map_editor.hpp"
// #include "red/tile_set_editor.hpp"
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

  // game::ScriptRuntimeLoader::add("audio_manager", [](const auto& manifest) { return _AudioManager(); });
  // game::ScriptRuntimeLoader::add("renderer", [](const auto& manifest) { return _Renderer(); });
  // game::ScriptRuntimeLoader::add("imgui_start", [](const auto& manifest) { return _ImGuiStart(); });
  // game::ScriptRuntimeLoader::add("imgui_end", [](const auto& manifest) { return _ImGuiEnd(); });
  // game::ScriptRuntimeLoader::add("player_character", [](const auto& manifest) { return _PlayerCharacter(); });
  // game::ScriptRuntimeLoader::add("tile_set_editor", [](const auto& manifest) { return _TileSetEditor(); });
  // game::ScriptRuntimeLoader::add("tile_map_editor", [](const auto& manifest) { return _TileMapEditor(); });
  // game::ScriptRuntimeLoader::add("texture_viewer", [](const auto& manifest) { return _TextureViewer(); });
  // game::ScriptRuntimeLoader::add("drag_and_drop", [](const auto& manifest) { return _DragAndDropAssetLoader(); });

  SDE_LOG_INFO("starting...");

  auto app_or_error = App::create({.initial_size = {1000, 500}});
  SDE_ASSERT_TRUE(app_or_error.has_value());

  // game::Scene scene;

  // addComponentsToScene(scene);

  // if (!asset::exists(argv[1]))
  // {
  //   scene.addScript("drag_and_drop", game::ScriptRuntimeLoader::load("drag_and_drop", {}));
  //   scene.addScript("renderer", game::ScriptRuntimeLoader::load("renderer", {}));
  //   scene.addScript("imgui_start", game::ScriptRuntimeLoader::load("imgui_start", {}));
  //   scene.addScript("audio_manager", game::ScriptRuntimeLoader::load("audio_manager", {}));
  //   scene.addScript("player_character", game::ScriptRuntimeLoader::load("player_character", {}));
  //   scene.addScript("tile_set_editor", game::ScriptRuntimeLoader::load("tile_set_editor", {}));
  //   scene.addScript("tile_map_editor", game::ScriptRuntimeLoader::load("tile_map_editor", {}));
  //   scene.addScript("texture_viewer", game::ScriptRuntimeLoader::load("texture_viewer", {}));
  //   scene.addScript("imgui_end", game::ScriptRuntimeLoader::load("imgui_end", {}));
  // }
  // else if (!scene.load(argv[1]))
  // {
  //   SDE_LOG_ERROR_FMT("Failed to load game data from: %s", argv[1]);
  //   return 1;
  // }

  game::Assets assets = {};
  // game::SceneGraph scene_graph = {};

  // const auto renderer_or_error = assets.scripts.create(asset::path{"engine/red/librenderer.so"});
  // if (!renderer_or_error.has_value())
  // {
  //   SDE_LOG_INFO("failed to load library: renderer");
  //   return 1;
  // }

  // const auto imgui_start_or_error = assets.scripts.create(asset::path{"engine/red/libimgui_start.so"});
  // if (!imgui_start_or_error.has_value())
  // {
  //   SDE_LOG_INFO("failed to load library: imgui_start");
  //   return 1;
  // }

  // const auto imgui_end_or_error = assets.scripts.create(asset::path{"engine/red/libimgui_end.so"});
  // if (!imgui_end_or_error.has_value())
  // {
  //   SDE_LOG_INFO("failed to load library: imgui_end");
  //   return 1;
  // }

  // const auto root_scene_or_error = assets.scenes.create(
  //   sde::make_vector(renderer_or_error->handle, imgui_start_or_error->handle),
  //   sde::make_vector(imgui_end_or_error->handle));
  // if (!root_scene_or_error.has_value())
  // {
  //   SDE_LOG_INFO("failed to create root scene");
  //   return 1;
  // }

  // for (const auto& de : std::filesystem::recursive_directory_iterator{"engine"})
  // {
  //   SDE_LOG_INFO((std::filesystem::current_path() / de.path()).string().c_str());
  // }

  // scene_graph.setRoot(root_scene_or_error->handle);

  auto scene_graph_or_error = sde::game::SceneGraph::load(assets, argv[1]);
  if (!scene_graph_or_error.has_value())
  {
    SDE_LOG_ERROR_FMT("Failed to load scene graph: %s", argv[1]);
    return 1;
  }

  app_or_error->spin([&](const auto& app_properties) {
    // assets.registry.view<Position, Dynamics>().each(
    //   [dt = toSeconds(app_properties.time_delta)](Position& pos, const Dynamics& state) {
    //     pos.center += state.velocity * dt;
    //   });

    if (const auto ok_or_error = scene_graph_or_error->tick(assets, app_properties); ok_or_error)
    {
      return AppDirective::kContinue;
    }
    else
    {
      SDE_LOG_INFO_FMT("%d", static_cast<int>(ok_or_error.error().error_type));
    }
    return AppDirective::kClose;
  });

  SDE_LOG_INFO("done.");


  // SDE_ASSERT_OK(scene_graph.save(argv[1]));

  return 0;
}
