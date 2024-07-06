/**
 * @copyright 2024-present Brian Cairl
 *
 * @file font.hpp
 */
#pragma once

// SDE
#include "sde/asset.hpp"
#include "sde/expected.hpp"
#include "sde/graphics/font_handle.hpp"
#include "sde/resource_cache.hpp"
#include "sde/resource_wrapper.hpp"

namespace sde::graphics
{

enum class FontError
{
  kElementAlreadyExists,
  kAssetNotFound,
  kAssetInvalid,
  kFontNotFound,
};

struct FontNativeDeleter
{
  void operator()(void* font) const;
};

using FontNativeID = UniqueResource<void*, FontNativeDeleter>;

struct FontInfo
{
  asset::path path;
  FontNativeID native_id;
};

class FontCache;

}  // namespace sde::graphics

namespace sde
{

template <> struct ResourceCacheTypes<graphics::FontCache>
{
  using error_type = graphics::FontError;
  using handle_type = graphics::FontHandle;
  using value_type = graphics::FontInfo;
};

}  // namespace sde

namespace sde::graphics
{

class FontCache : public ResourceCache<FontCache>
{
  friend cache_base;

private:
  static expected<void, FontError> reload(FontInfo& font);
  static expected<void, FontError> unload(FontInfo& font);
  expected<FontInfo, FontError>
  generate(const asset::path& font_path, ResourceLoading loading = ResourceLoading::kImmediate);
};

}  // namespace sde::graphics
