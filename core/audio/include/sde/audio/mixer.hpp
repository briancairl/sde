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
#include "sde/audio/track.hpp"
#include "sde/audio/typedef.hpp"
#include "sde/expected.hpp"
#include "sde/geometry.hpp"
#include "sde/resource.hpp"
#include "sde/unique_resource.hpp"
#include "sde/vector.hpp"
#include "sde/view.hpp"

namespace sde::audio
{

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

struct ListenerOptions : Resource<ListenerOptions>
{
  std::size_t track_count = 16;

  auto field_list() { return FieldList(Field{"track_count", track_count}); }
};

class ListenerTarget;

enum class TrackPlaybackError
{
  kNoFreeSources
};

std::ostream& operator<<(std::ostream& os, TrackPlaybackError error);

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
