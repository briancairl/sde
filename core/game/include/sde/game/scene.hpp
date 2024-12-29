/**
 * @copyright 2024-present Brian Cairl
 *
 * @file scene.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>
#include <string_view>

// SDE
#include "sde/app_fwd.hpp"
#include "sde/expected.hpp"
#include "sde/format.hpp"
#include "sde/game/archive_fwd.hpp"
#include "sde/game/native_script_instance.hpp"
#include "sde/game/native_script_instance_handle.hpp"
#include "sde/game/scene_fwd.hpp"
#include "sde/game/scene_handle.hpp"
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"
#include "sde/string.hpp"
#include "sde/type_name.hpp"
#include "sde/unordered_map.hpp"
#include "sde/vector.hpp"

namespace sde::game
{

enum class SceneError
{
  kInvalidHandle,
  kInvalidScript,
  kElementAlreadyExists,
};

std::ostream& operator<<(std::ostream& os, SceneError error);

struct SceneNodeFlattened : Resource<SceneNodeFlattened>
{
  /// Instance name
  std::string_view name;

  /// Instance handle
  NativeScriptInstanceHandle handle = NativeScriptInstanceHandle::null();

  /// Instance
  NativeScriptInstance instance = {};

  auto field_list() { return FieldList(Field{"handle", handle}, _Stub{"instance", instance}); }
};

struct SceneNode : Resource<SceneNode>
{
  /// Handle to another scene
  SceneHandle child = SceneHandle::null();

  /// Handle to script instance data
  NativeScriptInstanceHandle script = NativeScriptInstanceHandle::null();

  auto field_list() { return FieldList(Field{"child", child}, Field{"script", script}); }
};

struct SceneData : Resource<SceneData>
{
  /// Name associated with this scene
  sde::string name;

  /// Scripts associated with this scene (run-ordered)
  sde::vector<SceneNode> nodes;

  auto field_list() { return FieldList(Field{"name", name}, Field{"nodes", nodes}); }
};

/**
 * @brief All active game data
 */
class SceneCache : public ResourceCache<SceneCache>
{
  friend fundemental_type;

public:
  using fundemental_type::to_handle;
  SceneHandle to_handle(const sde::string& name) const;
  expected<sde::vector<SceneNodeFlattened>, SceneError> expand(SceneHandle root, dependencies deps) const;

private:
  sde::unordered_map<sde::string, SceneHandle> name_to_scene_lookup_;
  expected<SceneData, SceneError> generate(dependencies deps, sde::string name, sde::vector<SceneNode> nodes = {});
  void when_created(dependencies deps, SceneHandle handle, const SceneData* data);
  void when_removed(dependencies deps, SceneHandle handle, SceneData* data);
};

}  // namespace sde::game

namespace sde
{
template <> struct Hasher<game::SceneNode> : ResourceHasher
{};
}  // namespace sde