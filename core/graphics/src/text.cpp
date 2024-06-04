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
#include "sde/graphics/text.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/logging.hpp"
#include "sde/resource.hpp"

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

const auto kDefaultGlyphs{[] {
  std::array<char, 128> glyphs;
  std::iota(std::begin(glyphs), std::end(glyphs), 0);
  return glyphs;
}()};

View<const char> or_default(View<const char> glyphs)
{
  return glyphs.empty() ? View<const char>{kDefaultGlyphs.data(), kDefaultGlyphs.size()} : glyphs;
}

}  // namespace

GlyphSet::GlyphSet(TextureHandle atlas_texture, std::vector<Glyph> glyphs) :
    atlas_texture_{atlas_texture}, glyphs_{std::move(glyphs)}
{}

Font::Font(Font&& other) : native_handle_{other.native_handle_} { other.native_handle_ = nullptr; }

Font::~Font()
{
  if (native_handle_ == nullptr)
  {
    return;
  }
  FT_Done_Face(reinterpret_cast<FT_Face>(native_handle_));
}

expected<Font, FontError> Font::load(const asset::path& font_path)
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

  Font font;
  font.native_handle_ = reinterpret_cast<void*>(face);

  return font;
}

expected<GlyphSet, FontError> Font::createImpl(
  const TextureHandle& texture,
  const TextureInfo& texture_info,
  const GlyphOptions& options,
  const View<const char>& glyphs)
{
  const auto face = reinterpret_cast<FT_Face>(native_handle_);

  static constexpr int kWidthFromHeight = 0;
  if ((options.height_px == 0) or (FT_Set_Pixel_Sizes(face, kWidthFromHeight, options.height_px) != kFreeTypeSuccess))
  {
    SDE_LOG_DEBUG("GlyphSizeInvalid");
    return make_unexpected(FontError::kGlyphSizeInvalid);
  }

  std::vector<Glyph> glyph_data;
  glyph_data.reserve(glyphs.size());

  int last_offset_y = 0;
  for (const char c : glyphs)
  {
    if (FT_Load_Char(face, c, FT_LOAD_RENDER) != kFreeTypeSuccess)
    {
      SDE_LOG_DEBUG("GlyphMissing");
      return make_unexpected(FontError::kGlyphMissing);
    }

    const auto buffer_length =
      static_cast<std::size_t>(face->glyph->bitmap.width) * static_cast<std::size_t>(face->glyph->bitmap.rows);

    const auto buffer_data = reinterpret_cast<std::uint8_t*>(face->glyph->bitmap.buffer);

    const Vec2i tex_coord_min_px{0, last_offset_y};
    const Vec2i tex_coord_max_px{face->glyph->bitmap.width, last_offset_y + face->glyph->bitmap.rows};

    if (buffer_length == 0)
    {
      // DO NOT UPLOAD
    }
    else if (const auto ok_or_error = replace(
               texture_info, make_const_view(buffer_data, buffer_length), Bounds2i{tex_coord_min_px, tex_coord_max_px});
             !ok_or_error.has_value())
    {
      SDE_LOG_DEBUG("GlyphTextureInvalid");
      return make_unexpected(FontError::kGlyphTextureInvalid);
    }

    const Vec2f tex_coord_min{tex_coord_min_px.array().cast<float>() / texture_info.shape.value.array().cast<float>()};
    const Vec2f tex_coord_max{tex_coord_max_px.array().cast<float>() / texture_info.shape.value.array().cast<float>()};

    const float normalize_scaling = 1.0F / options.height_px;

    glyph_data.push_back({
      .tex_rect = Bounds2f{Vec2f{tex_coord_min.x(), tex_coord_max.y()}, Vec2f{tex_coord_max.x(), tex_coord_min.y()}},
      .size_px = normalize_scaling *
        Vec2f{static_cast<float>(face->glyph->bitmap.width), static_cast<float>(face->glyph->bitmap.rows)},
      .bearing_px = normalize_scaling *
        Vec2f{static_cast<float>(face->glyph->bitmap_left), static_cast<float>(face->glyph->bitmap_top)},
      .advance_px = normalize_scaling * static_cast<float>(face->glyph->advance.x) / 64.0F,
    });

    last_offset_y += face->glyph->bitmap.rows;
  }

  return GlyphSet{texture, std::move(glyph_data)};
}


expected<GlyphSet, FontError>
Font::glyphs(TextureCache& texture_cache, const GlyphOptions& options, View<const char> glyphs)
{
  glyphs = or_default(glyphs);

  // clang-format off
  auto glyph_atlas_texture_or_error = texture_cache.create<std::uint8_t>(
    TextureShape{{static_cast<int>(options.height_px), static_cast<int>(glyphs.size() * options.height_px)}},
    TextureLayout::kR,
    TextureOptions{
      .u_wrapping = TextureWrapping::kClampToEdge,
      .v_wrapping = TextureWrapping::kClampToEdge,
      .min_sampling = TextureSampling::kLinear,
      .mag_sampling = TextureSampling::kLinear,
      .flags = {
        .unpack_alignment = true
      }
    });
  // clang-format on

  if (glyph_atlas_texture_or_error.has_value())
  {
    return createImpl(
      *glyph_atlas_texture_or_error, *texture_cache.get(*glyph_atlas_texture_or_error), options, glyphs);
  }
  SDE_LOG_DEBUG("GlyphTextureInvalid");
  return make_unexpected(FontError::kGlyphTextureInvalid);
}

expected<GlyphSet, FontError> Font::glyphs(
  const TextureHandle& texture,
  const TextureInfo& texture_info,
  const GlyphOptions& options,
  View<const char> glyphs)
{
  if (texture_info.layout == TextureLayout::kR)
  {
    SDE_LOG_DEBUG("GlyphTextureInvalid");
    return make_unexpected(FontError::kGlyphTextureInvalid);
  }
  return createImpl(texture, texture_info, options, or_default(glyphs));
}

}  // namespace sde::graphics
