/**
 * @copyright 2024-present Brian Cairl
 *
 * @file player.hpp
 */
#pragma once

// SDE
#include "sde/audio/player_fwd.hpp"
#include "sde/audio/typedef.hpp"
#include "sde/expected.hpp"
#include "sde/resource_wrapper.hpp"
#include "sde/view.hpp"

namespace sde::audio
{

struct PlayerOptions
{
  const char* device_name = nullptr;
};

struct NativeDeviceDeleter
{
  void operator()(device_handle_t id) const;
};

using NativeDevice = UniqueResource<device_handle_t, NativeDeviceDeleter>;

struct NativeContextDeleter
{
  void operator()(context_handle_t id) const;
};

struct PlayerContext : UniqueResource<context_handle_t, NativeContextDeleter>
{
  explicit PlayerContext(context_handle_t handle);
  bool setActive() const;
};

enum class PlayerError
{
  kBackendCannotOpenDevice,
  kBackendFailedContextCreation,
};

/**
 * @brief High-level interface for sound playback
 */
class Player
{
public:
  ~Player() = default;
  Player(Player&& other) = default;

  [[nodiscard]] static expected<Player, PlayerError> create(const PlayerOptions& options = {});

  constexpr const PlayerContext& context() const { return context_; }

private:
  Player(NativeDevice&& device, PlayerContext&& context);
  Player() = delete;
  Player(const Player&) = delete;

  NativeDevice device_;
  PlayerContext context_;
};

}  // namespace sde::audio
