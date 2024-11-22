// C++ Standard Library
#include <algorithm>
#include <fstream>

/// EnTT
#include <entt/entt.hpp>

// JSON
#include <nlohmann/json.hpp>

// SDE
#include "sde/game/scene.hpp"
#include "sde/logging.hpp"

#include "sde/audio/assets.hpp"
#include "sde/audio/mixer.hpp"
#include "sde/game/assets.hpp"
#include "sde/game/native_script.hpp"
#include "sde/game/scene.hpp"
#include "sde/geometry_io.hpp"
#include "sde/resource_cache_io.hpp"
#include "sde/resource_io.hpp"
#include "sde/serial/std/filesystem.hpp"
#include "sde/serial/std/vector.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::game
{
SceneCache::SceneCache(NativeScriptCache& scripts) : scripts_{std::addressof(scripts)} {}

expected<SceneData, SceneError> SceneCache::generate(
  SceneType type,
  sde::string name,
  sde::vector<std::pair<NativeScriptHandle, NativeScriptInstance>>&& pre,
  sde::vector<std::pair<NativeScriptHandle, NativeScriptInstance>>&& post,
  sde::vector<SceneHandle>&& children)
{
  SceneData data;
  data.type = type;
  data.name = std::move(name);
  data.children = std::move(children);
  data.pre_scripts = std::move(pre);
  data.post_scripts = std::move(post);
  if (const auto ok_or_error = reload(data); !ok_or_error.has_value())
  {
    return make_unexpected(ok_or_error.error());
  }
  return data;
}

expected<SceneData, SceneError> SceneCache::generate(
  SceneType type,
  sde::string name,
  sde::vector<std::pair<NativeScriptHandle, NativeScriptInstance>> pre,
  sde::vector<std::pair<NativeScriptHandle, NativeScriptInstance>> post)
{
  return this->generate(type, std::move(name), std::move(pre), std::move(post), sde::vector<SceneHandle>{});
}

expected<SceneData, SceneError> SceneCache::generate(SceneType type, sde::string name)
{
  return this->generate(
    type,
    std::move(name),
    sde::vector<std::pair<NativeScriptHandle, NativeScriptInstance>>{},
    sde::vector<std::pair<NativeScriptHandle, NativeScriptInstance>>{},
    sde::vector<SceneHandle>{});
}

expected<void, SceneError> SceneCache::reload(SceneData& library) { return {}; }

expected<void, SceneError> SceneCache::unload(SceneData& library) { return {}; }

}  // namespace sde::game
