/**
 * @copyright 2025-present Brian Cairl
 *
 * @file game_loop.hpp
 */
#pragma once

// SDE
#include "sde/app_fwd.hpp"
#include "sde/asset.hpp"
#include "sde/expected.hpp"
#include "sde/game/game_resources_fwd.hpp"
#include "sde/game/scene.hpp"

namespace sde::game
{

class GameLoop
{
public:
  GameLoop() = default;
  GameLoop(SceneHandle handle, sde::vector<SceneNodeFlattened> nodes);

  GameLoop(GameLoop&& other);
  GameLoop& operator=(GameLoop&& other);

  void swap(GameLoop& other);

  const SceneHandle& handle() const { return handle_; }

  static expected<GameLoop, SceneError> create(GameResources& resources, SceneHandle root);

  expected<void, NativeScriptInstanceHandle> load(GameResources& resources, const asset::path& path) const;
  expected<void, NativeScriptInstanceHandle> save(GameResources& resources, const asset::path& path) const;
  expected<void, NativeScriptInstanceHandle> update(GameResources& resources, const AppProperties& app);
  expected<void, NativeScriptInstanceHandle> initialize(GameResources& resources, const AppProperties& app);
  expected<void, NativeScriptInstanceHandle> shutdown(GameResources& resources, const AppProperties& app);

private:
  GameLoop(const GameLoop& other) = delete;
  GameLoop& operator=(const GameLoop& other) = delete;

  SceneHandle handle_ = SceneHandle::null();
  sde::vector<SceneNodeFlattened> nodes_ = {};
};

}  // namespace sde::game
