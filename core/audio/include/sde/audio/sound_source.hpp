/**
 * @copyright 2024-present Brian Cairl
 *
 * @file sound.hpp
 */
#pragma once

// C++ Standard Library

// SDE
#include "sde/audio/player_fwd.hpp"
#include "sde/audio/sound_fwd.hpp"
#include "sde/audio/typedef.hpp"
#include "sde/expected.hpp"
#include "sde/geometry_types.hpp"


namespace sde::audio
{

struct NativeSourceDeleter
{
  void operator()(source_handle_t id) const;
};

using NativeSourceID = UniqueResource<source_handle_t, NativeSourceDeleter>;

enum class SoundSourceError
{
  kInvalidPlayerContext,
  kBackendSourceCreationFailure,
  kBackendSourceAttributeInitializationFailure
};

struct SoundSourceOptions
{
  Vec3f position = Vec3f::Zero();
  Vec3f velocity = Vec3f::Zero();
  float gain = 1.F;
  float pitch = 1.F;
  bool looped = false;
};

// TODO listeners (contexts) can between treated like render targets
// TODO sound sources can be treated like texture units (limited)

class SoundSource
{
public:
  SoundSource(SoundSource&& other) = default;
  SoundSource& operator=(SoundSource&& other) = default;

  [[nodiscard]] static expected<SoundSource, SoundSourceError>
  create(const PlayerContext& context, const SoundSourceOptions& options = {});

  void setPosition(const Vec3f& position) const;

  void setVelocity(const Vec3f& position) const;

  void setGain(float gain) const;

  void setPitch(float pitch) const;

  void setLooped(bool looped) const;

  [[nodiscard]] bool empty() const { return current_sound_ != 0; }

  [[nodiscard]] bool isPaused() const;

  [[nodiscard]] bool isPlaying() const;

  void play(const SoundInfo& sound, bool start_paused = false);

  void resume() const;

  void pause() const;

  void stop();

private:
  explicit SoundSource(NativeSourceID&& native_source);
  SoundSource(const SoundSource& other) = delete;
  SoundSource& operator=(const SoundSource& other) = delete;

  NativeSourceID native_source_;
  buffer_handle_t current_sound_ = 0;
};

}  // namespace sde::audio
