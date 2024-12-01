// C++ Standard Library
#include <cmath>

// SDE
#include "sde/app.hpp"
#include "sde/game/scene_graph.hpp"
#include "sde/logging.hpp"

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

  SDE_LOG_INFO() << "Starting...";

  // Create an application window
  auto app_or_error = App::create({.initial_size = {1000, 500}});
  SDE_ASSERT_OK(app_or_error);

  // Create scene graph from manifest
  auto game_or_error = sde::game::create(argv[1]);
  SDE_ASSERT_OK(game_or_error);

  // Run game
  app_or_error->spin(
    [&](const auto& app_properties) {
      if (const auto ok_or_error = game_or_error->initialize(app_properties); ok_or_error)
      {
        return AppDirective::kContinue;
      }
      else
      {
        SDE_LOG_ERROR() << ok_or_error.error();
      }
      return AppDirective::kClose;
    },
    [&](const auto& app_properties) {
      // assets.registry.view<Position, Dynamics>().each(
      //   [dt = toSeconds(app_properties.time_delta)](Position& pos, const Dynamics& state) {
      //     pos.center += state.velocity * dt;
      //   });

      if (const auto ok_or_error = game_or_error->tick(app_properties); ok_or_error)
      {
        return AppDirective::kContinue;
      }
      else
      {
        SDE_LOG_ERROR() << ok_or_error.error();
      }
      return AppDirective::kClose;
    });

  SDE_ASSERT_OK(sde::game::dump(*game_or_error, argv[1]));

  return 0;
}
