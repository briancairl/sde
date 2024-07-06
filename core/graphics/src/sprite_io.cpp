// SDE
#include "sde/graphics/sprite_io.hpp"
#include "sde/geometry_io.hpp"
#include "sde/graphics/sprite.hpp"
#include "sde/graphics/tile_set_io.hpp"
#include "sde/serialization_binary_file.hpp"
#include "sde/time_io.hpp"

namespace sde::serial
{

template <typename Archive>
void save<Archive, graphics::SpriteOptions>::operator()(Archive& ar, const graphics::SpriteOptions& options) const
{
  ar << named{"tint_color", options.tint_color};
  ar << named{"frames", options.frames};
  ar << named{"frame_index", options.frame_index};
}

template <typename Archive>
void load<Archive, graphics::SpriteOptions>::operator()(Archive& ar, graphics::SpriteOptions& options) const
{
  ar >> named{"tint_color", options.tint_color};
  ar >> named{"frames", options.frames};
  ar >> named{"frame_index", options.frame_index};
}

template <typename Archive>
void save<Archive, graphics::Sprite>::operator()(Archive& ar, const graphics::Sprite& sprite) const
{
  ar << named{"options", sprite.options()};
}

template <typename Archive>
void load<Archive, graphics::Sprite>::operator()(Archive& ar, graphics::Sprite& sprite) const
{
  graphics::SpriteOptions options;
  ar >> named{"options", options};
  sprite.setup(options);
}

template <typename Archive>
void save<Archive, graphics::AnimatedSpriteOptions>::operator()(
  Archive& ar,
  const graphics::AnimatedSpriteOptions& options) const
{
  ar << named{"tint_color", options.tint_color};
  ar << named{"frames", options.frames};
  ar << named{"time_offset", options.time_offset};
  ar << named{"frames_per_second", options.frames_per_second};
  ar << named{"mode", options.mode};
}

template <typename Archive>
void load<Archive, graphics::AnimatedSpriteOptions>::operator()(Archive& ar, graphics::AnimatedSpriteOptions& options)
  const
{
  ar >> named{"tint_color", options.tint_color};
  ar >> named{"frames", options.frames};
  ar >> named{"time_offset", options.time_offset};
  ar >> named{"frames_per_second", options.frames_per_second};
  ar >> named{"mode", options.mode};
}

template <typename Archive>
void save<Archive, graphics::AnimatedSprite>::operator()(Archive& ar, const graphics::AnimatedSprite& sprite) const
{
  ar << named{"options", sprite.options()};
}

template <typename Archive>
void load<Archive, graphics::AnimatedSprite>::operator()(Archive& ar, graphics::AnimatedSprite& sprite) const
{
  graphics::AnimatedSpriteOptions options;
  ar >> named{"options", options};
  sprite.setup(options);
}

template struct save<binary_ofarchive, graphics::SpriteOptions>;
template struct load<binary_ifarchive, graphics::SpriteOptions>;
template struct save<binary_ofarchive, graphics::Sprite>;
template struct load<binary_ifarchive, graphics::Sprite>;

template struct save<binary_ofarchive, graphics::AnimatedSpriteOptions>;
template struct load<binary_ifarchive, graphics::AnimatedSpriteOptions>;
template struct save<binary_ofarchive, graphics::AnimatedSprite>;
template struct load<binary_ifarchive, graphics::AnimatedSprite>;

}  // namespace sde::serial
