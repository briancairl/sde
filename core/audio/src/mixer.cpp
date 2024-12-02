// C++ Standard Library
#include <algorithm>
#include <ostream>

// Backend
#include "openal.inl"

// SDE
#include "sde/audio/mixer.hpp"
#include "sde/audio/sound.hpp"
#include "sde/audio/sound_device.hpp"
#include "sde/logging.hpp"
#include "sde/resource_wrapper.hpp"

namespace sde::audio
{

std::ostream& operator<<(std::ostream& os, ListenerError error)
{
  switch (error)
  {
    SDE_OS_ENUM_CASE(ListenerError::kBackendContextCreationFailure)
    SDE_OS_ENUM_CASE(ListenerError::kBackendTrackCreationFailure)
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, ListenerTargetError error)
{
  switch (error)
  {
    SDE_OS_ENUM_CASE(ListenerTargetError::kListenerAlreadyActive)
    SDE_OS_ENUM_CASE(ListenerTargetError::kListenerIDInvalid)
    SDE_OS_ENUM_CASE(ListenerTargetError::kBackendListenerContextSwitch)
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, MixerError error)
{
  switch (error)
  {
    SDE_OS_ENUM_CASE(MixerError::kBackendCannotOpenDevice)
    SDE_OS_ENUM_CASE(MixerError::kListenerConfigInvalid)
    SDE_OS_ENUM_CASE(MixerError::kListenerCreationFailure)
  }
  return os;
}

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

expected<Listener, ListenerError> Listener::create(NativeSoundDeviceHandle device, const ListenerOptions& options)
{
  // Create listener context
  const auto native_context_handle = alcCreateContext(reinterpret_cast<ALCdevice*>(device), NULL);
  SDE_LOG_DEBUG_FMT("alcCreateContext(%p, NULL) -> %p", device, native_context_handle);
  if (native_context_handle == nullptr)
  {
    SDE_LOG_DEBUG_FMT("alcCreateContext: %p", native_context_handle);
    return make_unexpected(ListenerError::kBackendContextCreationFailure);
  }

  // Set context as current context
  NativeContext context{reinterpret_cast<void*>(native_context_handle)};
  if (alcMakeContextCurrent(native_context_handle) != ALC_TRUE)
  {
    SDE_LOG_DEBUG_FMT("alcMakeContextCurrent: %p", native_context_handle);
    return make_unexpected(ListenerError::kBackendContextCreationFailure);
  }

  alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);

  // Create create sources attached to this context
  sde::vector<Track> tracks;
  tracks.reserve(options.track_count);
  for (std::size_t t = 0; t < options.track_count; ++t)
  {
    source_handle_t native_source_id{0};
    alGenSources(1, &native_source_id);
    SDE_LOG_DEBUG_FMT(
      "alGenSources(1, &native_source_id = %p) -> native_source_id := %d",
      &native_source_id,
      static_cast<int>(native_source_id));
    if (const auto error = alGetError(); error != AL_NO_ERROR)
    {
      SDE_LOG_DEBUG_FMT("%s", al_error_to_str(error));
      return make_unexpected(ListenerError::kBackendTrackCreationFailure);
    }
    tracks.emplace_back(NativeSource{native_source_id});
  }
  return Listener{std::move(context), std::move(tracks)};
}

Listener::Listener(NativeContext&& context, sde::vector<Track>&& tracks) :
    context_{std::move(context)}, tracks_{std::move(tracks)}
{
  source_buffer_.reserve(tracks_.size());
}

void Listener::set(const ListenerState& state) const
{
  alcMakeContextCurrent(reinterpret_cast<ALCcontext*>(context_.value()));
  alListenerfv(AL_POSITION, state.position.data());
  alListenerfv(AL_VELOCITY, state.velocity.data());
  ALfloat orientation_buffer[6];
  std::copy(
    state.orientation_at.data(), state.orientation_at.data() + state.orientation_at.size(), (orientation_buffer + 0));
  std::copy(
    state.orientation_up.data(), state.orientation_up.data() + state.orientation_up.size(), (orientation_buffer + 3));
  alListenerfv(AL_ORIENTATION, orientation_buffer);
  alListenerf(AL_GAIN, state.gain);
}

expected<TrackPlayback, TrackPlaybackError> Listener::set(const Sound& sound, const TrackOptions& options)
{
  const auto track_itr = std::find_if(
    std::begin(tracks_), std::end(tracks_), [](const auto& track) { return !track.queued() and track.stopped(); });
  if (track_itr == std::end(tracks_))
  {
    SDE_LOG_DEBUG_FMT(
      "Listener::set(Sound{%d}, ...) failed; no sources free", static_cast<int>(sound.native_id.value()));
    return make_unexpected(TrackPlaybackError::kNoFreeSources);
  }
  SDE_LOG_DEBUG_FMT("Listener::set(Sound{%d}, ...) succeeded", static_cast<int>(sound.native_id.value()));
  return track_itr->set(sound, options);
}

void Listener::play()
{
  alcMakeContextCurrent(reinterpret_cast<ALCcontext*>(context_.value()));
  for (auto& track : tracks_)
  {
    track.pop(source_buffer_);
  }
  if (source_buffer_.empty())
  {
    return;
  }
  alSourcePlayv(source_buffer_.size(), source_buffer_.data());
  SDE_ASSERT_EQ(alGetError(), AL_NO_ERROR);
  SDE_LOG_DEBUG() << "alSourcePlayv(" << source_buffer_.size() << ", " << source_buffer_.data() << ')';
  source_buffer_.clear();
}

void Listener::stop()
{
  alcMakeContextCurrent(reinterpret_cast<ALCcontext*>(context_.value()));
  for (const auto& track : tracks_)
  {
    if (track.playing())
    {
      source_buffer_.push_back(track.source());
    }
  }
  if (source_buffer_.empty())
  {
    return;
  }
  alSourceStopv(source_buffer_.size(), source_buffer_.data());
  SDE_ASSERT_EQ(alGetError(), AL_NO_ERROR);
  SDE_LOG_DEBUG() << "alSourceStopv(" << source_buffer_.size() << ", " << source_buffer_.data() << ')';
  source_buffer_.clear();
}

expected<ListenerTarget, ListenerTargetError> ListenerTarget::create(Mixer& mixer, std::size_t listener_id)
{
  if (mixer.listener_active_ != nullptr)
  {
    SDE_LOG_ERROR() << "ListenerAlreadyActive: " << SDE_OSNV(mixer.listener_active_);
    return make_unexpected(ListenerTargetError::kListenerAlreadyActive);
  }

  if (listener_id >= mixer.listeners_.size())
  {
    SDE_LOG_ERROR() << "ListenerIDInvalid : " << SDE_OSNV(listener_id) << "(of " << SDE_OSNV(mixer.listeners_.size())
                    << ')';
    return make_unexpected(ListenerTargetError::kListenerIDInvalid);
  }

  auto* listener_p = mixer.listeners_.data() + listener_id;
  if (alcMakeContextCurrent(reinterpret_cast<ALCcontext*>(listener_p->context_.value())) != ALC_TRUE)
  {
    SDE_LOG_ERROR() << "alcMakeContextCurrent(" << listener_p->context_.value() << ')';
    return make_unexpected(ListenerTargetError::kBackendListenerContextSwitch);
  }
  return ListenerTarget{&mixer, listener_p};
}

ListenerTarget::ListenerTarget(Mixer* m, Listener* p) : m_{m}, l_{p} { m_->listener_active_ = p; }

ListenerTarget::ListenerTarget(ListenerTarget&& other) : m_{nullptr}, l_{nullptr} { this->swap(other); }

ListenerTarget::~ListenerTarget()
{
  if (m_ == nullptr)
  {
    return;
  }
  l_->play();
  m_->listener_active_ = nullptr;
}

ListenerTarget& ListenerTarget::operator=(ListenerTarget&& other)
{
  this->swap(other);
  return *this;
}

void ListenerTarget::swap(ListenerTarget& other)
{
  std::swap(this->m_, other.m_);
  std::swap(this->l_, other.l_);
}


expected<Mixer, MixerError> Mixer::create(const SoundDevice& sound_device, const MixerOptions& options)
{
  return Mixer::create(sound_device.handle(), options);
}

expected<Mixer, MixerError> Mixer::create(NativeSoundDeviceHandle sound_device, const MixerOptions& options)
{
  if (options.listener_options.empty())
  {
    SDE_LOG_ERROR() << "ListenerConfigInvalid";
    return make_unexpected(MixerError::kListenerConfigInvalid);
  }

  // Create mixer listener
  sde::vector<Listener> listeners;
  listeners.reserve(options.listener_options.size());
  for (const auto& options : options.listener_options)
  {
    SDE_LOG_DEBUG() << "Listener::create(" << listeners.size() << ')';
    if (auto listener_or_error = Listener::create(sound_device, options); listener_or_error.has_value())
    {
      listeners.push_back(std::move(listener_or_error).value());
    }
    else
    {
      SDE_LOG_ERROR() << "ListenerCreationFailure: " << listener_or_error.error();
      return make_unexpected(MixerError::kListenerCreationFailure);
    }
  }

  // Create the mixer
  return Mixer{std::move(listeners)};
}

Mixer::Mixer(sde::vector<Listener>&& listeners) : listeners_{std::move(listeners)} {}


}  // namespace sde::audio
