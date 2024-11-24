/**
 * @copyright 2024-present Brian Cairl
 *
 * @file scene_graph.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>

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

struct SceneGraphError
{
  SceneGraphErrorCode code;
  SceneHandle scene;
  const char* script_name;
};

std::ostream& operator<<(std::ostream& os, const SceneGraphError& error);

class SceneGraph : public Resource<SceneGraph>
{
  friend fundemental_type;

public:
  [[nodiscard]] expected<void, SceneGraphError> initialize(const AppProperties& properties) const;

  [[nodiscard]] expected<void, SceneGraphError> tick(const AppProperties& properties) const;

  [[nodiscard]] expected<void, SceneGraphError> load(const asset::path& directory);

  [[nodiscard]] expected<void, SceneGraphError> save(const asset::path& directory) const;

  [[nodiscard]] static expected<SceneGraph, SceneGraphErrorCode> create(Assets& assets, const SceneManifest& manifest);

private:
  explicit SceneGraph(Assets& assets);

  template <typename OnVisitT>
  [[nodiscard]] expected<void, SceneGraphError> visit(SceneHandle scene_handle, OnVisitT on_visit) const;

  Assets* assets_;
  SceneHandle root_;

  auto field_list() { return FieldList(Field{"root", root_}); }
};

}  // namespace sde::game
