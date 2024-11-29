// C++ Standard Library
#include <ostream>

/// EnTT
#include <entt/entt.hpp>

// SDE
#include "sde/game/assets.hpp"
#include "sde/logging.hpp"

namespace sde::game
{

std::ostream& operator<<(std::ostream& os, AssetError error)
{
  switch (error)
  {
    SDE_OS_ENUM_CASE(AssetError::kFailedComponentsLoading)
    SDE_OS_ENUM_CASE(AssetError::kFailedEntitiesLoading)
    SDE_OS_ENUM_CASE(AssetError::kFailedLibraryLoading)
    SDE_OS_ENUM_CASE(AssetError::kFailedAudioLoading)
    SDE_OS_ENUM_CASE(AssetError::kFailedGraphicsLoading)
    SDE_OS_ENUM_CASE(AssetError::kFailedScriptsLoading)
    SDE_OS_ENUM_CASE(AssetError::kFailedSceneLoading)
  }
  return os;
}

Assets::Assets() :
    registry{}, components{libraries}, entities{registry, components}, scripts{libraries}, scenes{scripts}
{}

Assets::~Assets() = default;

expected<void, AssetError> Assets::refresh()
{
  if (auto ok_or_error = components.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << "FailedComponentsLoading: " << ok_or_error.error();
    return make_unexpected(AssetError::kFailedComponentsLoading);
  }

  if (auto ok_or_error = entities.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << "FailedEntitiesLoading: " << ok_or_error.error();
    return make_unexpected(AssetError::kFailedEntitiesLoading);
  }

  if (auto ok_or_error = libraries.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << "FailedLibraryLoading: " << ok_or_error.error();
    return make_unexpected(AssetError::kFailedLibraryLoading);
  }

  if (auto ok_or_error = audio.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << "FailedAudioLoading: " << ok_or_error.error();
    return make_unexpected(AssetError::kFailedAudioLoading);
  }

  if (auto ok_or_error = graphics.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << "FailedGraphicsLoading: " << ok_or_error.error();
    return make_unexpected(AssetError::kFailedGraphicsLoading);
  }

  if (auto ok_or_error = scripts.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << "FailedScriptsLoading: " << ok_or_error.error();
    return make_unexpected(AssetError::kFailedScriptsLoading);
  }

  if (auto ok_or_error = scenes.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << "FailedSceneLoading: " << ok_or_error.error();
    return make_unexpected(AssetError::kFailedSceneLoading);
  }

  return {};
}

}  // namespace sde::game
