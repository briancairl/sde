/**
 * @copyright 2024-present Brian Cairl
 *
 * @file sprite_io.hpp
 */
#pragma once

// SDE
#include "sde/graphics/sprite_fwd.hpp"
#include "sde/serialization.hpp"

namespace sde::serial
{

template <typename Archive> struct save<Archive, graphics::SpriteOptions>
{
  void operator()(Archive& ar, const graphics::SpriteOptions& options) const;
};

template <typename Archive> struct load<Archive, graphics::SpriteOptions>
{
  void operator()(Archive& ar, graphics::SpriteOptions& options) const;
};

template <typename Archive> struct save<Archive, graphics::Sprite>
{
  void operator()(Archive& ar, const graphics::Sprite& sprite) const;
};

template <typename Archive> struct load<Archive, graphics::Sprite>
{
  void operator()(Archive& ar, graphics::Sprite& sprite) const;
};

template <typename Archive> struct save<Archive, graphics::AnimatedSpriteOptions>
{
  void operator()(Archive& ar, const graphics::AnimatedSpriteOptions& options) const;
};

template <typename Archive> struct load<Archive, graphics::AnimatedSpriteOptions>
{
  void operator()(Archive& ar, graphics::AnimatedSpriteOptions& options) const;
};

template <typename Archive> struct save<Archive, graphics::AnimatedSprite>
{
  void operator()(Archive& ar, const graphics::AnimatedSprite& sprite) const;
};

template <typename Archive> struct load<Archive, graphics::AnimatedSprite>
{
  void operator()(Archive& ar, graphics::AnimatedSprite& sprite) const;
};

}  // namespace sde::serial
