// C++ Standard Library
#include <ostream>

// SDE
#include "sde/graphics/assets.hpp"
#include "sde/logging.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::graphics
{

std::ostream& operator<<(std::ostream& os, AssetError error)
{
  switch (error)
  {
    SDE_OSTREAM_ENUM_CASE(AssetError::kFailedImageLoading)
    SDE_OSTREAM_ENUM_CASE(AssetError::kFailedFontLoading)
    SDE_OSTREAM_ENUM_CASE(AssetError::kFailedShaderLoading)
    SDE_OSTREAM_ENUM_CASE(AssetError::kFailedTextureLoading)
    SDE_OSTREAM_ENUM_CASE(AssetError::kFailedTileSetLoading)
    SDE_OSTREAM_ENUM_CASE(AssetError::kFailedTypeSetLoading)
    SDE_OSTREAM_ENUM_CASE(AssetError::kFailedRenderTargetLoading)
  }
  return os;
}

Assets::Assets() : textures{images}, tile_sets{textures}, type_sets{textures, fonts}, render_targets{textures} {}

expected<void, AssetError> Assets::refresh()
{
  if (auto ok_or_error = images.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << "FailedImageLoading: " << ok_or_error.error();
    return make_unexpected(AssetError::kFailedImageLoading);
  }
  if (auto ok_or_error = fonts.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << "FailedFontLoading: " << ok_or_error.error();
    return make_unexpected(AssetError::kFailedFontLoading);
  }
  if (auto ok_or_error = shaders.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << "FailedShaderLoading: " << ok_or_error.error();
    return make_unexpected(AssetError::kFailedShaderLoading);
  }
  if (auto ok_or_error = textures.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << "FailedTextureLoading: " << ok_or_error.error();
    return make_unexpected(AssetError::kFailedTextureLoading);
  }
  if (auto ok_or_error = tile_sets.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << "FailedTileSetLoading: " << ok_or_error.error();
    return make_unexpected(AssetError::kFailedTileSetLoading);
  }
  if (auto ok_or_error = type_sets.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << "FailedTypeSetLoading: " << ok_or_error.error();
    return make_unexpected(AssetError::kFailedTypeSetLoading);
  }
  if (auto ok_or_error = render_targets.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << "FailedRenderTargetLoading: " << ok_or_error.error();
    return make_unexpected(AssetError::kFailedRenderTargetLoading);
  }
  return {};
}

void Assets::strip() { SDE_ASSERT_OK(images.relinquish()); }


}  // namespace sde::graphics
