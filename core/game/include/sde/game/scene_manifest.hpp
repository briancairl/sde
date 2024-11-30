/**
 * @copyright 2024-present Brian Cairl
 *
 * @file scene_graph.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>
#include <optional>

// SDE
#include "sde/asset.hpp"
#include "sde/expected.hpp"
#include "sde/resource.hpp"
#include "sde/string.hpp"
#include "sde/unordered_map.hpp"
#include "sde/vector.hpp"

namespace sde::game
{
enum class SceneManifestError
{
  kInvalidLoadJSONPath,
  kInvalidLoadJSONLayout,
  kInvalidSaveJSONPath,
  kFailedSaveJSON,
  kRootNotInSceneGraph,
  kSceneAlreadyInGraph,
};

std::ostream& operator<<(std::ostream& os, SceneManifestError error);

class SceneScriptData : public Resource<SceneScriptData>
{
  friend fundemental_type;
  friend class SceneManifest;
  friend class SceneManifestEntry;

public:
  asset::path path;

  std::optional<asset::path> data;

private:
  template <typename KeyValueArchiveT>
  [[nodiscard]] expected<void, SceneManifestError> load(const KeyValueArchiveT& kv_store);

  template <typename KeyValueArchiveT>
  [[nodiscard]] expected<void, SceneManifestError> save(KeyValueArchiveT& kv_store) const;

  auto field_list() { return FieldList(Field{"path", path}, Field{"data", data}); }
};

class SceneManifestEntry : public Resource<SceneManifestEntry>
{
  friend fundemental_type;
  friend class SceneManifest;

public:
  sde::vector<sde::string> children;

  sde::vector<SceneScriptData> pre_scripts;

  sde::vector<SceneScriptData> post_scripts;

private:
  template <typename KeyValueArchiveT>
  [[nodiscard]] expected<void, SceneManifestError> load(const KeyValueArchiveT& kv_store);

  template <typename KeyValueArchiveT>
  [[nodiscard]] expected<void, SceneManifestError> save(KeyValueArchiveT& kv_store) const;

  auto field_list()
  {
    return FieldList(
      Field{"children", children}, Field{"pre_scripts", pre_scripts}, Field{"post_scripts", post_scripts});
  }
};

class SceneManifest : public Resource<SceneManifest>
{
  friend fundemental_type;

public:
  [[nodiscard]] expected<void, SceneManifestError> load(const asset::path& path);

  [[nodiscard]] expected<void, SceneManifestError> save(const asset::path& path) const;

  [[nodiscard]] const auto& root() const { return root_; }

  [[nodiscard]] const auto& scenes() const { return scenes_; }

  [[nodiscard]] const auto* operator->() const { return std::addressof(scenes_); }

  [[nodiscard]] static expected<SceneManifest, SceneManifestError> create(const asset::path& path);

  [[nodiscard]] const auto& path() const { return path_; }

  void setPath(const asset::path& path);

  void setRoot(const sde::string& scene_name);

  expected<void, SceneManifestError> setScene(const sde::string& scene_name, SceneManifestEntry&& entry);

private:
  template <typename KeyValueArchiveT>
  [[nodiscard]] expected<void, SceneManifestError> save(KeyValueArchiveT& kv_store) const;

  template <typename KeyValueArchiveT>
  [[nodiscard]] expected<void, SceneManifestError> load(const KeyValueArchiveT& kv_store);

  asset::path path_;
  sde::string root_;
  sde::unordered_map<sde::string, SceneManifestEntry> scenes_;

  auto field_list() { return FieldList(Field{"path", path_}, Field{"root", root_}, Field{"scenes", scenes_}); }
};

}  // namespace sde::game
