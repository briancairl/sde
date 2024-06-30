// C++ Standard Library
#include <ostream>

// SDE
#include "sde/game/systems.hpp"
#include "sde/logging.hpp"

namespace sde::game
{

expected<Systems, SystemError> Systems::create(const SystemOptions& options)
{
  auto mixer_or_error = audio::Mixer::create(options.mixer);
  if (!mixer_or_error.has_value())
  {
    SDE_LOG_DEBUG("MixerCreationFailure");
    return make_unexpected(SystemError::kMixerCreationFailure);
  }

  auto renderer_or_error = graphics::Renderer2D::create(options.renderer);
  if (!renderer_or_error.has_value())
  {
    SDE_LOG_DEBUG("RendererCreationFailure");
    return make_unexpected(SystemError::kRendererCreationFailure);
  }

  return Systems{
    std::move(mixer_or_error).value(),
    std::move(renderer_or_error).value(),
  };
}

Systems::Systems(audio::Mixer&& _mixer, graphics::Renderer2D&& _renderer) :
    mixer{std::move(_mixer)}, renderer{std::move(_renderer)}
{}

}  // namespace sde::game
