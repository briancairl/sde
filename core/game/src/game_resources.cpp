// SDE
#include "sde/game/game_resources.hpp"
#include "sde/logging.hpp"

namespace sde::game
{

GameResources::GameResources(asset::path root) : root_{std::move(root)}
{
  SDE_ASSERT_TRUE(asset::exists(root_)) << SDE_OSNV(root_) << " must exist";
  SDE_ASSERT_TRUE(asset::is_directory(root_)) << SDE_OSNV(root_) << " must be a directory";
}

}  // namespace sde::game
