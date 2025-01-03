// C++ Standard Library
#include <ostream>

// SDE
#include "sde/format.hpp"
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

asset::path getPath(const SceneNodeFlattened& node)
{
  return {sde::format("%s_%s_%lu.bin", node.name.data(), node.instance.type().data(), node.handle.id())};
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

expected<Scene, SceneError> SceneCache::expand(SceneHandle root, dependencies deps) const
{
  sde::vector<SceneNodeFlattened> sequence;
  if (auto ok_or_error = expand_recursive(sequence, *this, deps, root); !ok_or_error.has_value())
  {
    return make_unexpected(ok_or_error.error());
  }
  return {Scene{root, std::move(sequence)}};
}

expected<SceneData, SceneError> SceneCache::generate(dependencies deps, sde::string name, sde::vector<SceneNode> nodes)
{
  SceneData data;
  data.name = std::move(name);
  data.nodes = std::move(nodes);
  return data;
}

bool SceneCache::when_created([[maybe_unused]] dependencies deps, SceneHandle handle, const SceneData* data)
{
  if (const auto [itr, added] = name_to_scene_lookup_.emplace(data->name, handle); !added and (itr->second != handle))
  {
    SDE_LOG_ERROR() << "Scene " << SDE_OSNV(itr->first) << " was already added as " << SDE_OSNV(itr->second)
                    << ". Want: " << SDE_OSNV(handle);
    return false;
  }
  return true;
}

bool SceneCache::when_removed([[maybe_unused]] dependencies deps, SceneHandle handle, SceneData* data)
{
  name_to_scene_lookup_.erase(data->name);
  return true;
}

Scene::Scene(SceneHandle handle, sde::vector<SceneNodeFlattened> nodes) : handle_{handle}, nodes_{std::move(nodes)} {}

Scene::Scene(Scene&& other) { this->swap(other); }

Scene& Scene::operator=(Scene&& other)
{
  this->swap(other);
  return *this;
}

void Scene::swap(Scene& other)
{
  std::swap(this->handle_, other.handle_);
  std::swap(this->nodes_, other.nodes_);
}

expected<void, NativeScriptInstanceHandle> Scene::save(const asset::path& path) const
{
  for (const auto& node : nodes_)
  {
    if (node.instance.save(path / getPath(node)))
    {
      SDE_LOG_DEBUG() << "Saved data for " << SDE_OSNV(node.handle) << " : " << SDE_OSNV(node.name) << " to "
                      << SDE_OSNV(path);
    }
    else
    {
      SDE_LOG_ERROR() << "Saving data for " << SDE_OSNV(node.handle) << " : " << SDE_OSNV(node.name) << " to "
                      << SDE_OSNV(path) << " failed";
      return make_unexpected(node.handle);
    }
  }
  return {};
}

expected<void, NativeScriptInstanceHandle> Scene::load(const asset::path& path) const
{
  for (const auto& node : nodes_)
  {
    if (node.instance.load(path / getPath(node)))
    {
      SDE_LOG_DEBUG() << "Loaded data for " << SDE_OSNV(node.handle) << " : " << SDE_OSNV(node.name) << " from "
                      << SDE_OSNV(path);
    }
    else
    {
      SDE_LOG_ERROR() << "Loading data for " << SDE_OSNV(node.handle) << " : " << SDE_OSNV(node.name) << " from "
                      << SDE_OSNV(path) << " failed";
      return make_unexpected(node.handle);
    }
  }
  return {};
}

expected<void, NativeScriptInstanceHandle> Scene::initialize(GameResources& resources, const AppProperties& app)
{
  for (const auto& node : nodes_)
  {
    if (!node.instance.initialize(node.handle, node.name, resources, app))
    {
      SDE_LOG_ERROR() << SDE_OSNV(node.name) << SDE_OSNV(node.handle) << " failed to initialize";
      return make_unexpected(node.handle);
    }
  }
  return {};
}

expected<void, NativeScriptInstanceHandle> Scene::update(GameResources& resources, const AppProperties& app)
{
  for (const auto& node : nodes_)
  {
    if (!node.instance.update(resources, app))
    {
      SDE_LOG_ERROR() << SDE_OSNV(node.name) << SDE_OSNV(node.handle) << " failed to update";
      return make_unexpected(node.handle);
    }
  }
  return {};
}

expected<void, NativeScriptInstanceHandle> Scene::shutdown(GameResources& resources, const AppProperties& app)
{
  for (const auto& node : nodes_)
  {
    if (!node.instance.shutdown(resources, app))
    {
      SDE_LOG_ERROR() << SDE_OSNV(node.name) << SDE_OSNV(node.handle) << " failed to shutdown";
      return make_unexpected(node.handle);
    }
  }
  return {};
}


}  // namespace sde::game
