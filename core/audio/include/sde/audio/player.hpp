/**
 * @copyright 2024-present Brian Cairl
 *
 * @file player.hpp
 */
#pragma once

// C++ Standard Library

// SDE
#include "sde/audio/typedef.hpp"
#include "sde/expected.hpp"
#include "sde/view.hpp"

namespace sde::audio
{

/**
 * @brief High-level interface for sound playback
 */
class Player
{
public:
  ~Player();

private:
  Player() = default;
  Player(const Player&) = delete;
};

}  // namespace sde::audio
