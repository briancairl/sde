/**
 * @copyright 2024-present Brian Cairl
 *
 * @file font.hpp
 */
#pragma once

// SDE
#include "sde/asset.hpp"
#include "sde/expected.hpp"
#include "sde/graphics/font_fwd.hpp"
#include "sde/graphics/font_handle.hpp"
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"
#include "sde/resource_wrapper.hpp"

namespace sde::graphics
{

enum class FontError
{
  kElementAlreadyExists,
  kInvalidHandle,
  kAssetNotFound,
  kAssetInvalid,
  kFontNotFound,
};

struct FontNativeDeleter
{
  void operator()(void* font) const;
};

using FontNativeID = UniqueResource<void*, FontNativeDeleter>;

struct Font : Resource<Font>
{
  asset::path path = {};
  FontNativeID native_id = FontNativeID{nullptr};

  auto field_list() { return std::make_tuple((Field{"path", path}), (_Stub{"native_id", native_id})); }
};

}  // namespace sde::graphics

namespace sde
{

template <> struct Hasher<graphics::Font> : ResourceHasher
{};

template <> struct ResourceCacheTypes<graphics::FontCache>
{
  using error_type = graphics::FontError;
  using handle_type = graphics::FontHandle;
  using value_type = graphics::Font;
};

}  // namespace sde

namespace sde::graphics
{

class FontCache : public ResourceCache<FontCache>
{
  friend fundemental_type;

private:
  static expected<void, FontError> reload(Font& font);
  static expected<void, FontError> unload(Font& font);
  expected<Font, FontError> generate(const asset::path& font_path);
};

}  // namespace sde::graphics
