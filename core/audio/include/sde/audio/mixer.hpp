/**
 * @copyright 2024-present Brian Cairl
 *
 * @file mixer.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>

// SDE
#include "sde/audio/sound_device.hpp"
#include "sde/audio/sound_fwd.hpp"
#include "sde/audio/typedef.hpp"
#include "sde/expected.hpp"
#include "sde/geometry.hpp"
#include "sde/resource.hpp"
#include "sde/unique_resource.hpp"
#include "sde/vector.hpp"
#include "sde/view.hpp"

namespace sde::audio
{


struct NativeSourceDeleter
{
  void operator()(source_handle_t id) const;
};

using NativeSource = UniqueResource<source_handle_t, NativeSourceDeleter>;

struct QueuedSoundDeleter
{
  void operator()(buffer_handle_t& id) const { id = 0; }
};

struct TrackOptions : Resource<TrackOptions>
{
  Vec3f position = Vec3f::Zero();
  Vec3f velocity = Vec3f::Zero();
  Vec3f orientation = Vec3f::UnitZ();
  float volume = 1.F;
  float pitch = 1.F;
  float cutoff_distance = 0.0F;
  bool looped = false;

  auto field_list()
  {
    return FieldList(
      Field{"position", position},
      Field{"velocity", velocity},
      Field{"orientation", orientation},
      Field{"volume", volume},
      Field{"pitch", pitch},
      Field{"cutoff_distance", cutoff_distance},
      Field{"looped", looped});
  }
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
  void jump(float p) const;

  TrackPlayback set(const Sound& sound, const TrackOptions& track_options);
  void pop(sde::vector<source_handle_t>& target);

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

  const Track* track() const { return track_; }

private:
  std::size_t instance_id_ = 0;
  const Track* track_ = nullptr;
};

enum class ListenerError
{
  kBackendContextCreationFailure,
  kBackendTrackCreationFailure,
};

std::ostream& operator<<(std::ostream& os, ListenerError error);

struct ListenerState : Resource<ListenerState>
{
  float gain = 0.5F;
  Vec3f position = Vec3f::Zero();
  Vec3f velocity = Vec3f::Zero();
  Vec3f orientation_at = Vec3f::UnitX();
  Vec3f orientation_up = Vec3f::UnitZ();

  auto field_list()
  {
    return FieldList(
      Field{"gain", gain},
      Field{"position", position},
      Field{"velocity", velocity},
      Field{"orientation_at", orientation_at},
      Field{"orientation_up", orientation_up});
  }
};

struct ListenerOptions : Resource<ListenerState>
{
  std::size_t track_count = 16;

  auto field_list() { return FieldList(Field{"track_count", track_count}); }
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
  create(NativeSoundDeviceHandle device, const ListenerOptions& options);

  void set(const ListenerState& state) const;
  expected<TrackPlayback, TrackPlaybackError> set(const Sound& sound, const TrackOptions& options);
  void play();
  void stop();

private:
  Listener(NativeContext&& context, sde::vector<Track>&& tracks);

  NativeContext context_;
  sde::vector<Track> tracks_;
  sde::vector<source_handle_t> source_buffer_;
};

class Mixer;

enum class ListenerTargetError
{
  kListenerAlreadyActive,
  kListenerIDInvalid,
  kBackendListenerContextSwitch,
};

std::ostream& operator<<(std::ostream& os, ListenerTargetError error);

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

struct MixerOptions : Resource<MixerOptions>
{
  sde::vector<ListenerOptions> listener_options = {
    ListenerOptions{.track_count = 2},
    ListenerOptions{.track_count = 16}};

  auto field_list() { return FieldList(Field{"listener_options", listener_options}); }
};

enum class MixerError
{
  kBackendCannotOpenDevice,
  kListenerConfigInvalid,
  kListenerCreationFailure,
};

std::ostream& operator<<(std::ostream& os, MixerError error);

/**
 * @brief High-level interface for sound playback
 */
class Mixer
{
  friend class ListenerTarget;

public:
  ~Mixer() = default;
  Mixer(Mixer&& other) = default;

  [[nodiscard]] static expected<Mixer, MixerError> create(const SoundDevice& device, const MixerOptions& options = {});

  [[nodiscard]] static expected<Mixer, MixerError>
  create(NativeSoundDeviceHandle device, const MixerOptions& options = {});

  std::size_t size() const { return listeners_.size(); }

private:
  explicit Mixer(sde::vector<Listener>&& listener);
  Mixer() = delete;
  Mixer(const Mixer&) = delete;

  sde::vector<Listener> listeners_;
  Listener* listener_active_ = nullptr;
};

}  // namespace sde::audio
