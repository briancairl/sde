/**
 * @copyright 2024-present Brian Cairl
 *
 * @file sprite.hpp
 */
#pragma once

// SDE
#include "sde/geometry.hpp"
#include "sde/graphics/renderer_fwd.hpp"
#include "sde/graphics/texture_handle.hpp"
#include "sde/graphics/tile_set_handle.hpp"
#include "sde/resource.hpp"
#include "sde/time.hpp"

namespace sde::graphics
{

struct SpriteOptions : Resource<SpriteOptions>
{
  Vec4f tint_color = Vec4f::Ones();
  TileSetHandle frames;
  std::size_t frame_index = 0;

  auto field_list()
  {
    return FieldList((Field{"tint_color", tint_color}), (Field{"frames", frames}), (Field{"frame_index", frame_index}));
  }
};

bool operator==(const SpriteOptions& lhs, const SpriteOptions& rhs);

class Sprite : public Resource<Sprite>
{
  friend fundemental_type;

public:
  using Options = SpriteOptions;

  Sprite() = default;
  explicit Sprite(const Options& options);

  void draw(RenderPass& rp, const Bounds2f& rect) const;

  void setup(const Options& options) { options_ = options; }
  void setTintColor(const Vec4f& color) { options_.tint_color = color; }
  void setFrames(TileSetHandle frames) { options_.frames = frames; }
  void setFrameIndex(std::size_t frame_index) { options_.frame_index = frame_index; }

  const Options& options() const { return options_; }

private:
  auto field_list() { return options_.field_list(); }
  Options options_;
};

bool operator==(const Sprite& lhs, const Sprite& rhs);

enum class AnimatedSpriteMode
{
  kLooped,
  kOneShot
};

struct AnimatedSpriteOptions : Resource<AnimatedSpriteOptions>
{
  Vec4f tint_color = Vec4f::Ones();
  TileSetHandle frames;
  TimeOffset time_offset = TimeOffset::zero();
  Rate frames_per_second = Hertz(5.0);
  AnimatedSpriteMode mode = AnimatedSpriteMode::kOneShot;

  auto field_list()
  {
    return FieldList(
      (Field{"tint_color", tint_color}),
      (Field{"frames", frames}),
      (Field{"time_offset", time_offset}),
      (Field{"frames_per_second", frames_per_second}),
      (Field{"mode", mode}));
  }
};

bool operator==(const AnimatedSpriteOptions& lhs, const AnimatedSpriteOptions& rhs);

class AnimatedSprite : public Resource<AnimatedSprite>
{
  friend fundemental_type;

public:
  using Mode = AnimatedSpriteMode;
  using Options = AnimatedSpriteOptions;

  AnimatedSprite() = default;
  explicit AnimatedSprite(const Options& options);

  void draw(RenderPass& rp, TimeOffset t, const Bounds2f& rect) const;

  void setup(const Options& options) { options_ = options; }
  void setTintColor(const Vec4f& color) { options_.tint_color = color; }
  void setFrames(TileSetHandle frames) { options_.frames = frames; }
  void setTimeOffset(TimeOffset time_offset) { options_.time_offset = time_offset; }
  void setFrameRate(Rate rate) { options_.frames_per_second = rate; }
  void setMode(Mode mode) { options_.mode = mode; }

  const Options& options() const { return options_; }

private:
  auto field_list() { return options_.field_list(); }
  Options options_;
};

bool operator==(const AnimatedSprite& lhs, const AnimatedSprite& rhs);

}  // namespace sde::graphics
