// C++ Standard Library
#include <algorithm>
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
#include "sde/graphics/type_set.hpp"
#include "sde/logging.hpp"
#include "sde/view.hpp"

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

static constexpr std::size_t kDefaultGlyphCount{128UL};

const auto kDefaultGlyphs{[] {
  std::array<char, kDefaultGlyphCount> glyphs;
  std::iota(std::begin(glyphs), std::end(glyphs), 0);
  return glyphs;
}()};

expected<void, TypeSetError> loadGlyphsFromFont(std::vector<Glyph>& glyph_lut, const FontInfo& font, int glyph_height)
{
  if (glyph_height == 0)
  {
    SDE_LOG_DEBUG("GlyphSizeInvalid");
    return make_unexpected(TypeSetError::kGlyphSizeInvalid);
  }

  glyph_lut.resize(kDefaultGlyphCount);

  const auto face = reinterpret_cast<FT_Face>(font.native_id.value());

  static constexpr int kWidthFromHeight = 0;
  if (FT_Set_Pixel_Sizes(face, kWidthFromHeight, glyph_height) != kFreeTypeSuccess)
  {
    SDE_LOG_DEBUG("GlyphSizeInvalid");
    return make_unexpected(TypeSetError::kGlyphSizeInvalid);
  }
  for (std::size_t char_index = 0; char_index < kDefaultGlyphs.size(); ++char_index)
  {
    if (FT_Load_Char(face, kDefaultGlyphs[char_index], FT_LOAD_RENDER) != kFreeTypeSuccess)
    {
      SDE_LOG_DEBUG("GlyphMissing");
      return make_unexpected(TypeSetError::kGlyphDataMissing);
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

expected<TextureHandle, TypeSetError> sendGlyphsToTexture(
  TextureCache& texture_cache,
  TextureHandle glyph_atlas,
  std::vector<Glyph>& glyph_lut,
  const FontInfo& font,
  const TypeSetOptions& options)
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
    return make_unexpected(TypeSetError::kGlyphAtlasTextureCreationFailed);
  }

  // clang-format off
  auto glyph_atlas_or_error = texture_cache.find_or_create(
    glyph_atlas,
    TypeCode::kUInt8,
    TextureShape{{texture_dimensions}},
    TextureLayout::kR,
    TextureOptions{
      .u_wrapping = TextureWrapping::kClampToEdge,
      .v_wrapping = TextureWrapping::kClampToEdge,
      .min_sampling = (options.height_px < 50) ? TextureSampling::kNearest : TextureSampling::kLinear,
      .mag_sampling = (options.height_px < 50) ? TextureSampling::kNearest : TextureSampling::kLinear,
      .flags = {
        .unpack_alignment = true
      }
    });
  // clang-format on

  if (!glyph_atlas_or_error.has_value())
  {
    SDE_LOG_DEBUG("GlyphAtlasTextureCreationFailed");
    return make_unexpected(TypeSetError::kGlyphAtlasTextureCreationFailed);
  }

  const auto face = reinterpret_cast<FT_Face>(font.native_id.value());

  int prex_px_y = 0;
  for (auto& g : glyph_lut)
  {
    if (g.size_px.prod() == 0)
    {
      continue;
    }

    // TODO(bcairl) is there anyway to prevent rendering twice?
    if (FT_Load_Char(face, g.character, FT_LOAD_RENDER) != kFreeTypeSuccess)
    {
      SDE_LOG_DEBUG("GlyphMissing");
      return make_unexpected(TypeSetError::kGlyphDataMissing);
    }

    const auto* buffer_ptr = reinterpret_cast<std::uint8_t*>(face->glyph->bitmap.buffer);
    const auto buffer_length =
      static_cast<std::size_t>(face->glyph->bitmap.width) * static_cast<std::size_t>(face->glyph->bitmap.rows);

    const Vec2i tex_coord_min_px{0, prex_px_y};
    const Vec2i tex_coord_max_px{tex_coord_min_px + g.size_px};

    if (const auto ok_or_error = replace(
          *glyph_atlas_or_error,
          make_const_view(buffer_ptr, buffer_length),
          Bounds2i{tex_coord_min_px, tex_coord_max_px});
        !ok_or_error.has_value())
    {
      SDE_LOG_DEBUG("GlyphRenderingFailure");
      return make_unexpected(TypeSetError::kGlyphRenderingFailure);
    }

    const Vec2f tex_coord_min{tex_coord_min_px.array().cast<float>() / texture_dimensions.array().cast<float>()};
    const Vec2f tex_coord_max{tex_coord_max_px.array().cast<float>() / texture_dimensions.array().cast<float>()};

    g.atlas_bounds = Bounds2f{Vec2f{tex_coord_min.x(), tex_coord_max.y()}, Vec2f{tex_coord_max.x(), tex_coord_min.y()}};

    prex_px_y += g.size_px.y();
  }

  return glyph_atlas_or_error->handle;
}

}  // namespace

TypeSetCache::TypeSetCache(TextureCache& textures, FontCache& fonts) :
    textures_{std::addressof(textures)}, fonts_{std::addressof(fonts)}
{}

const Bounds2i TypeSetInfo::getTextBounds(std::string_view text) const
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

expected<void, TypeSetError> TypeSetCache::reload(TypeSetInfo& type_set)
{
  const auto* font = fonts_->get_if(type_set.font);
  if (font == nullptr)
  {
    return make_unexpected(TypeSetError::kInvalidFont);
  }

  if (auto ok_or_error = loadGlyphsFromFont(type_set.glyphs, *font, static_cast<int>(type_set.options.height_px));
      !ok_or_error.has_value())
  {
    return make_unexpected(ok_or_error.error());
  }

  auto glyph_atlas_or_error =
    sendGlyphsToTexture(*textures_, type_set.glyph_atlas, type_set.glyphs, *font, type_set.options);
  if (!glyph_atlas_or_error.has_value())
  {
    SDE_LOG_DEBUG("GlyphTextureInvalid");
    return make_unexpected(glyph_atlas_or_error.error());
  }
  type_set.glyph_atlas = (*glyph_atlas_or_error);
  SDE_LOG_DEBUG_FMT("GlyphAtlasTexture(%lu)", type_set.glyph_atlas.id());
  return {};
}

expected<void, TypeSetError> TypeSetCache::unload(TypeSetInfo& type_set)
{
  textures_->remove(type_set.glyph_atlas);
  type_set.glyphs.clear();
  return {};
}

expected<TypeSetInfo, TypeSetError> TypeSetCache::generate(
  FontHandle font,
  const TypeSetOptions& options,
  TextureHandle glyph_atlas,
  ResourceLoading loading)
{
  TypeSetInfo type_set{.options = options, .font = font, .glyph_atlas = glyph_atlas, .glyphs = {}};
  if (loading == ResourceLoading::kDeferred)
  {
    return type_set;
  }
  if (auto ok_or_error = reload(type_set); !ok_or_error.has_value())
  {
    return make_unexpected(ok_or_error.error());
  }
  return type_set;
}

}  // namespace sde::graphics
