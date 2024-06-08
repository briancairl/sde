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
#include "sde/graphics/glyph_set.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/logging.hpp"
#include "sde/view.hpp"

namespace sde::graphics
{
namespace
{

using GlyphLookup = std::array<Glyph, GlyphSetInfo::kGlyphCount>;

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
  std::array<char, GlyphSetInfo::kGlyphCount> glyphs;
  std::iota(std::begin(glyphs), std::end(glyphs), 0);
  return glyphs;
}()};

expected<void, GlyphSetError> loadGlyphsFromFont(GlyphLookup& glyph_lut, const FontInfo& font, int glyph_height)
{
  if (glyph_height == 0)
  {
    SDE_LOG_DEBUG("GlyphSizeInvalid");
    return make_unexpected(GlyphSetError::kGlyphSizeInvalid);
  }

  const auto face = reinterpret_cast<FT_Face>(font.native_id.value());

  static constexpr int kWidthFromHeight = 0;
  if (FT_Set_Pixel_Sizes(face, kWidthFromHeight, glyph_height) != kFreeTypeSuccess)
  {
    SDE_LOG_DEBUG("GlyphSizeInvalid");
    return make_unexpected(GlyphSetError::kGlyphSizeInvalid);
  }
  for (std::size_t char_index = 0; char_index < kDefaultGlyphs.size(); ++char_index)
  {
    if (FT_Load_Char(face, kDefaultGlyphs[char_index], FT_LOAD_RENDER) != kFreeTypeSuccess)
    {
      SDE_LOG_DEBUG("GlyphMissing");
      return make_unexpected(GlyphSetError::kGlyphDataMissing);
    }
    else
    {
      glyph_lut[char_index] = Glyph{
        .character = kDefaultGlyphs[char_index],
        .size_px = Vec2i{static_cast<float>(face->glyph->bitmap.width), static_cast<float>(face->glyph->bitmap.rows)},
        .bearing_px = Vec2i{static_cast<float>(face->glyph->bitmap_left), static_cast<float>(face->glyph->bitmap_top)},
        .advance_px = static_cast<float>(face->glyph->advance.x) / 64.0F,
        .atlas_bounds = Bounds2f{},
      };
    }
  }
  return {};
}

expected<TextureHandle, GlyphSetError>
sendGlyphsToTexture(TextureCache& texture_cache, GlyphLookup& glyph_lut, const FontInfo& font)
{
  // Compute required texture dimensions
  Vec2i texture_dimensions{0, 0};
  for (const auto& g : glyph_lut)
  {
    texture_dimensions.x() = std::max(texture_dimensions.x(), g.size_px.x());
    texture_dimensions.y() += g.size_px.y();
  }

  if (texture_dimensions.prod() == 0)
  {
    SDE_LOG_DEBUG("GlyphAtlasTextureCreationFailed");
    return make_unexpected(GlyphSetError::kGlyphAtlasTextureCreationFailed);
  }

  // clang-format off
  auto atlas_texture_or_error = texture_cache.create(
    Type<std::uint8_t>,
    TextureShape{{texture_dimensions}},
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

  if (!atlas_texture_or_error.has_value())
  {
    SDE_LOG_DEBUG("GlyphAtlasTextureCreationFailed");
    return make_unexpected(GlyphSetError::kGlyphAtlasTextureCreationFailed);
  }

  const auto face = reinterpret_cast<FT_Face>(font.native_id.value());

  int prex_px_y = 0;
  for (auto& g : glyph_lut)
  {
    if (g.size_px.prod() == 0)
    {
      continue;
    }

    if (FT_Load_Char(face, g.character, FT_LOAD_RENDER) != kFreeTypeSuccess)
    {
      SDE_LOG_DEBUG("GlyphMissing");
      return make_unexpected(GlyphSetError::kGlyphDataMissing);
    }

    const auto* buffer_ptr = reinterpret_cast<std::uint8_t*>(face->glyph->bitmap.buffer);
    const auto buffer_length =
      static_cast<std::size_t>(face->glyph->bitmap.width) * static_cast<std::size_t>(face->glyph->bitmap.rows);

    const Vec2i tex_coord_min_px{0, prex_px_y};
    const Vec2i tex_coord_max_px{tex_coord_min_px + g.size_px};

    if (const auto ok_or_error = replace(
          *atlas_texture_or_error,
          make_const_view(buffer_ptr, buffer_length),
          Bounds2i{tex_coord_min_px, tex_coord_max_px});
        !ok_or_error.has_value())
    {
      SDE_LOG_DEBUG("GlyphRenderingFailure");
      return make_unexpected(GlyphSetError::kGlyphRenderingFailure);
    }

    const Vec2f tex_coord_min{tex_coord_min_px.array().cast<float>() / texture_dimensions.array().cast<float>()};
    const Vec2f tex_coord_max{tex_coord_max_px.array().cast<float>() / texture_dimensions.array().cast<float>()};

    g.atlas_bounds = Bounds2f{Vec2f{tex_coord_min.x(), tex_coord_max.y()}, Vec2f{tex_coord_max.x(), tex_coord_min.y()}};

    prex_px_y += g.size_px.y();
  }

  return atlas_texture_or_error->handle;
}

}  // namespace

const Bounds2i GlyphSetInfo::getTextBounds(std::string_view text) const
{
  Bounds2i text_bounds;

  Vec2i cursor{0, 0};
  text_bounds.extend(cursor);

  for (const char c : text)
  {
    const auto& g = getGlyph(c);
    const Vec2i rect_min = cursor + Vec2i{g.bearing_px.x(), (g.bearing_px.y() - g.size_px.y())};
    const Vec2i rect_max = rect_min + g.size_px;
    text_bounds.extend(rect_min);
    text_bounds.extend(rect_max);
    cursor.x() += g.advance_px;
  }
  return text_bounds;
}

expected<GlyphSetInfo, GlyphSetError>
GlyphSetCache::generate(TextureCache& texture_cache, const element_t<FontCache>& font, const GlyphSetOptions& options)
{
  GlyphLookup glyph_lut;
  if (auto ok_or_error = loadGlyphsFromFont(glyph_lut, font, static_cast<int>(options.height_px));
      !ok_or_error.has_value())
  {
    return make_unexpected(ok_or_error.error());
  }

  auto glyph_atlas_texture_or_error = sendGlyphsToTexture(texture_cache, glyph_lut, font);
  if (!glyph_atlas_texture_or_error.has_value())
  {
    SDE_LOG_DEBUG("GlyphTextureInvalid");
    return make_unexpected(glyph_atlas_texture_or_error.error());
  }

  return GlyphSetInfo{
    .options = options,
    .font = font,
    .glyph_atlas = std::move(glyph_atlas_texture_or_error).value(),
    .glyphs = std::move(glyph_lut)};
}

}  // namespace sde::graphics
