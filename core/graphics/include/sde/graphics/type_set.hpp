/**
 * @copyright 2024-present Brian Cairl
 *
 * @file glyph_set.hpp
 */
#pragma once

// C++ Standard Library
#include <vector>

// SDE
#include "sde/asset.hpp"
#include "sde/expected.hpp"
#include "sde/geometry_types.hpp"
#include "sde/graphics/font_fwd.hpp"
#include "sde/graphics/font_handle.hpp"
#include "sde/graphics/texture_fwd.hpp"
#include "sde/graphics/texture_handle.hpp"
#include "sde/graphics/type_set_fwd.hpp"
#include "sde/graphics/type_set_handle.hpp"
#include "sde/resource.hpp"
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

struct TypeSetOptions : Resource<TypeSet>
{
  std::size_t height_px = 10;

  auto field_list() { return std::make_tuple((Field{"height_px", height_px})); }
};

struct TypeSet : Resource<TypeSet>
{
  TypeSetOptions options;
  FontHandle font;
  TextureHandle glyph_atlas;
  std::vector<Glyph> glyphs;

  auto field_list()
  {
    return std::make_tuple(
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
  kElementAlreadyExists,
  kInvalidHandle,
  kInvalidFont,
  kGlyphSizeInvalid,
  kGlyphDataMissing,
  kGlyphRenderingFailure,
  kGlyphAtlasTextureCreationFailed,
};

}  // namespace sde::graphics

namespace sde
{

template <> struct Hasher<graphics::TypeSetOptions> : ResourceHasher
{};
template <> struct Hasher<graphics::TypeSet> : ResourceHasher
{};

template <> struct ResourceCacheTypes<graphics::TypeSetCache>
{
  using error_type = graphics::TypeSetError;
  using handle_type = graphics::TypeSetHandle;
  using value_type = graphics::TypeSet;
};

}  // namespace sde

namespace sde::graphics
{

class TypeSetCache : public ResourceCache<TypeSetCache>
{
  friend cache_base;

public:
  TypeSetCache(TextureCache& texture, FontCache& fonts);

private:
  TextureCache* textures_;
  FontCache* fonts_;

  expected<void, TypeSetError> reload(TypeSet& type_set);
  expected<void, TypeSetError> unload(TypeSet& type_set);
  expected<TypeSet, TypeSetError> generate(FontHandle font, const TypeSetOptions& options = {});
};

}  // namespace sde::graphics
