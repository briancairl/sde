// C++ Standard Library
#include <ostream>

// SDE
#include "sde/graphics/assets.hpp"
#include "sde/logging.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::graphics
{

Assets::Assets() : textures{images}, tile_sets{textures}, type_sets{textures, fonts}, render_targets{textures} {}

expected<void, AssetError> Assets::refresh()
{
  if (auto ok_or_error = images.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_DEBUG("FailedImageLoading");
    return make_unexpected(AssetError::kFailedImageLoading);
  }
  if (auto ok_or_error = fonts.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_DEBUG("FailedFontLoading");
    return make_unexpected(AssetError::kFailedFontLoading);
  }
  if (auto ok_or_error = shaders.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_DEBUG("FailedShaderLoading");
    return make_unexpected(AssetError::kFailedShaderLoading);
  }
  if (auto ok_or_error = textures.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_DEBUG("FailedTextureLoading");
    return make_unexpected(AssetError::kFailedTextureLoading);
  }
  if (auto ok_or_error = tile_sets.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_DEBUG("FailedTileSetLoading");
    return make_unexpected(AssetError::kFailedTileSetLoading);
  }
  if (auto ok_or_error = type_sets.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_DEBUG("FailedTypeSetLoading");
    return make_unexpected(AssetError::kFailedTypeSetLoading);
  }
  if (auto ok_or_error = render_targets.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_DEBUG("FailedRenderTargetLoading");
    return make_unexpected(AssetError::kFailedRenderTargetLoading);
  }
  return {};
}

void Assets::strip() { SDE_ASSERT_OK(images.relinquish()); }


}  // namespace sde::graphics
