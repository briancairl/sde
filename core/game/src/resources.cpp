// C++ Standard Library
#include <ostream>

// SDE
#include "sde/game/resources.hpp"
#include "sde/logging.hpp"

namespace sde::game
{

expected<Resources, ResourceError> Resources::create(const ResourceOptions& options)
{
  auto mixer_or_error = audio::Mixer::create(options.mixer);
  if (!mixer_or_error.has_value())
  {
    SDE_LOG_DEBUG("MixerCreationFailure");
    return make_unexpected(ResourceError::kMixerCreationFailure);
  }

  auto renderer_or_error = graphics::Renderer2D::create(options.renderer);
  if (!renderer_or_error.has_value())
  {
    SDE_LOG_DEBUG("RendererCreationFailure");
    return make_unexpected(ResourceError::kRendererCreationFailure);
  }

  return Resources{
    std::move(mixer_or_error).value(),
    std::move(renderer_or_error).value(),
  };
}

Resources::Resources(audio::Mixer&& _mixer, graphics::Renderer2D&& _renderer) :
    mixer{std::move(_mixer)}, renderer{std::move(_renderer)}
{}

}  // namespace sde::game
