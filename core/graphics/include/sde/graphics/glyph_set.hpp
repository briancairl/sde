/**
 * @copyright 2024-present Brian Cairl
 *
 * @file glyph_set.hpp
 */
#pragma once

// C++ Standard Library
#include <array>

// SDE
#include "sde/asset.hpp"
#include "sde/expected.hpp"
#include "sde/geometry_types.hpp"
#include "sde/graphics/font_fwd.hpp"
#include "sde/graphics/font_handle.hpp"
#include "sde/graphics/glyph_set_handle.hpp"
#include "sde/graphics/texture_fwd.hpp"
#include "sde/graphics/texture_handle.hpp"
#include "sde/resource_cache.hpp"

namespace sde::graphics
{

struct Glyph
{
  char character;
  Vec2i size_px;
  Vec2i bearing_px;
  float advance_px;
  Bounds2f atlas_bounds = Bounds2f{};
};

struct GlyphSetOptions
{
  std::size_t height_px = 10;
};

struct GlyphSetInfo
{
  static constexpr std::size_t kGlyphCount = 128UL;

  GlyphSetOptions options;
  FontHandle font;
  TextureHandle glyph_atlas;
  std::array<Glyph, kGlyphCount> glyphs;

  const Glyph& getGlyph(char c) { return glyphs[static_cast<std::size_t>(c)]; }
  const Glyph& operator[](char c) { return getGlyph(c); }
};

enum class GlyphSetError
{
  kElementAlreadyExists,
  kGlyphSizeInvalid,
  kGlyphDataMissing,
  kGlyphRenderingFailure,
  kGlyphAtlasTextureCreationFailed,
};

class GlyphSetCache;

}  // namespace sde::graphics

namespace sde
{

template <> struct ResourceCacheTypes<graphics::GlyphSetCache>
{
  using error_type = graphics::GlyphSetError;
  using handle_type = graphics::GlyphSetHandle;
  using value_type = graphics::GlyphSetInfo;
};

}  // namespace sde

namespace sde::graphics
{

class GlyphSetCache : public ResourceCache<GlyphSetCache>
{
  friend class ResourceCache<GlyphSetCache>;

private:
  expected<GlyphSetInfo, GlyphSetError>
  generate(TextureCache& texture_cache, const element_t<FontCache>& font, const GlyphSetOptions& options = {});
};

}  // namespace sde::graphics
