/**
 * @copyright 2024-present Brian Cairl
 *
 * @file assets.hpp
 */
#pragma once

// C++ Standard Library
#include <unordered_map>

// SDE
#include "sde/asset.hpp"
#include "sde/audio/assets.hpp"
#include "sde/game/systems_fwd.hpp"
#include "sde/graphics/assets.hpp"

namespace sde::game
{

enum class AssetError
{
  kFailedGraphicsLoading,
  kFailedAudioLoading,
};

/**
 * @brief Collection of active game assets
 */
class Assets
{
public:
  /// Collection of active audio assets
  audio::Assets audio;

  /// Collection of graphics audio assets
  graphics::Assets graphics;

  explicit Assets(Systems& systems);

  [[nodiscard]] expected<void, AssetError> refresh();

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
  template <typename CacheT, typename... CreateArgTs>
  [[nodiscard]] expected<void, typename CacheT::error_type> assignImpl(
    typename CacheT::handle_type& handle,
    ResourceCache<CacheT>& asset_cache,
    CreateArgTs&&... asset_create_args)
  {
    if (!handle.isNull() or asset_cache.exists(handle))
    {
      return {};
    }
    auto handle_and_value_or_error = asset_cache.create(std::forward<CreateArgTs>(asset_create_args)...);
    if (handle_and_value_or_error.has_value())
    {
      handle = handle_and_value_or_error->handle;
      return {};
    }
    return make_unexpected(handle_and_value_or_error.error());
  }
};

}  // namespace sde::game
