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
#include "sde/graphics/texture_fwd.hpp"
#include "sde/graphics/texture_handle.hpp"
#include "sde/graphics/type_set_handle.hpp"
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

struct TypeSetOptions
{
  std::size_t height_px = 10;
};

struct TypeSetInfo
{
  static constexpr std::size_t kGlyphCount = 128UL;

  TypeSetOptions options;
  FontHandle font;
  TextureHandle glyph_atlas;
  std::array<Glyph, kGlyphCount> glyphs;

  const Glyph& getGlyph(char c) const { return glyphs[static_cast<std::size_t>(c)]; }
  const Glyph& operator[](char c) const { return getGlyph(c); }

  const Bounds2i getTextBounds(std::string_view text) const;
};

enum class TypeSetError
{
  kElementAlreadyExists,
  kGlyphSizeInvalid,
  kGlyphDataMissing,
  kGlyphRenderingFailure,
  kGlyphAtlasTextureCreationFailed,
};

class TypeSetCache;

}  // namespace sde::graphics

namespace sde
{

template <> struct ResourceCacheTypes<graphics::TypeSetCache>
{
  using error_type = graphics::TypeSetError;
  using handle_type = graphics::TypeSetHandle;
  using value_type = graphics::TypeSetInfo;
};

}  // namespace sde

namespace sde::graphics
{

class TypeSetCache : public ResourceCache<TypeSetCache>
{
  friend cache_base;

private:
  expected<TypeSetInfo, TypeSetError>
  generate(TextureCache& texture_cache, const element_t<FontCache>& font, const TypeSetOptions& options = {});
};

}  // namespace sde::graphics
