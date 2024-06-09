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

void FontNativeDeleter::operator()(void* font) const { FT_Done_Face(reinterpret_cast<FT_Face>(font)); }

expected<FontInfo, FontError> FontCache::generate(const asset::path& font_path)
{
  if (!asset::exists(font_path))
  {
    SDE_LOG_DEBUG("AssetNotFound");
    return make_unexpected(FontError::kAssetNotFound);
  }

  static_assert(std::is_pointer_v<FT_Face>);

  FT_Face face = nullptr;

  static constexpr FT_Long kFontIndex = 0;
  if (FT_New_Face(FreeType, font_path.string().c_str(), kFontIndex, &face) != kFreeTypeSuccess)
  {
    SDE_LOG_DEBUG("AssetInvalid");
    return make_unexpected(FontError::kAssetInvalid);
  }

  return FontInfo{.native_id = FontNativeID{reinterpret_cast<void*>(face)}};
}

}  // namespace sde::graphics
