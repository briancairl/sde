/**
 * @copyright 2024-present Brian Cairl
 *
 * @file track.hpp
 */
#pragma once

// SDE
#include "sde/audio/sound_fwd.hpp"
#include "sde/audio/typedef.hpp"
#include "sde/geometry.hpp"
#include "sde/resource.hpp"
#include "sde/unique_resource.hpp"
#include "sde/vector.hpp"

namespace sde::audio
{
struct NativeSourceDeleter
{
  void operator()(source_handle_t id) const;
};

using NativeSource = UniqueResource<source_handle_t, NativeSourceDeleter>;

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
  ~TrackPlayback();

  TrackPlayback() = default;
  TrackPlayback(std::size_t instance_id, Track& track);

  TrackPlayback(TrackPlayback&& other);
  TrackPlayback& operator=(TrackPlayback&& other);

  bool isValid() const { return (track_ != nullptr) and (instance_id_ == track_->instance()); }
  bool setPosition(const Vec3f& position) const;
  bool setVelocity(const Vec3f& velocity) const;
  bool setVolume(float level) const;
  bool setPitch(float level) const;
  bool setLooped(bool looped = true) const;

  bool resume() const;
  bool pause() const;
  bool stop();

  void swap(TrackPlayback& other);

  const Track* track() const { return track_; }

private:
  TrackPlayback(const TrackPlayback& other) = default;
  TrackPlayback& operator=(const TrackPlayback& other) = default;

  std::size_t instance_id_ = 0;
  const Track* track_ = nullptr;
};

}  // namespace sde::audio
