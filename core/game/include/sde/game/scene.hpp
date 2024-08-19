/**
 * @copyright 2024-present Brian Cairl
 *
 * @file scene.hpp
 */
#pragma once

// C++ Standard Library
#include <vector>

// SDE
#include "sde/app_fwd.hpp"
#include "sde/format.hpp"
#include "sde/game/archive_fwd.hpp"
#include "sde/game/assets.hpp"
#include "sde/game/script_runtime_fwd.hpp"
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"
#include "sde/type_name.hpp"

namespace sde::game
{

enum class SceneError
{
  kPathInvalid,
  kPathMissingFiles,
  kPathFailedToCreate,
  kFailedToSave,
  kFailedToLoad,
  kScriptAlreadyAdded,
  kComponentAlreadyAdded,
};

struct SceneData : Resource<SceneData>
{
  /// Child scenes
  std::vector<SceneHandle> children;
  /// Scripts to run before children
  std::vector<ScriptHandle> pre_scripts;
  /// Scripts to run after children
  std::vector<ScriptHandle> post_scripts;

  auto field_list()
  {
    return FieldList(
      Field{"children", children}, Field{"pre_scripts", pre_scripts}, Field{"post_scripts", post_scripts});
  }
};

}  // namespace sde::game

namespace sde
{

template <> struct ResourceCacheTypes<game::SceneCache>
{
  using error_type = game::SceneError;
  using handle_type = game::SceneHandle;
  using value_type = game::SceneData;
};

}  // namespace sde

namespace sde::game
{

/**
 * @brief All active game data
 */
class SceneCache : public ResourceCache<SceneCache>
{
  friend fundemental_type;

public:
  explicit SceneCache(NativeScriptCache& libraries);

  expected<void, SceneError> save(const asset::path& path) const;

  expected<void, SceneError> load(const asset::path& path);

  bool tick(AppState& state, const AppProperties& properties);

private:
  NativeScriptCache* scripts_;
  expected<void, SceneError> reload(NativeScriptData& library);
  expected<void, SceneError> unload(NativeScriptData& library);
  expected<NativeScriptData, SceneError> generate(const asset::path& path);
  expected<NativeScriptData, SceneError> generate(LibraryHandle library);
};

}  // namespace sde::game
