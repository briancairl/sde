// Backend
#include "openal.inl"

// SDE
#include "sde/audio/sound.hpp"
#include "sde/audio/track.hpp"
#include "sde/logging.hpp"

namespace sde::audio
{

void NativeSourceDeleter::operator()(source_handle_t id) const { alDeleteSources(1, &id); }

Track::Track(NativeSource&& source) : source_{std::move(source)}, playback_queued_{false}, playback_buffer_length_{0} {}

bool Track::stopped() const
{
  ALint source_state;
  alGetSourcei(source_, AL_SOURCE_STATE, &source_state);
  return (source_state == AL_STOPPED) or (source_state == AL_INITIAL);
}

bool Track::playing() const
{
  ALint source_state;
  alGetSourcei(source_, AL_SOURCE_STATE, &source_state);
  return (source_state == AL_PLAYING) or (source_state == AL_LOOPING);
}

float Track::progress() const
{
  if (playback_buffer_length_ == 0)
  {
    return -1.0F;
  }
  else if (queued())
  {
    return 0.0F;
  }
  ALint byte_offset;
  alGetSourcei(source_, AL_BYTE_OFFSET, &byte_offset);
  return static_cast<float>(byte_offset) / static_cast<float>(playback_buffer_length_);
}

void Track::jump(float p) const
{
  if (playback_buffer_length_ == 0)
  {
    return;
  }
  const ALint byte_offset = static_cast<ALint>(std::clamp(p, 0.0F, 1.0F) * playback_buffer_length_);
  alSourcei(source_, AL_BYTE_OFFSET, byte_offset);
}

TrackPlayback Track::set(const Sound& sound, const TrackOptions& track_options)
{
  ++instance_counter_;

  playback_queued_ = true;
  playback_buffer_length_ = sound.buffer_length;

  SDE_LOG_DEBUG_FMT(
    "alSourcei(%d, AL_BUFFER, %d) : instance(%lu)",
    source_.value(),
    static_cast<int>(sound.native_id.value()),
    instance_counter_);

  alSourcei(source_, AL_BUFFER, sound.native_id);
  alSourcei(source_, AL_LOOPING, static_cast<ALint>(track_options.looped));
  alSourcei(source_, AL_SOURCE_RELATIVE, static_cast<ALint>(false));
  if (track_options.cutoff_distance > 0.F)
  {
    alSourcef(source_, AL_REFERENCE_DISTANCE, 0.2F * track_options.cutoff_distance);
    alSourcef(source_, AL_MAX_DISTANCE, track_options.cutoff_distance);
  }
  else
  {
    alSourcef(source_, AL_REFERENCE_DISTANCE, 1000.0F);
    alSourcef(source_, AL_MAX_DISTANCE, 1000.0F);
  }
  alSourcef(source_, AL_GAIN, track_options.volume);
  alSourcef(source_, AL_PITCH, track_options.pitch);
  alSourcefv(source_, AL_POSITION, track_options.position.data());
  alSourcefv(source_, AL_VELOCITY, track_options.velocity.data());
  alSourcefv(source_, AL_DIRECTION, track_options.orientation.data());

  return TrackPlayback{instance_counter_, *this};
}

void Track::pop(sde::vector<source_handle_t>& target)
{
  if (playback_queued_)
  {
    target.push_back(source_.value());
    playback_queued_ = false;
  }
}

TrackPlayback::TrackPlayback(std::size_t instance_id, Track& track) :
    instance_id_{instance_id}, track_{std::addressof(track)}
{}

bool TrackPlayback::setPosition(const Vec3f& position) const
{
  if (!isValid())
  {
    return false;
  }
  alSourcefv(track_->source(), AL_POSITION, position.data());
  return true;
}

bool TrackPlayback::setVelocity(const Vec3f& velocity) const
{
  if (!isValid())
  {
    return false;
  }
  alSourcefv(track_->source(), AL_VELOCITY, velocity.data());
  return true;
}

bool TrackPlayback::setVolume(float level) const
{
  if (!isValid())
  {
    return false;
  }
  alSourcef(track_->source(), AL_GAIN, level);
  return true;
}

bool TrackPlayback::setPitch(float level) const
{
  if (!isValid())
  {
    return false;
  }
  alSourcef(track_->source(), AL_PITCH, level);
  return true;
}

bool TrackPlayback::setLooped(bool looped) const
{
  if (!isValid())
  {
    return false;
  }
  alSourcei(track_->source(), AL_LOOPING, looped ? AL_TRUE : AL_FALSE);
  return true;
}

bool TrackPlayback::resume() const
{
  if (!isValid())
  {
    return false;
  }
  alSourcePlay(track_->source());
  return true;
}

bool TrackPlayback::pause() const
{
  if (!isValid())
  {
    return false;
  }
  alSourcePause(track_->source());
  return true;
}

bool TrackPlayback::stop()
{
  if (!isValid())
  {
    return false;
  }
  alSourceStop(track_->source());
  track_ = nullptr;
  return true;
}

void TrackPlayback::swap(TrackPlayback& other)
{
  std::swap(this->track_, other.track_);
  std::swap(this->instance_id_, other.instance_id_);
}

TrackPlayback::~TrackPlayback() { this->stop(); }

TrackPlayback::TrackPlayback(TrackPlayback&& other) : TrackPlayback{} { this->swap(other); }

TrackPlayback& TrackPlayback::operator=(TrackPlayback&& other)
{
  this->swap(other);
  return *this;
}

}  // namespace sde::audio
