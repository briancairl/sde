/**
 * @copyright 2024-present Brian Cairl
 *
 * @file assets.hpp
 */
#pragma once

// SDE
#include "sde/audio/mixer.hpp"
#include "sde/expected.hpp"
#include "sde/graphics/renderer.hpp"

namespace sde::game
{

struct SystemOptions
{
  audio::MixerOptions mixer;
  graphics::Renderer2DOptions renderer;
};

enum class SystemError
{
  kMixerCreationFailure,
  kRendererCreationFailure
};

/**
 * @brief Collection of active game assets
 */
class Systems
{
public:
  /// Audio mixer
  audio::Mixer mixer;
  /// Rendering facilities
  graphics::Renderer2D renderer;

  static expected<Systems, SystemError> create(const SystemOptions& options = {});

private:
  explicit Systems(audio::Mixer&& _mixer, graphics::Renderer2D&& _renderer);
};

}  // namespace sde::game