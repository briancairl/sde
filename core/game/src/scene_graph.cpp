// C++ Standard Library
#include <ostream>

// SDE
#include "sde/game/native_script.hpp"
#include "sde/game/scene.hpp"
#include "sde/game/scene_graph.hpp"
#include "sde/game/scene_handle.hpp"
#include "sde/logging.hpp"

namespace sde::game
{

expected<void, SceneGraphError> SceneGraph::tick(
  const SceneHandle scene_handle,
  const SceneCache& scenes,
  Assets& assets,
  const AppProperties& properties)
{
  const auto this_scene = scenes(scene_handle);

  SDE_ASSERT_TRUE(this_scene);

  // Call scripts which run before children
  for (const auto& script_handle : this_scene->pre_scripts)
  {
    const auto script = scenes.scripts()(script_handle);
    SDE_ASSERT_TRUE(script);
    if (const auto ok_or_error = script->script(assets, properties))
    {
      return make_unexpected(SceneGraphError{SceneGraphErrorType::kPreScriptFailure, scene_handle});
    }
  }

  // Run child scenes
  for (const auto& child_handle : this_scene->children)
  {
    if (const auto ok_or_error = SceneGraph::tick(child_handle, scenes, assets, properties); !ok_or_error.has_value())
    {
      return make_unexpected(ok_or_error.error());
    }
  }

  // Call scripts which run after children
  for (const auto& script_handle : this_scene->post_scripts)
  {
    const auto script = scenes.scripts()(script_handle);
    SDE_ASSERT_TRUE(script);
    if (!script->script(assets, properties))
    {
      return make_unexpected(SceneGraphError{SceneGraphErrorType::kPostScriptFailure, scene_handle});
    }
  }

  return {};
}

expected<void, SceneGraphError>
SceneGraph::tick(const SceneCache& scenes, Assets& assets, const AppProperties& properties) const
{
  SDE_ASSERT_TRUE(root_.isValid());
  return SceneGraph::tick(root_, scenes, assets, properties);
}

}  // namespace sde::game
