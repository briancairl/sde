// SDE
#include "sde/graphics/sprite_io.hpp"
#include "sde/geometry_io.hpp"
#include "sde/graphics/sprite.hpp"
#include "sde/graphics/tile_set_io.hpp"
#include "sde/serialization_binary_file.hpp"
#include "sde/time_io.hpp"

namespace sde::serial
{

template <>
void save<binary_ofarchive, graphics::SpriteOptions>::operator()(
  binary_ofarchive& ar,
  const graphics::SpriteOptions& options) const
{
  ar << named{"tint_color", options.tint_color};
  ar << named{"frames", options.frames};
  ar << named{"frame_index", options.frame_index};
}

template <>
void load<binary_ifarchive, graphics::SpriteOptions>::operator()(binary_ifarchive& ar, graphics::SpriteOptions& options)
  const
{
  ar >> named{"tint_color", options.tint_color};
  ar >> named{"frames", options.frames};
  ar >> named{"frame_index", options.frame_index};
}

template <>
void save<binary_ofarchive, graphics::Sprite>::operator()(binary_ofarchive& ar, const graphics::Sprite& sprite) const
{
  ar << named{"options", sprite.options()};
}

template <>
void load<binary_ifarchive, graphics::Sprite>::operator()(binary_ifarchive& ar, graphics::Sprite& sprite) const
{
  graphics::SpriteOptions options;
  ar >> named{"options", options};
  sprite.setup(options);
}

template <>
void save<binary_ofarchive, graphics::AnimatedSpriteOptions>::operator()(
  binary_ofarchive& ar,
  const graphics::AnimatedSpriteOptions& options) const
{
  ar << named{"tint_color", options.tint_color};
  ar << named{"frames", options.frames};
  ar << named{"time_offset", options.time_offset};
  ar << named{"frames_per_second", options.frames_per_second};
  ar << named{"mode", options.mode};
}

template <>
void load<binary_ifarchive, graphics::AnimatedSpriteOptions>::operator()(
  binary_ifarchive& ar,
  graphics::AnimatedSpriteOptions& options) const
{
  ar >> named{"tint_color", options.tint_color};
  ar >> named{"frames", options.frames};
  ar >> named{"time_offset", options.time_offset};
  ar >> named{"frames_per_second", options.frames_per_second};
  ar >> named{"mode", options.mode};
}

template <>
void save<binary_ofarchive, graphics::AnimatedSprite>::operator()(
  binary_ofarchive& ar,
  const graphics::AnimatedSprite& sprite) const
{
  ar << named{"options", sprite.options()};
}

template <>
void load<binary_ifarchive, graphics::AnimatedSprite>::operator()(
  binary_ifarchive& ar,
  graphics::AnimatedSprite& sprite) const
{
  graphics::AnimatedSpriteOptions options;
  ar >> named{"options", options};
  sprite.setup(options);
}

}  // namespace sde::serial
