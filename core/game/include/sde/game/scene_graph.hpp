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
  kInvalidScriptData,
  kInvalidScriptDataPath,
  kInvalidScriptAssets,
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
  [[nodiscard]] expected<void, SceneGraphError> initialize(const AppProperties& properties);

  [[nodiscard]] expected<void, SceneGraphError> tick(const AppProperties& properties);

  [[nodiscard]] expected<void, SceneGraphError> load(const asset::path& directory);

  [[nodiscard]] expected<void, SceneGraphError> save(const asset::path& directory);

  [[nodiscard]] expected<SceneManifest, SceneGraphError> manifest() const;

  [[nodiscard]] static expected<SceneGraph, SceneGraphErrorCode>
  create(const SceneManifest& manifest, sde::unique_ptr<Assets>&& assets);

  [[nodiscard]] static expected<SceneGraph, SceneGraphErrorCode> create(const SceneManifest& manifest);

  [[nodiscard]] static expected<SceneGraph, SceneGraphErrorCode> create(const asset::path& manifest_path);

  [[nodiscard]] static expected<void, SceneGraphErrorCode> dump(SceneGraph& graph, const asset::path& manifest_path);

  [[nodiscard]] const asset::path& path() const { return path_; }

  [[nodiscard]] const asset::path assetPath() const { return path() / "assets.bin"; }

private:
  explicit SceneGraph(sde::unique_ptr<Assets>&& assets);

  template <typename OnVisitScriptT>
  [[nodiscard]] expected<void, SceneGraphError> visit(SceneHandle scene_handle, OnVisitScriptT on_visit_script);

  template <typename OnVisitSceneT>
  [[nodiscard]] expected<void, SceneGraphError>
  visit_scene(SceneHandle scene_handle, OnVisitSceneT on_visit_scene) const;

  sde::unique_ptr<Assets> assets_;
  SceneHandle root_;
  asset::path path_;

  auto field_list() { return FieldList(Field{"root", root_}); }
};

}  // namespace sde::game
