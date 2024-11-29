/**
 * @copyright 2024-present Brian Cairl
 *
 * @file assets.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>

// SDE
#include "sde/asset.hpp"
#include "sde/audio/assets.hpp"
#include "sde/game/component.hpp"
#include "sde/game/entity.hpp"
#include "sde/game/library.hpp"
#include "sde/game/native_script.hpp"
#include "sde/game/registry.hpp"
#include "sde/game/scene.hpp"
#include "sde/graphics/assets.hpp"
#include "sde/memory.hpp"
#include "sde/resource.hpp"

namespace sde::game
{

enum class AssetError
{
  kFailedComponentsLoading,
  kFailedEntitiesLoading,
  kFailedLibraryLoading,
  kFailedAudioLoading,
  kFailedGraphicsLoading,
  kFailedScriptsLoading,
  kFailedSceneLoading,
};

/**
 * @brief Collection of active game assets
 */
class Assets : public Resource<Assets>
{
  friend fundemental_type;

public:
  /// Collection of active audio assets
  audio::Assets audio;

  /// Collection of graphics audio assets
  graphics::Assets graphics;

  /// Holds active game system/script data (passed between scripts)
  Registry registry;

  /// Holds information about components assigned to entities
  ComponentCache components;

  /// Holds absolute references to entities
  EntityCache entities;

  /// Holds runtime loaded libraries
  LibraryCache libraries;

  /// Holds scipt data
  NativeScriptCache scripts;

  /// Holds scene data
  SceneCache scenes;

  ~Assets();

  auto* operator->() { return std::addressof(registry.ctx()); }
  const auto* operator->() const { return std::addressof(registry.ctx()); }

  [[nodiscard]] Assets();

  [[nodiscard]] expected<void, AssetError> refresh();

  [[nodiscard]] auto assign(EntityHandle& handle) { return assignImpl(handle, entities); }

  template <typename... CreateArgTs> [[nodiscard]] auto assign(audio::SoundDataHandle& handle, CreateArgTs&&... args)
  {
    return assignImpl(handle, audio.sound_data, std::forward<CreateArgTs>(args)...);
  }

  template <typename... CreateArgTs> [[nodiscard]] auto assign(audio::SoundHandle& handle, CreateArgTs&&... args)
  {
    return assignImpl(handle, audio.sounds, std::forward<CreateArgTs>(args)...);
  }

  template <typename... CreateArgTs> [[nodiscard]] auto assign(graphics::ImageHandle& handle, CreateArgTs&&... args)
  {
    return assignImpl(handle, graphics.images, std::forward<CreateArgTs>(args)...);
  }

  template <typename... CreateArgTs> [[nodiscard]] auto assign(graphics::FontHandle& handle, CreateArgTs&&... args)
  {
    return assignImpl(handle, graphics.fonts, std::forward<CreateArgTs>(args)...);
  }

  template <typename... CreateArgTs> [[nodiscard]] auto assign(graphics::ShaderHandle& handle, CreateArgTs&&... args)
  {
    return assignImpl(handle, graphics.shaders, std::forward<CreateArgTs>(args)...);
  }

  template <typename... CreateArgTs> [[nodiscard]] auto assign(graphics::TextureHandle& handle, CreateArgTs&&... args)
  {
    return assignImpl(handle, graphics.textures, std::forward<CreateArgTs>(args)...);
  }

  template <typename... CreateArgTs> [[nodiscard]] auto assign(graphics::TileSetHandle& handle, CreateArgTs&&... args)
  {
    return assignImpl(handle, graphics.tile_sets, std::forward<CreateArgTs>(args)...);
  }

  template <typename... CreateArgTs> [[nodiscard]] auto assign(graphics::TypeSetHandle& handle, CreateArgTs&&... args)
  {
    return assignImpl(handle, graphics.type_sets, std::forward<CreateArgTs>(args)...);
  }

  template <typename... CreateArgTs>
  [[nodiscard]] auto assign(graphics::RenderTargetHandle& handle, CreateArgTs&&... args)
  {
    return assignImpl(handle, graphics.render_targets, std::forward<CreateArgTs>(args)...);
  }

private:
  auto field_list()
  {
    return FieldList(
      Field{"components", components},
      Field{"entities", entities},
      Field{"libraries", libraries},
      Field{"audio", audio},
      Field{"graphics", graphics},
      Field{"scripts", scripts},
      Field{"scenes", scenes});
  }

  template <typename CacheT, typename... CreateArgTs>
  [[nodiscard]] expected<ResourceStatus, typename CacheT::error_type> assignImpl(
    typename CacheT::handle_type& handle,
    ResourceCache<CacheT>& asset_cache,
    CreateArgTs&&... asset_create_args)
  {
    auto handle_and_value_or_error =
      asset_cache.find_or_create(handle, std::forward<CreateArgTs>(asset_create_args)...);
    if (handle_and_value_or_error.has_value())
    {
      handle = handle_and_value_or_error->handle;
      return handle_and_value_or_error->status;
    }
    return make_unexpected(handle_and_value_or_error.error());
  }
};

}  // namespace sde::game
