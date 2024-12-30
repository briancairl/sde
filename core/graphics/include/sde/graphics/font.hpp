/**
 * @copyright 2024-present Brian Cairl
 *
 * @file font.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>

// SDE
#include "sde/asset.hpp"
#include "sde/expected.hpp"
#include "sde/graphics/font_fwd.hpp"
#include "sde/graphics/font_handle.hpp"
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"
#include "sde/unique_resource.hpp"

namespace sde::graphics
{

enum class FontError
{
  SDE_RESOURCE_CACHE_ERROR_ENUMS,
  kAssetNotFound,
  kAssetInvalid,
  kFontNotFound,
};

std::ostream& operator<<(std::ostream& os, FontError error);

struct FontNativeDeleter
{
  void operator()(void* font) const;
};

using FontNativeID = UniqueResource<void*, FontNativeDeleter>;

struct Font : Resource<Font>
{
  asset::path path = {};
  FontNativeID native_id = FontNativeID{nullptr};

  auto field_list() { return FieldList((Field{"path", path}), (_Stub{"native_id", native_id})); }
};

class FontCache : public ResourceCache<FontCache>
{
  friend fundemental_type;

private:
  static expected<void, FontError> reload(dependencies deps, Font& font);
  static expected<void, FontError> unload(dependencies deps, Font& font);
  expected<Font, FontError> generate(dependencies deps, const asset::path& font_path);
};

}  // namespace sde::graphics

namespace sde
{
template <> struct Hasher<graphics::Font> : ResourceHasher
{};
}  // namespace sde
