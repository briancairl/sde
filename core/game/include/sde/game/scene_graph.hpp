/**
 * @copyright 2024-present Brian Cairl
 *
 * @file scene_graph.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>
#include <string_view>

// SDE
#include "sde/app_fwd.hpp"
#include "sde/game/assets.hpp"
#include "sde/game/native_script_fwd.hpp"
#include "sde/game/native_script_handle.hpp"
#include "sde/game/scene_fwd.hpp"
#include "sde/game/scene_handle.hpp"
#include "sde/game/scene_manifest_fwd.hpp"
#include "sde/resource.hpp"

namespace sde::game
{

enum class SceneGraphErrorCode
{
  kInvalidSceneManifest,
  kInvalidSceneCreation,
  kInvalidSceneRoot,
  kInvalidScript,
  kPreScriptFailure,
  kPostScriptFailure,
};

std::ostream& operator<<(std::ostream& os, SceneGraphErrorCode error);

struct SceneGraphError : Resource<SceneGraphError>
{
  /// Error type
  SceneGraphErrorCode code;
  /// Associated scene
  SceneHandle scene;
  /// Associated script
  std::string_view script_name;

  SceneGraphError(SceneGraphErrorCode _code, SceneHandle _scene, std::string_view _script_name) :
      code{_code}, scene{_scene}, script_name{_script_name}
  {}

  auto field_list() { return FieldList(Field{"code", code}, Field{"scene", scene}, Field{"script_name", script_name}); }
};

class SceneGraph : public Resource<SceneGraph>
{
  friend fundemental_type;

public:
  SceneGraph() = default;

  SceneGraph(SceneGraph&&) = default;
  SceneGraph& operator=(SceneGraph&&) = default;

  SceneGraph(const SceneGraph&) = delete;
  SceneGraph& operator=(const SceneGraph&) = delete;

  [[nodiscard]] expected<void, SceneGraphError> initialize(Assets& assets, const AppProperties& properties);

  [[nodiscard]] expected<void, SceneGraphError> tick(Assets& assets, const AppProperties& properties);

  [[nodiscard]] expected<void, SceneGraphError> load(Assets& assets, const asset::path& directory);

  [[nodiscard]] expected<void, SceneGraphError> save(Assets& assets, const asset::path& directory);

  [[nodiscard]] expected<SceneManifest, SceneGraphError> manifest(Assets& assets) const;

  [[nodiscard]] static expected<SceneGraph, SceneGraphErrorCode> create(Assets& assets, const SceneManifest& manifest);

private:
  template <typename OnVisitScriptT>
  [[nodiscard]] expected<void, SceneGraphError>
  visit(Assets& assets, SceneHandle scene_handle, OnVisitScriptT on_visit_script);

  template <typename OnVisitSceneT>
  [[nodiscard]] expected<void, SceneGraphError>
  visit_scene(Assets& assets, SceneHandle scene_handle, OnVisitSceneT on_visit_scene) const;

  SceneHandle root_;

  auto field_list() { return FieldList(Field{"root", root_}); }
};

}  // namespace sde::game
