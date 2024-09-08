/**
 * @copyright 2024-present Brian Cairl
 *
 * @file scene.hpp
 */
#pragma once

// SDE
#include "sde/app_fwd.hpp"
#include "sde/expected.hpp"
#include "sde/format.hpp"
#include "sde/game/archive_fwd.hpp"
#include "sde/game/assets_fwd.hpp"
#include "sde/game/native_script_fwd.hpp"
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

enum class SceneType
{
  kRoot,
  kChild
};

struct SceneData : Resource<SceneData>
{
  /// Type of scene
  SceneType type;
  /// Name of scene
  sde::string name;
  /// Child scenes
  sde::vector<SceneHandle> children;
  /// Scripts to run before children
  sde::vector<NativeScriptHandle> pre_scripts;
  /// Scripts to run after children
  sde::vector<NativeScriptHandle> post_scripts;

  auto field_list()
  {
    return FieldList(
      Field{"type", type},
      Field{"name", name},
      Field{"children", children},
      Field{"pre_scripts", pre_scripts},
      Field{"post_scripts", post_scripts});
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
  explicit SceneCache(NativeScriptCache& scripts);

private:
  NativeScriptCache* scripts_;
  expected<void, SceneError> reload(SceneData& library);
  expected<void, SceneError> unload(SceneData& library);
  expected<SceneData, SceneError> generate(
    SceneType type,
    sde::string name,
    NativeScriptHandle pre,
    NativeScriptHandle post,
    sde::vector<SceneHandle> children = {});
  expected<SceneData, SceneError> generate(
    SceneType type,
    sde::string name,
    sde::vector<NativeScriptHandle> pre = {},
    sde::vector<NativeScriptHandle> post = {},
    sde::vector<SceneHandle> children = {});
};

}  // namespace sde::game
