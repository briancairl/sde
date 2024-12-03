// SDE
#include "sde/app.hpp"
#include "sde/game/game.hpp"
#include "sde/logging.hpp"

using namespace sde;

int main(int argc, char** argv)
{
  SDE_ASSERT_GT(argc, 1) << argv[0] << " <dir>";

  SDE_LOG_INFO() << "loading game data from: " << argv[1];

  // Create an application window
  auto app_or_error = App::create({.initial_size = {1000, 500}});
  SDE_ASSERT_OK(app_or_error);

  // Create scene graph from manifest
  auto game_or_error = game::create(argv[1]);
  SDE_ASSERT_OK(game_or_error);

  // Run game
  game_or_error->spin(*app_or_error);

  // Save game on close
  SDE_ASSERT_OK(game::dump(*game_or_error, argv[1]));
  SDE_LOG_INFO() << "saved game data to: " << argv[1];
  return 0;
}
