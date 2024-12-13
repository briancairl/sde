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
    SDE_OS_ENUM_CASE(AssetError::kFailedImageLoading)
    SDE_OS_ENUM_CASE(AssetError::kFailedFontLoading)
    SDE_OS_ENUM_CASE(AssetError::kFailedShaderLoading)
    SDE_OS_ENUM_CASE(AssetError::kFailedTextureLoading)
    SDE_OS_ENUM_CASE(AssetError::kFailedTileSetLoading)
    SDE_OS_ENUM_CASE(AssetError::kFailedTypeSetLoading)
    SDE_OS_ENUM_CASE(AssetError::kFailedRenderTargetLoading)
  }
  return os;
}

expected<void, AssetError> Assets::refresh()
{
  if (auto ok_or_error = get<ImageCache>().refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << "FailedImageLoading: " << ok_or_error.error();
    return make_unexpected(AssetError::kFailedImageLoading);
  }
  if (auto ok_or_error = get<FontCache>().refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << "FailedFontLoading: " << ok_or_error.error();
    return make_unexpected(AssetError::kFailedFontLoading);
  }
  if (auto ok_or_error = get<ShaderCache>().refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << "FailedShaderLoading: " << ok_or_error.error();
    return make_unexpected(AssetError::kFailedShaderLoading);
  }
  if (auto ok_or_error = get<TextureCache>().refresh(all()); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << "FailedTextureLoading: " << ok_or_error.error();
    return make_unexpected(AssetError::kFailedTextureLoading);
  }
  if (auto ok_or_error = get<TileSetCache>().refresh(all()); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << "FailedTileSetLoading: " << ok_or_error.error();
    return make_unexpected(AssetError::kFailedTileSetLoading);
  }
  if (auto ok_or_error = get<TypeSetCache>().refresh(all()); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << "FailedTypeSetLoading: " << ok_or_error.error();
    return make_unexpected(AssetError::kFailedTypeSetLoading);
  }
  if (auto ok_or_error = get<RenderTargetCache>().refresh(all()); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << "FailedRenderTargetLoading: " << ok_or_error.error();
    return make_unexpected(AssetError::kFailedRenderTargetLoading);
  }
  return {};
}

void Assets::strip() { SDE_ASSERT_OK(get<ImageCache>().relinquish()); }


}  // namespace sde::graphics
