// C++ Standard Library
#include <ostream>

// SDE
#include "sde/game/scene.hpp"
#include "sde/logging.hpp"


namespace sde::game
{

std::ostream& operator<<(std::ostream& os, SceneError error)
{
  switch (error)
  {
    SDE_OSTREAM_ENUM_CASE(SceneError::kInvalidHandle)
  }
  return os;
}

SceneCache::SceneCache(NativeScriptCache& scripts) : scripts_{std::addressof(scripts)} {}

expected<SceneData, SceneError> SceneCache::generate(
  sde::string name,
  sde::vector<SceneScriptInstance>&& pre,
  sde::vector<SceneScriptInstance>&& post,
  sde::vector<SceneHandle>&& children)
{
  SceneData data;
  data.name = std::move(name);
  data.children = std::move(children);
  data.pre_scripts = std::move(pre);
  data.post_scripts = std::move(post);
  if (const auto ok_or_error = reload(data); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << ok_or_error.error();
    return make_unexpected(ok_or_error.error());
  }
  return data;
}

expected<SceneData, SceneError>
SceneCache::generate(sde::string name, sde::vector<SceneScriptInstance> pre, sde::vector<SceneScriptInstance> post)
{
  return this->generate(std::move(name), std::move(pre), std::move(post), sde::vector<SceneHandle>{});
}

expected<SceneData, SceneError> SceneCache::generate(sde::string name)
{
  return this->generate(
    std::move(name),
    sde::vector<SceneScriptInstance>{},
    sde::vector<SceneScriptInstance>{},
    sde::vector<SceneHandle>{});
}

expected<void, SceneError> SceneCache::reload(SceneData& library) { return {}; }

expected<void, SceneError> SceneCache::unload(SceneData& library) { return {}; }

}  // namespace sde::game
