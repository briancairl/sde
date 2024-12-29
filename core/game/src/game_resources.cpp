// SDE
#include "sde/game/game_resources.hpp"
#include "sde/logging.hpp"

namespace sde::game
{

GameResources::GameResources(asset::path root) : root_path_{std::move(root)}
{
  SDE_ASSERT_TRUE(asset::exists(root_path_)) << SDE_OSNV(root_path_) << " must exist";
  SDE_ASSERT_TRUE(asset::is_directory(root_path_)) << SDE_OSNV(root_path_) << " must be a directory";
}

asset::path GameResources::directory(const asset::path& original_path) const
{
  if (original_path.empty())
  {
    return root_path_;
  }
  asset::path resolved_path = original_path.is_absolute() ? original_path : (root_path_ / original_path);
  if (!asset::exists(resolved_path) and !asset::create_directories(resolved_path))
  {
    SDE_FAIL() << "Failed to create path: " << SDE_OSNV(resolved_path);
  }
  return resolved_path;
}

asset::path GameResources::path(const asset::path& original_path) const
{
  return directory(original_path.parent_path()) / original_path.filename();
}

bool GameResources::setNextScene(const SceneHandle scene)
{
  if (this->get<SceneCache>().exists(scene))
  {
    next_scene_ = scene;
    return true;
  }
  return false;
}

bool GameResources::setNextScene(const sde::string& scene_name)
{
  const auto scene = this->get<SceneCache>().to_handle(scene_name);
  return scene and setNextScene(scene);
}

}  // namespace sde::game
