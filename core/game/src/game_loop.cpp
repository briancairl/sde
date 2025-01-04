// C++ Standard Library
#include <ostream>

// SDE
#include "sde/format.hpp"
#include "sde/game/game_loop.hpp"
#include "sde/game/game_resources.hpp"
#include "sde/game/scene.hpp"
#include "sde/logging.hpp"


namespace sde::game
{
namespace
{

asset::path getPath(const SceneNodeFlattened& node, const NativeScriptInstance& script)
{
  return {sde::format("%s_%s_%lu.bin", node.name.data(), script.type().data(), node.script.id())};
}

}  // namespace

GameLoop::GameLoop(SceneHandle handle, sde::vector<SceneNodeFlattened> nodes) :
    handle_{handle}, nodes_{std::move(nodes)}
{}

GameLoop::GameLoop(GameLoop&& other) { this->swap(other); }

GameLoop& GameLoop::operator=(GameLoop&& other)
{
  this->swap(other);
  return *this;
}

void GameLoop::swap(GameLoop& other)
{
  std::swap(this->handle_, other.handle_);
  std::swap(this->nodes_, other.nodes_);
}

expected<void, NativeScriptInstanceHandle> GameLoop::save(GameResources& resources, const asset::path& path) const
{
  for (const auto& node : nodes_)
  {
    if (auto script = resources(node.script); !script)
    {
      return make_unexpected(node.script);
    }
    else if (script->instance.save(path / getPath(node, script->instance)))
    {
      SDE_LOG_DEBUG() << "Saved data for " << SDE_OSNV(node.script) << " : " << SDE_OSNV(node.name) << " to "
                      << SDE_OSNV(path);
    }
    else
    {
      SDE_LOG_ERROR() << "Saving data for " << SDE_OSNV(node.script) << " : " << SDE_OSNV(node.name) << " to "
                      << SDE_OSNV(path) << " failed";
      return make_unexpected(node.script);
    }
  }
  return {};
}

expected<void, NativeScriptInstanceHandle> GameLoop::load(GameResources& resources, const asset::path& path) const
{
  for (const auto& node : nodes_)
  {
    if (auto script = resources(node.script); !script)
    {
      return make_unexpected(node.script);
    }
    else if (script->instance.load(path / getPath(node, script->instance)))
    {
      SDE_LOG_DEBUG() << "Loaded data for " << SDE_OSNV(node.script) << " : " << SDE_OSNV(node.name) << " from "
                      << SDE_OSNV(path);
    }
    else
    {
      SDE_LOG_ERROR() << "Loading data for " << SDE_OSNV(node.script) << " : " << SDE_OSNV(node.name) << " from "
                      << SDE_OSNV(path) << " failed";
      return make_unexpected(node.script);
    }
  }
  return {};
}

expected<void, NativeScriptInstanceHandle> GameLoop::initialize(GameResources& resources, const AppProperties& app)
{
  for (const auto& node : nodes_)
  {
    if (auto script = resources(node.script); !script)
    {
      return make_unexpected(node.script);
    }
    else if (!script->instance.initialize(node.script, node.name, resources, app))
    {
      SDE_LOG_ERROR() << SDE_OSNV(node.name) << SDE_OSNV(node.script) << " failed to initialize";
      return make_unexpected(node.script);
    }
  }
  return {};
}

expected<void, NativeScriptInstanceHandle> GameLoop::update(GameResources& resources, const AppProperties& app)
{
  for (const auto& node : nodes_)
  {
    if (auto script = resources(node.script); !script)
    {
      return make_unexpected(node.script);
    }
    else if (!script->instance.update(resources, app))
    {
      SDE_LOG_ERROR() << SDE_OSNV(node.name) << SDE_OSNV(node.script) << " failed to update";
      return make_unexpected(node.script);
    }
  }
  return {};
}

expected<void, NativeScriptInstanceHandle> GameLoop::shutdown(GameResources& resources, const AppProperties& app)
{
  for (const auto& node : nodes_)
  {
    if (auto script = resources(node.script); !script)
    {
      return make_unexpected(node.script);
    }
    else if (!script->instance.shutdown(resources, app))
    {
      SDE_LOG_ERROR() << SDE_OSNV(node.name) << SDE_OSNV(node.script) << " failed to shutdown";
      return make_unexpected(node.script);
    }
  }
  return {};
}

expected<GameLoop, SceneError> GameLoop::create(GameResources& resources, SceneHandle root)
{
  if (auto sequence_or_error = resources.get<SceneCache>().expand(root, resources.all()); sequence_or_error.has_value())
  {
    return {GameLoop{root, std::move(sequence_or_error).value()}};
  }
  else
  {
    return make_unexpected(sequence_or_error.error());
  }
}


}  // namespace sde::game
