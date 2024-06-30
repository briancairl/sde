/**
 * @copyright 2024-present Brian Cairl
 *
 * @file sprite.hpp
 */
#pragma once

// SDE
#include "sde/geometry_types.hpp"
#include "sde/graphics/renderer_fwd.hpp"
#include "sde/graphics/texture_handle.hpp"
#include "sde/graphics/tile_set_handle.hpp"
#include "sde/time.hpp"

namespace sde::graphics
{

struct SpriteOptions
{
  Vec4f tint_color = Vec4f::Ones();
  TileSetHandle frames;
  std::size_t frame_index = 0;
};

bool operator==(const SpriteOptions& lhs, const SpriteOptions& rhs);

class Sprite
{
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
  Options options_;
};

bool operator==(const Sprite& lhs, const Sprite& rhs);

enum class AnimatedSpriteMode
{
  kLooped,
  kOneShot
};

struct AnimatedSpriteOptions
{
  Vec4f tint_color = Vec4f::Ones();
  TileSetHandle frames;
  TimeOffset time_offset = TimeOffset::zero();
  Rate frames_per_second = Hertz(5.0);
  AnimatedSpriteMode mode = AnimatedSpriteMode::kOneShot;
};

bool operator==(const AnimatedSpriteOptions& lhs, const AnimatedSpriteOptions& rhs);

class AnimatedSprite
{
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
  Options options_;
};

bool operator==(const AnimatedSprite& lhs, const AnimatedSprite& rhs);

}  // namespace sde::graphics
