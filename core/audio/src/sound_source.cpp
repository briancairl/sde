// C++ Standard Library
#include <optional>
#include <ostream>

// Backend
#include "openal.inl"

// SDE
#include "sde/audio/player.hpp"
#include "sde/audio/sound.hpp"
#include "sde/audio/sound_source.hpp"
#include "sde/logging.hpp"

namespace sde::audio
{

void NativeSourceDeleter::operator()(source_handle_t id) const { alDeleteSources(1, &id); }

SoundSource::SoundSource(NativeSourceID&& native_source) : native_source_{std::move(native_source)} {}

void SoundSource::setPosition(const Vec3f& position) const
{
  alSource3f(native_source_, AL_POSITION, position.x(), position.y(), position.z());
}

void SoundSource::setVelocity(const Vec3f& position) const
{
  alSource3f(native_source_, AL_VELOCITY, position.x(), position.y(), position.z());
}

void SoundSource::setGain(float gain) const { alSourcei(native_source_, AL_GAIN, gain); }

void SoundSource::setPitch(float pitch) const { alSourcei(native_source_, AL_PITCH, pitch); }

void SoundSource::setLooped(bool looped) const { alSourcei(native_source_, AL_LOOPING, looped ? ALC_TRUE : ALC_FALSE); }

expected<SoundSource, SoundSourceError>
SoundSource::create(const PlayerContext& context, const SoundSourceOptions& options)
{
  if (!context.setActive())
  {
    return make_unexpected(SoundSourceError::kInvalidPlayerContext);
  }

  source_handle_t native_source_id{0};
  alGenSources(1, &native_source_id);
  if (const auto error = alGetError(); error != AL_NO_ERROR)
  {
    SDE_LOG_DEBUG_FMT("%s", al_error_to_str(error));
    return make_unexpected(SoundSourceError::kBackendSourceCreationFailure);
  }

  SoundSource sound{NativeSourceID{native_source_id}};

  sound.setPosition(options.position);
  if (const auto error = alGetError(); error != AL_NO_ERROR)
  {
    SDE_LOG_DEBUG_FMT("setPosition: %s", al_error_to_str(error));
    return make_unexpected(SoundSourceError::kBackendSourceAttributeInitializationFailure);
  }

  sound.setVelocity(options.velocity);
  if (const auto error = alGetError(); error != AL_NO_ERROR)
  {
    SDE_LOG_DEBUG_FMT("setVelocity: %s", al_error_to_str(error));
    return make_unexpected(SoundSourceError::kBackendSourceAttributeInitializationFailure);
  }

  sound.setGain(options.gain);
  if (const auto error = alGetError(); error != AL_NO_ERROR)
  {
    SDE_LOG_DEBUG_FMT("setGain: %s", al_error_to_str(error));
    return make_unexpected(SoundSourceError::kBackendSourceAttributeInitializationFailure);
  }

  sound.setPitch(options.pitch);
  if (const auto error = alGetError(); error != AL_NO_ERROR)
  {
    SDE_LOG_DEBUG_FMT("setPitch: %s", al_error_to_str(error));
    return make_unexpected(SoundSourceError::kBackendSourceAttributeInitializationFailure);
  }

  sound.setLooped(options.looped);
  if (const auto error = alGetError(); error != AL_NO_ERROR)
  {
    SDE_LOG_DEBUG_FMT("alGetError: %s", al_error_to_str(error));
    return make_unexpected(SoundSourceError::kBackendSourceAttributeInitializationFailure);
  }

  return sound;
}


bool SoundSource::isPaused() const
{
  if (empty())
  {
    return false;
  }
  ALint source_state;
  alGetSourcei(native_source_, AL_SOURCE_STATE, &source_state);
  return source_state == AL_PAUSED;
}

bool SoundSource::isPlaying() const
{
  if (empty())
  {
    return false;
  }
  ALint source_state;
  alGetSourcei(native_source_, AL_SOURCE_STATE, &source_state);
  return source_state == AL_PLAYING;
}

void SoundSource::play(const SoundInfo& sound, const bool start_paused)
{
  if (sound.native_id == current_sound_)
  {
    alSourceStop(native_source_);
    alSourceRewind(native_source_);
  }
  else
  {
    current_sound_ = sound.native_id;
    alSourcei(native_source_, AL_BUFFER, current_sound_);
  }

  if (start_paused)
  {
    return;
  }

  alSourcePlay(native_source_);
}

void SoundSource::stop()
{
  if (empty())
  {
    return;
  }
  alSourceStop(native_source_);
  current_sound_ = 0;
}

void SoundSource::pause() const
{
  if (empty())
  {
    return;
  }
  alSourcePause(native_source_);
}

void SoundSource::resume() const
{
  if (empty())
  {
    return;
  }
  alSourcePlay(native_source_);
}

}  // namespace sde::audio
