/**
 * @copyright 2024-present Brian Cairl
 *
 * @file sprite_io.hpp
 */
#pragma once

// SDE
#include "sde/geometry_io.hpp"
#include "sde/graphics/sprite.hpp"
#include "sde/graphics/tile_set_io.hpp"
#include "sde/resource_handle_io.hpp"
#include "sde/serialization.hpp"
#include "sde/time_io.hpp"

namespace sde::serial
{

template <typename Archive> struct serialize<Archive, graphics::SpriteOptions>
{
  void operator()(Archive& ar, graphics::SpriteOptions& options) const
  {
    ar& named{"tint_color", options.tint_color};
    ar& named{"frames", options.frames};
    ar& named{"frame_index", options.frame_index};
  }
};

template <typename Archive> struct save<Archive, graphics::Sprite>
{
  void operator()(Archive& ar, const graphics::Sprite& sprite) const { ar << named{"options", sprite.options()}; }
};

template <typename Archive> struct load<Archive, graphics::Sprite>
{
  void operator()(Archive& ar, graphics::Sprite& sprite) const
  {
    graphics::SpriteOptions options;
    ar >> named{"options", options};
    sprite.setup(options);
  }
};

template <typename Archive> struct serialize<Archive, graphics::AnimatedSpriteOptions>
{
  void operator()(Archive& ar, graphics::AnimatedSpriteOptions& options) const
  {
    ar& named{"tint_color", options.tint_color};
    ar& named{"frames", options.frames};
    ar& named{"time_offset", options.time_offset};
    ar& named{"frames_per_second", options.frames_per_second};
    ar& named{"mode", options.mode};
  }
};

template <typename Archive> struct save<Archive, graphics::AnimatedSprite>
{
  void operator()(Archive& ar, const graphics::AnimatedSprite& sprite) const
  {
    ar << named{"options", sprite.options()};
  }
};

template <typename Archive> struct load<Archive, graphics::AnimatedSprite>
{
  void operator()(Archive& ar, graphics::AnimatedSprite& sprite) const
  {
    graphics::AnimatedSpriteOptions options;
    ar >> named{"options", options};
    sprite.setup(options);
  }
};

}  // namespace sde::serial
