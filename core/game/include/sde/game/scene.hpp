/**
 * @copyright 2024-present Brian Cairl
 *
 * @file scene.hpp
 */
#pragma once

// C++ Standard Library
#include <optional>

// SDE
#include "sde/app_fwd.hpp"
#include "sde/expected.hpp"
#include "sde/format.hpp"
#include "sde/game/archive_fwd.hpp"
#include "sde/game/assets_fwd.hpp"
#include "sde/game/native_script.hpp"
#include "sde/game/native_script_handle.hpp"
#include "sde/game/scene_fwd.hpp"
#include "sde/game/scene_handle.hpp"
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"
#include "sde/string.hpp"
#include "sde/type_name.hpp"
#include "sde/vector.hpp"

namespace sde::game
{

enum class SceneError
{
  kInvalidHandle,
  kPathInvalid,
  kPathMissingFiles,
  kPathFailedToCreate,
  kFailedToSave,
  kFailedToLoad,
  kScriptAlreadyAdded,
  kComponentAlreadyAdded,
};

struct SceneScriptInstance : Resource<SceneScriptInstance>
{
  /// Handle to original script
  NativeScriptHandle handle;
  /// Current instance of the script
  NativeScriptInstance instance;
  /// Path to scripts load/save data
  std::optional<asset::path> instance_data_path;

  auto field_list()
  {
    return FieldList(
      Field{"handle", handle}, _Stub{"instance", instance}, Field{"instance_data_path", instance_data_path});
  }
};

struct SceneData : Resource<SceneData>
{
  /// Name associated with this scene
  sde::string name;
  /// Child scenes
  sde::vector<SceneHandle> children;
  /// Scripts to run before children
  sde::vector<SceneScriptInstance> pre_scripts;
  /// Scripts to run after children
  sde::vector<SceneScriptInstance> post_scripts;

  auto field_list()
  {
    return FieldList(
      Field{"children", children}, Field{"pre_scripts", pre_scripts}, Field{"post_scripts", post_scripts});
  }
};

}  // namespace sde::game

namespace sde
{

template <> struct Hasher<game::SceneScriptInstance> : ResourceHasher
{};

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
  explicit SceneCache(NativeScriptCache& scripts);

private:
  NativeScriptCache* scripts_;

  expected<void, SceneError> reload(SceneData& library);
  expected<void, SceneError> unload(SceneData& library);

  expected<SceneData, SceneError> generate(
    sde::string name,
    sde::vector<SceneScriptInstance>&& pre,
    sde::vector<SceneScriptInstance>&& post,
    sde::vector<SceneHandle>&& children);

  expected<SceneData, SceneError>
  generate(sde::string name, sde::vector<SceneScriptInstance> pre, sde::vector<SceneScriptInstance> post);

  expected<SceneData, SceneError> generate(sde::string name);
};

}  // namespace sde::game
