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

struct ResourceOptions
{
  audio::MixerOptions mixer;
  graphics::Renderer2DOptions renderer;
};

enum class ResourceError
{
  kMixerCreationFailure,
  kRendererCreationFailure
};

/**
 * @brief Collection of active game assets
 */
class Resources
{
public:
  /// Audio mixer
  audio::Mixer mixer;
  /// Rendering facilities
  graphics::Renderer2D renderer;

  static expected<Resources, ResourceError> create(const ResourceOptions& options = {});

private:
  explicit Resources(audio::Mixer&& _mixer, graphics::Renderer2D&& _renderer);
};

}  // namespace sde::game