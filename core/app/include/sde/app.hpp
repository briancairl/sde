/**
 * @copyright 2024-present Brian Cairl
 *
 * @file app.hpp
 */
#pragma once

// C++ Standard Library
#include <chrono>
#include <functional>
#include <iosfwd>

// SDE
#include "sde/app_fwd.hpp"
#include "sde/app_properties.hpp"
#include "sde/audio/sound_device.hpp"
#include "sde/expected.hpp"
#include "sde/geometry.hpp"
#include "sde/graphics/window.hpp"
#include "sde/keyboard.hpp"
#include "sde/time.hpp"

namespace sde
{

enum class AppDirective
{
  kContinue,
  kReset,
  kClose
};

std::ostream& operator<<(std::ostream& os, AppDirective directive);

enum class AppError
{
  kWindowInvalid,
  kWindowCreationFailure,
  kSoundDeviceInvalid,
  kSoundDeviceCreationFailure,
};

std::ostream& operator<<(std::ostream& os, AppError error);

class App
{
public:
  using SoundDevice = audio::SoundDevice;
  using Window = graphics::Window;
  using WindowOptions = graphics::WindowOptions;
  using OnStart = std::function<AppDirective(const AppProperties&)>;
  using OnUpdate = std::function<AppDirective(const AppProperties&)>;

  App(App&&) = default;

  void spin(OnStart on_start, OnUpdate on_update, const Rate spin_rate = Hertz(60.0F));

  const Window& window() const { return window_; }

  static expected<App, AppError> create(Window&& window, SoundDevice&& sound_device);

  static expected<App, AppError> create(const WindowOptions& window_options);

private:
  App(Window&& window, SoundDevice&& sound_device);
  App(const App&) = delete;
  Window window_;
  SoundDevice sound_device_;
};

}  // namespace sde
