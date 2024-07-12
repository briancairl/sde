/**
 * @copyright 2024-present Brian Cairl
 *
 * @file player.hpp
 */
#pragma once

// C++ Standard Library
#include <array>
#include <vector>

// SDE
#include "sde/audio/sound_fwd.hpp"
#include "sde/audio/typedef.hpp"
#include "sde/expected.hpp"
#include "sde/geometry_types.hpp"
#include "sde/resource_wrapper.hpp"
#include "sde/view.hpp"

namespace sde::audio
{

struct NativeDeviceDeleter
{
  void operator()(device_handle_t id) const;
};

using NativeDevice = UniqueResource<device_handle_t, NativeDeviceDeleter>;

struct NativeContextDeleter
{
  void operator()(context_handle_t id) const;
};

using NativeContext = UniqueResource<context_handle_t, NativeContextDeleter>;

struct NativeSourceDeleter
{
  void operator()(source_handle_t id) const;
};

using NativeSource = UniqueResource<source_handle_t, NativeSourceDeleter>;

struct QueuedSoundDeleter
{
  void operator()(buffer_handle_t& id) const { id = 0; }
};

struct TrackOptions
{
  Vec3f position = Vec3f::Zero();
  Vec3f velocity = Vec3f::Zero();
  Vec3f orientation = Vec3f::UnitZ();
  float volume = 1.F;
  float pitch = 1.F;
  float cutoff_distance = 0.0F;
  bool looped = false;
};

class TrackPlayback;

class Track
{
public:
  explicit Track(NativeSource&& source);

  bool queued() const { return playback_queued_; };
  bool stopped() const;
  bool playing() const;
  float progress() const;

  TrackPlayback set(const Sound& sound, const TrackOptions& track_options);
  void pop(std::vector<source_handle_t>& target);

  const NativeSource& source() const { return source_; }
  const std::size_t instance() const { return instance_counter_; }

private:
  std::size_t instance_counter_ = 0;
  NativeSource source_;
  bool playback_queued_ = false;
  std::size_t playback_buffer_length_ = 0;
};

class TrackPlayback
{
public:
  TrackPlayback() = default;
  TrackPlayback(std::size_t instance_id, Track& track);

  bool isValid() const { return (track_ != nullptr) and (instance_id_ == track_->instance()); }
  bool setPosition(const Vec3f& position) const;
  bool setVelocity(const Vec3f& velocity) const;
  bool setVolume(float level) const;
  bool setPitch(float level) const;
  bool resume() const;
  bool pause() const;
  bool stop();

private:
  std::size_t instance_id_ = 0;
  const Track* track_ = nullptr;
};

enum class ListenerError
{
  kBackendContextCreationFailure,
  kBackendTrackCreationFailure,
};

struct ListenerState
{
  float gain = 0.5F;
  Vec3f position = Vec3f::Zero();
  Vec3f velocity = Vec3f::Zero();
  Vec3f orientation_at = Vec3f::UnitX();
  Vec3f orientation_up = Vec3f::UnitZ();
};

struct ListenerOptions
{
  std::size_t track_count = 16;
};

class ListenerTarget;

enum class TrackPlaybackError
{
  kNoFreeSources
};

class Listener
{
  friend class ListenerTarget;

public:
  [[nodiscard]] static expected<Listener, ListenerError>
  create(const NativeDevice& device, const ListenerOptions& options);

  void set(const ListenerState& state) const;
  expected<TrackPlayback, TrackPlaybackError> set(const Sound& sound, const TrackOptions& options);
  void play();
  void stop();

private:
  Listener(NativeContext&& context, std::vector<Track>&& tracks);

  NativeContext context_;
  std::vector<Track> tracks_;
  std::vector<source_handle_t> source_buffer_;
};

class Mixer;

enum class ListenerTargetError
{
  kListenerAlreadyActive,
  kListenerIDInvalid,
  kBackendListenerContextSwitch,
};


class ListenerTarget
{
public:
  ~ListenerTarget();
  ListenerTarget(ListenerTarget&& other);
  ListenerTarget& operator=(ListenerTarget&& other);

  void swap(ListenerTarget& other);

  [[nodiscard]] static expected<ListenerTarget, ListenerTargetError> create(Mixer& mixer, std::size_t listener_id);

  void set(const ListenerState& state) const { l_->set(state); }
  expected<TrackPlayback, TrackPlaybackError> set(const Sound& sound, const TrackOptions& options = {})
  {
    return l_->set(sound, options);
  }

private:
  explicit ListenerTarget(Mixer* m, Listener* p);
  ListenerTarget() = delete;
  ListenerTarget(const ListenerTarget& other) = delete;
  ListenerTarget& operator=(const ListenerTarget& other) = delete;

  Mixer* m_ = nullptr;
  Listener* l_ = nullptr;
};

struct MixerOptions
{
  const char* device_name = nullptr;
  std::vector<ListenerOptions> listener_options = {ListenerOptions{2}, ListenerOptions{16}};
};

enum class MixerError
{
  kBackendCannotOpenDevice,
  kListenerConfigInvalid,
  kListenerCreationFailure,
};

/**
 * @brief High-level interface for sound playback
 */
class Mixer
{
  friend class ListenerTarget;

public:
  ~Mixer() = default;
  Mixer(Mixer&& other) = default;

  [[nodiscard]] static expected<Mixer, MixerError> create(const MixerOptions& options = {});

  std::size_t size() const { return listeners_.size(); }

private:
  Mixer(NativeDevice&& device, std::vector<Listener>&& listener);
  Mixer() = delete;
  Mixer(const Mixer&) = delete;

  NativeDevice device_;
  std::vector<Listener> listeners_;
  Listener* listener_active_ = nullptr;
};

}  // namespace sde::audio
