// C++ Standard Library
#include <ostream>

/// EnTT
#include <entt/entt.hpp>

// SDE
#include "sde/game/assets.hpp"
#include "sde/logging.hpp"

namespace sde::game
{

Assets::Assets() :
    registry{}, components{libraries}, entities{registry, components}, scripts{libraries}, scenes{scripts}
{}

expected<void, AssetError> Assets::refresh()
{
  if (auto ok_or_error = components.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_DEBUG("FailedComponentsLoading");
    return make_unexpected(AssetError::kFailedComponentsLoading);
  }

  if (auto ok_or_error = entities.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_DEBUG("FailedEntitiesLoading");
    return make_unexpected(AssetError::kFailedEntitiesLoading);
  }

  if (auto ok_or_error = libraries.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_DEBUG("FailedLibraryLoading");
    return make_unexpected(AssetError::kFailedLibraryLoading);
  }

  if (auto ok_or_error = audio.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_DEBUG("FailedAudioLoading");
    return make_unexpected(AssetError::kFailedAudioLoading);
  }

  if (auto ok_or_error = graphics.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_DEBUG("FailedGraphicsLoading");
    return make_unexpected(AssetError::kFailedGraphicsLoading);
  }

  if (auto ok_or_error = scripts.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_DEBUG("FailedScriptsLoading");
    return make_unexpected(AssetError::kFailedScriptsLoading);
  }

  if (auto ok_or_error = scenes.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_DEBUG("FailedSceneLoading");
    return make_unexpected(AssetError::kFailedSceneLoading);
  }

  return {};
}

}  // namespace sde::game
