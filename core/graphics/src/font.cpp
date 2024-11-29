// C++ Standard Library
#include <array>
#include <numeric>
#include <ostream>
#include <type_traits>
#include <vector>

// FreeType
#include <ft2build.h>
#include FT_FREETYPE_H

// SDE
#include "sde/graphics/font.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/logging.hpp"

namespace sde::graphics
{
namespace
{

constexpr int kFreeTypeSuccess = 0;

struct FreeTypeRelease
{
  void operator()(const FT_Library& library) const { FT_Done_FreeType(library); }
};

UniqueResource<FT_Library, FreeTypeRelease> FreeType{[] {
  FT_Library ft;
  SDE_ASSERT_EQ(FT_Init_FreeType(&ft), kFreeTypeSuccess);
  return ft;
}()};

}  // namespace

std::ostream& operator<<(std::ostream& os, FontError error)
{
  switch (error)
  {
    SDE_OSTREAM_ENUM_CASE(FontError::kElementAlreadyExists)
    SDE_OSTREAM_ENUM_CASE(FontError::kInvalidHandle)
    SDE_OSTREAM_ENUM_CASE(FontError::kAssetNotFound)
    SDE_OSTREAM_ENUM_CASE(FontError::kAssetInvalid)
    SDE_OSTREAM_ENUM_CASE(FontError::kFontNotFound)
  }
  return os;
}

void FontNativeDeleter::operator()(void* font) const
{
  SDE_LOG_DEBUG() << "FontNativeDeleter(" << font << ')';
  FT_Done_Face(reinterpret_cast<FT_Face>(font));
}

expected<void, FontError> FontCache::reload(Font& font)
{
  if (!asset::exists(font.path))
  {
    SDE_LOG_DEBUG() << "AssetNotFound";
    return make_unexpected(FontError::kAssetNotFound);
  }

  static_assert(std::is_pointer_v<FT_Face>);

  FT_Face face = nullptr;

  static constexpr FT_Long kFontIndex = 0;
  if (FT_New_Face(FreeType, font.path.string().c_str(), kFontIndex, &face) != kFreeTypeSuccess)
  {
    SDE_LOG_DEBUG() << "AssetInvalid";
    return make_unexpected(FontError::kAssetInvalid);
  }

  font.native_id = FontNativeID{reinterpret_cast<void*>(face)};
  SDE_LOG_DEBUG() << "Font(" << font.native_id << ") " << font.path;
  return {};
}

expected<void, FontError> FontCache::unload(Font& font)
{
  font.native_id = FontNativeID{nullptr};
  return {};
}

expected<Font, FontError> FontCache::generate(const asset::path& font_path)
{
  Font font{.path = font_path, .native_id = FontNativeID{nullptr}};
  if (auto ok_or_error = reload(font); !ok_or_error.has_value())
  {
    return make_unexpected(ok_or_error.error());
  }
  return font;
}

}  // namespace sde::graphics
