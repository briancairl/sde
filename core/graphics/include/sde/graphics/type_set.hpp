/**
 * @copyright 2024-present Brian Cairl
 *
 * @file type_set.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>

// SDE
#include "sde/asset.hpp"
#include "sde/expected.hpp"
#include "sde/geometry.hpp"
#include "sde/graphics/font_fwd.hpp"
#include "sde/graphics/font_handle.hpp"
#include "sde/graphics/texture_fwd.hpp"
#include "sde/graphics/texture_handle.hpp"
#include "sde/graphics/type_set_fwd.hpp"
#include "sde/graphics/type_set_handle.hpp"
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"
#include "sde/vector.hpp"

namespace sde::graphics
{

struct Glyph
{
  char character;
  Vec2i size_px;
  Vec2i bearing_px;
  float advance_px;
  Rect2f atlas_bounds = Rect2f{};
};

struct TypeSetOptions : Resource<TypeSetOptions>
{
  std::size_t height_px = 10;

  auto field_list() { return FieldList((Field{"height_px", height_px})); }
};

struct TypeSet : Resource<TypeSet>
{
  TypeSetOptions options;
  FontHandle font;
  TextureHandle glyph_atlas;
  sde::vector<Glyph> glyphs;

  auto field_list()
  {
    return FieldList(
      (Field{"options", options}),
      (Field{"font", font}),
      (Field{"glyph_atlas", glyph_atlas}),
      (_Stub{"glyphs", glyphs}));
  }

  const Glyph& getGlyph(char c) const { return glyphs[static_cast<std::size_t>(c)]; }
  const Glyph& operator[](char c) const { return getGlyph(c); }

  const Bounds2i getTextBounds(std::string_view text) const;
};

enum class TypeSetError
{
  SDE_RESOURCE_CACHE_ERROR_ENUMS,
  kInvalidFont,
  kGlyphSizeInvalid,
  kGlyphDataMissing,
  kGlyphRenderingFailure,
  kGlyphAtlasTextureCreationFailed,
};

std::ostream& operator<<(std::ostream& os, TypeSetError error);

class TypeSetCache : public ResourceCache<TypeSetCache>
{
  friend fundemental_type;

private:
  expected<void, TypeSetError> reload(dependencies deps, TypeSet& type_set);
  expected<void, TypeSetError> unload(dependencies deps, TypeSet& type_set);
  expected<TypeSet, TypeSetError> generate(dependencies deps, FontHandle font, const TypeSetOptions& options = {});
};

}  // namespace sde::graphics

namespace sde
{
template <> struct Hasher<graphics::TypeSetOptions> : ResourceHasher
{};

template <> struct Hasher<graphics::TypeSet> : ResourceHasher
{};
}  // namespace sde