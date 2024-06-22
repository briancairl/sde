// C++ Standard Library
#include <algorithm>
#include <ostream>

// Backend
#include "openal.inl"

// SDE
#include "sde/audio/mixer.hpp"
#include "sde/audio/sound.hpp"
#include "sde/logging.hpp"
#include "sde/resource_wrapper.hpp"

namespace sde::audio
{

void NativeSourceDeleter::operator()(source_handle_t id) const
{
  if (id != 0)
  {
    alDeleteSources(1, &id);
  }
}

void NativeDeviceDeleter::operator()(device_handle_t id) const
{
  if (id != nullptr)
  {
    SDE_LOG_DEBUG_FMT("device closed: %p", id);
    alcCloseDevice(reinterpret_cast<ALCdevice*>(id));
  }
}

void NativeContextDeleter::operator()(context_handle_t id) const
{
  if (id != nullptr)
  {
    alcDestroyContext(reinterpret_cast<ALCcontext*>(id));
  }
}

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
  ALint byte_offset;
  alGetSourcei(source_, AL_BYTE_OFFSET, &byte_offset);
  return static_cast<float>(byte_offset) / static_cast<float>(playback_buffer_length_);
}

void Track::set(const SoundInfo& sound, const TrackOptions& track_options)
{
  playback_queued_ = true;
  playback_buffer_length_ = sound.buffer_length;

  SDE_LOG_DEBUG_FMT("alSourcei(%d, AL_BUFFER, %d)", source_.value(), static_cast<int>(sound.native_id.value()));

  alSourcei(source_, AL_BUFFER, sound.native_id);
  alSourcei(source_, AL_LOOPING, track_options.looped);
  alSourcef(source_, AL_GAIN, track_options.gain);
  alSourcef(source_, AL_PITCH, track_options.pitch);
  alSourcefv(source_, AL_POSITION, track_options.position.data());
  alSourcefv(source_, AL_VELOCITY, track_options.velocity.data());
}

source_handle_t Track::pop()
{
  source_handle_t playback_source{0};
  if (playback_queued_)
  {
    playback_source = source_.value();
    playback_queued_ = false;
  }
  return playback_source;
}

expected<Listener, ListenerError> Listener::create(const NativeDevice& device, const ListenerOptions& options)
{
  // Create listener context
  const auto native_context_handle = alcCreateContext(reinterpret_cast<ALCdevice*>(device.value()), NULL);
  SDE_LOG_DEBUG_FMT("alcCreateContext(%p, NULL) -> %p", device.value(), native_context_handle);
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

  // Create create sources attached to this context
  std::vector<Track> tracks;
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

Listener::Listener(NativeContext&& context, std::vector<Track>&& tracks) :
    context_{std::move(context)}, tracks_{std::move(tracks)}
{
  source_buffer_.reserve(tracks_.size());
}

void Listener::set(const ListenerState& state) const
{
  alcMakeContextCurrent(reinterpret_cast<ALCcontext*>(context_.value()));
  alListenerfv(AL_POSITION, state.position.data());  // centered
  alListenerfv(AL_VELOCITY, state.velocity.data());  // stationary
  alListenerfv(AL_ORIENTATION, state.orientation_up.data());
}

bool Listener::set(const SoundInfo& sound, const TrackOptions& options)
{
  const auto track_itr = std::find_if(
    std::begin(tracks_), std::end(tracks_), [](const auto& track) { return !track.queued() and track.stopped(); });
  if (track_itr == std::end(tracks_))
  {
    SDE_LOG_DEBUG_FMT(
      "Listener::set(SoundInfo{%d}, ...) failed; no sources free", static_cast<int>(sound.native_id.value()));
    return false;
  }
  track_itr->set(sound, options);
  return true;
}

void Listener::play()
{
  alcMakeContextCurrent(reinterpret_cast<ALCcontext*>(context_.value()));
  for (auto& track : tracks_)
  {
    if (auto playback_source = track.pop(); playback_source != 0)
    {
      source_buffer_.push_back(playback_source);
    }
  }
  if (source_buffer_.empty())
  {
    return;
  }
  alSourcePlayv(source_buffer_.size(), source_buffer_.data());
  SDE_ASSERT_EQ(alGetError(), AL_NO_ERROR);
  SDE_LOG_DEBUG_FMT("alSourcePlayv(%lu, %p)", source_buffer_.size(), source_buffer_.data());
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
  SDE_LOG_DEBUG_FMT("alSourceStopv(%lu, %p)", source_buffer_.size(), source_buffer_.data());
  source_buffer_.clear();
}

expected<Mixer, MixerError> Mixer::create(const MixerOptions& options)
{
  if (options.listener_options.empty())
  {
    SDE_LOG_DEBUG("ListenerConfigInvalid");
    return make_unexpected(MixerError::kListenerConfigInvalid);
  }

  // Initalize sound device
  const auto native_device_handle = [&options] {
    if (options.device_name == nullptr)
    {
      const char* default_device_name = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
      SDE_LOG_DEBUG_FMT("alcOpenDevice: %s", default_device_name);
      return alcOpenDevice(default_device_name);
    }
    else
    {
      return alcOpenDevice(options.device_name);
    }
  }();
  if (native_device_handle == nullptr)
  {
    SDE_LOG_DEBUG_FMT("alcOpenDevice: %p", native_device_handle);
    return make_unexpected(MixerError::kBackendCannotOpenDevice);
  }

  NativeDevice device{reinterpret_cast<void*>(native_device_handle)};

  // Create mixer listener
  std::vector<Listener> listeners;
  listeners.reserve(options.listener_options.size());
  for (const auto& options : options.listener_options)
  {
    SDE_LOG_DEBUG_FMT("Listener::create(%lu)", listeners.size());
    if (auto groul_or_error = Listener::create(device, options); groul_or_error.has_value())
    {
      listeners.push_back(std::move(groul_or_error).value());
    }
    else
    {
      SDE_LOG_DEBUG("ListenerCreationFailure");
      return make_unexpected(MixerError::kListenerCreationFailure);
    }
  }

  // Create the mixer
  return Mixer{std::move(device), std::move(listeners)};
}

expected<ListenerTarget, ListenerTargetError> ListenerTarget::create(Mixer& mixer, std::size_t listener_id)
{
  if (mixer.listener_active_ != nullptr)
  {
    SDE_LOG_DEBUG_FMT("ListenerAlreadyActive : %p", mixer.listener_active_);
    return make_unexpected(ListenerTargetError::kListenerAlreadyActive);
  }

  if (listener_id >= mixer.listeners_.size())
  {
    SDE_LOG_DEBUG_FMT("ListenerIDInvalid : %lu (of %lu)", listener_id, mixer.listeners_.size());
    return make_unexpected(ListenerTargetError::kListenerIDInvalid);
  }

  auto* listener_p = mixer.listeners_.data() + listener_id;
  if (alcMakeContextCurrent(reinterpret_cast<ALCcontext*>(listener_p->context_.value())) != ALC_TRUE)
  {
    SDE_LOG_DEBUG_FMT("alcMakeContextCurrent(%p)", listener_p->context_.value());
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

Mixer::Mixer(NativeDevice&& device, std::vector<Listener>&& listeners) :
    device_{std::move(device)}, listeners_{std::move(listeners)}
{}

}  // namespace sde::audio
