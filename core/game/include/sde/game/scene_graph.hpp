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
#include "sde/type_name.hpp"

namespace sde::game
{

enum class SceneGraphErrorType
{
  kInvalidRoot,
  kPreScriptFailure,
  kPostScriptFailure,
};

struct SceneGraphError
{
  SceneGraphErrorType error_type;
  SceneHandle handle = SceneHandle::null();
};


class SceneGraph : public Resource<SceneGraph>
{
  friend fundemental_type;

public:
  expected<void, SceneGraphError> tick(Assets& assets, const AppProperties& properties) const;

  void setRoot(SceneHandle root) { root_ = root; }

private:
  static expected<void, SceneGraphError>
  tick(const SceneHandle handle, Assets& assets, const AppProperties& properties);

  SceneHandle root_;

  auto field_list() { return FieldList(Field{"root", root_}); }
};

}  // namespace sde::game
