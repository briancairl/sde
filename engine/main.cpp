// SDE
#include "sde/app.hpp"
#include "sde/game/game.hpp"
#include "sde/logging.hpp"

using namespace sde;
using namespace sde::game;

int main(int argc, char** argv)
{
  SDE_ASSERT_GT(argc, 1) << argv[0] << " <dir>";

  SDE_LOG_INFO() << "loading game data from: " << argv[1];

  // Create an application window
  auto app_or_error = App::create({.initial_size = {1000, 500}});
  SDE_ASSERT_OK(app_or_error);

  // Run game
  Game::create(argv[1]).spin(*app_or_error);
  return 0;
}
