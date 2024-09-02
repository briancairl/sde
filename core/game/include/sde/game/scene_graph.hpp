/**
 * @copyright 2024-present Brian Cairl
 *
 * @file scene_graph.hpp
 */
#pragma once

// SDE
#include "sde/app_fwd.hpp"
#include "sde/game/assets_fwd.hpp"
#include "sde/game/native_script_fwd.hpp"
#include "sde/game/native_script_handle.hpp"
#include "sde/game/scene_fwd.hpp"
#include "sde/game/scene_handle.hpp"
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"
#include "sde/type_name.hpp"

namespace sde::game
{

enum class SceneGraphErrorType
{
  kPreScriptFailure,
  kPostScriptFailure,
};

struct SceneGraphError
{
  SceneGraphErrorType error_type;
  SceneHandle handle;
};


class SceneGraph : public ResourceCache<SceneCache>
{
  friend fundemental_type;

public:
  expected<void, SceneGraphError> tick(const SceneCache& scenes, Assets& assets, const AppProperties& properties) const;

private:
  static expected<void, SceneGraphError>
  tick(const SceneHandle handle, const SceneCache& scenes, Assets& assets, const AppProperties& properties);

  SceneHandle root_;

  auto field_list() { return FieldList(Field{"root", root_}); }
};

}  // namespace sde::game
