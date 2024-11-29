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
#include "sde/game/assets_fwd.hpp"
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
  kInvalidSceneCreation,
  kInvalidSceneRoot,
  kInvalidScript,
  kInvalidScriptData,
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

  SceneGraphError(SceneGraphErrorCode _code, SceneHandle _scene, const char* _script_name) :
      code{_code}, scene{_scene}, script_name{_script_name}
  {}

  auto field_list() { return FieldList(Field{"code", code}, Field{"scene", scene}, Field{"script_name", script_name}); }
};

class SceneGraph : public Resource<SceneGraph>
{
  friend fundemental_type;

public:
  [[nodiscard]] expected<void, SceneGraphError> initialize(const AppProperties& properties) const;

  [[nodiscard]] expected<void, SceneGraphError> tick(const AppProperties& properties) const;

  [[nodiscard]] expected<void, SceneGraphError> load(const asset::path& directory);

  [[nodiscard]] expected<void, SceneGraphError> save(const asset::path& directory) const;

  [[nodiscard]] expected<SceneManifest, SceneGraphError> manifest() const;

  [[nodiscard]] static expected<SceneGraph, SceneGraphErrorCode> create(Assets& assets, const SceneManifest& manifest);

private:
  explicit SceneGraph(Assets& assets);

  template <typename OnVisitScriptT>
  [[nodiscard]] expected<void, SceneGraphError> visit(SceneHandle scene_handle, OnVisitScriptT on_visit_script) const;

  template <typename OnVisitSceneT>
  [[nodiscard]] expected<void, SceneGraphError>
  visit_scene(SceneHandle scene_handle, OnVisitSceneT on_visit_scene) const;

  Assets* assets_;
  SceneHandle root_;

  auto field_list() { return FieldList(Field{"root", root_}); }
};

}  // namespace sde::game
