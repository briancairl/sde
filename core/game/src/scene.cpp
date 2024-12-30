// C++ Standard Library
#include <ostream>

// SDE
#include "sde/game/scene.hpp"
#include "sde/logging.hpp"


namespace sde::game
{
namespace
{

expected<void, SceneError> expand_recursive(
  sde::vector<SceneNodeFlattened>& sequence,
  const SceneCache& scenes,
  SceneCache::dependencies deps,
  SceneHandle root)
{
  if (!root)
  {
    return {};
  }
  auto scene = scenes(root);
  if (!scene)
  {
    SDE_LOG_ERROR() << SDE_OSNV(root) << " is not a valid scene handle";
    return make_unexpected(SceneError::kInvalidHandle);
  }
  for (const auto& [child_handle, script_handle] : scene->nodes)
  {
    if (script_handle)
    {
      SDE_LOG_INFO() << SDE_OSNV(root) << " --> " << SDE_OSNV(script_handle);
      if (auto script = deps(script_handle))
      {
        sequence.push_back({.name = script->name, .handle = script_handle, .instance = script->instance});
      }
      else
      {
        SDE_LOG_ERROR() << SDE_OSNV(script_handle) << " is not a valid script instance handle";
        return make_unexpected(SceneError::kInvalidScript);
      }
    }

    if (child_handle)
    {
      SDE_LOG_INFO() << SDE_OSNV(root) << " --> " << SDE_OSNV(child_handle);
      if (auto ok_or_error = expand_recursive(sequence, scenes, deps, child_handle); !ok_or_error.has_value())
      {
        return make_unexpected(ok_or_error.error());
      }
    }
  }
  return {};
}

}  // namespace

std::ostream& operator<<(std::ostream& os, SceneError error)
{
  switch (error)
  {
    SDE_OS_ENUM_CASES_FOR_RESOURCE_CACHE_ERRORS(SceneError)
    SDE_OS_ENUM_CASE(SceneError::kInvalidScript)
  }
  return os;
}

SceneHandle SceneCache::to_handle(const sde::string& name) const
{
  const auto itr = name_to_scene_lookup_.find(name);
  if (itr == std::end(name_to_scene_lookup_))
  {
    return SceneHandle::null();
  }
  return itr->second;
}

expected<sde::vector<SceneNodeFlattened>, SceneError> SceneCache::expand(SceneHandle root, dependencies deps) const
{
  sde::vector<SceneNodeFlattened> sequence;
  if (auto ok_or_error = expand_recursive(sequence, *this, deps, root); !ok_or_error.has_value())
  {
    return make_unexpected(ok_or_error.error());
  }
  return {std::move(sequence)};
}

expected<SceneData, SceneError> SceneCache::generate(dependencies deps, sde::string name, sde::vector<SceneNode> nodes)
{
  SceneData data;
  data.name = std::move(name);
  data.nodes = std::move(nodes);
  return data;
}

void SceneCache::when_created([[maybe_unused]] dependencies deps, SceneHandle handle, const SceneData* data)
{
  const auto [itr, added] = name_to_scene_lookup_.emplace(data->name, handle);
  SDE_ASSERT_TRUE(added) << "Script " << data->name << " was already added as " << itr->first;
}

void SceneCache::when_removed([[maybe_unused]] dependencies deps, SceneHandle handle, SceneData* data)
{
  name_to_scene_lookup_.erase(data->name);
}

}  // namespace sde::game
