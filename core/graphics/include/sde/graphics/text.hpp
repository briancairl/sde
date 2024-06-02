/**
 * @copyright 2024-present Brian Cairl
 *
 * @file text.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>
#include <string_view>
#include <vector>

// SDE
#include "sde/asset.hpp"
#include "sde/expected.hpp"
#include "sde/geometry_types.hpp"
#include "sde/graphics/texture_fwd.hpp"
#include "sde/graphics/texture_handle.hpp"
#include "sde/view.hpp"

namespace sde::graphics
{


struct Glyph
{
  Bounds2f tex_rect;
  Vec2f size_px;
  Vec2f bearing_px;
  float advance_px;
};


struct GlyphOptions
{
  std::size_t height_px = 10;
};


class GlyphSet
{
public:
  /**
   * @brief Returns handle to atlas texture for this tile set
   */
  const TextureHandle atlas() const { return atlas_texture_; }

  /**
   * @brief Returns texture-space bounds for a given tile
   */
  const Glyph& get(const std::size_t tile) const { return glyphs_[tile]; }

  /**
   * @brief Returns texture-space bounds for a given tile
   */
  const Glyph& operator[](const std::size_t tile) const { return glyphs_[tile]; }

  /**
   * @brief Returns the number of tiles
   */
  std::size_t size() const { return glyphs_.size(); }

  GlyphSet(TextureHandle atlas_texture, std::vector<Glyph> glyphs);

private:
  TextureHandle atlas_texture_;
  std::vector<Glyph> glyphs_;
};


enum class FontError
{
  kAssetNotFound,
  kAssetInvalid,
  kGlyphMissing,
  kGlyphSizeInvalid,
  kGlyphTextureInvalid,
};

class Font
{
public:
  Font(Font&& other);

  ~Font();

  static expected<Font, FontError> load(const asset::path& font_path);

  expected<GlyphSet, FontError> glyphs(
    const TextureHandle& texture,
    const TextureInfo& texture_info,
    const GlyphOptions& options = {},
    View<const char> glyphs = View<const char>{nullptr});

  expected<GlyphSet, FontError> glyphs(
    TextureCache& texture_cache,
    const GlyphOptions& options = {},
    View<const char> glyphs = View<const char>{nullptr});

private:
  expected<GlyphSet, FontError> createImpl(
    const TextureHandle& texture,
    const TextureInfo& texture_info,
    const GlyphOptions& options,
    const View<const char>& glyphs);

  Font() = default;
  void* native_handle_ = nullptr;
};

struct Text
{
  std::string_view text = "text";
  Vec2f position = Vec2f::Zero();
  Vec4f color = Vec4f::Ones();
  float scale = 0.05f;
  std::size_t texture_unit = 0;
};

}  // namespace sde::graphics
