/**
 * @copyright 2024-present Brian Cairl
 *
 * @file platform.hpp
 */
#pragma once

// C++ Standard Library
#include <bitset>
#include <chrono>
#include <functional>

// SDE
#include "sde/expected.hpp"
#include "sde/geometry_types.hpp"
#include "sde/graphics/window.hpp"

namespace sde
{

enum class KeyCode : std::size_t
{
  kNum1,
  kNum2,
  kNum3,
  kNum4,
  kNum5,
  kNum6,
  kNum7,
  kNum8,
  kNum9,
  kNum0,
  kQ,
  kW,
  kE,
  kA,
  kS,
  kD,
  kZ,
  kX,
  kC,
  kSpace,
  kLShift,
  kRShift,
  kLCtrl,
  kRCtrl,
  kLAlt,
  kRAlt,
  _Count_
};

static const auto kKeyCount = static_cast<std::size_t>(KeyCode::_Count_);

struct AppKeyStates
{
  std::bitset<kKeyCount> down{0};
  std::bitset<kKeyCount> pressed{0};
  std::bitset<kKeyCount> released{0};

  constexpr bool isDown(KeyCode code) const { return down[static_cast<std::size_t>(code)]; }
  constexpr bool isPressed(KeyCode code) const { return pressed[static_cast<std::size_t>(code)]; }
  constexpr bool isReleased(KeyCode code) const { return released[static_cast<std::size_t>(code)]; }
};

struct AppProperties
{
  using Clock = std::chrono::steady_clock;
  using Duration = Clock::duration;

  Duration time = Duration::zero();
  Duration time_delta = Duration::zero();

  Vec2i size = {640, 480};
  Vec2d mouse_position_px = {0.0, 0.0};
  Vec2d mouse_scroll = {0.0, 0.0};

  AppKeyStates keys;

  Vec2f getMousePositionViewport(Vec2i viewport_size) const
  {
    return {
      static_cast<float>(2.0 * mouse_position_px.x() / static_cast<double>(viewport_size.x()) - 1.0),
      static_cast<float>(1.0 - 2.0 * mouse_position_px.y() / static_cast<double>(viewport_size.y()))};
  }
};

enum class AppDirective
{
  kContinue,
  kReset,
  kClose
};

enum class AppError
{
  kWindowInvalid,
  kWindowCreationFailure,
};

class App
{
public:
  using Window = graphics::Window;
  using WindowOptions = graphics::WindowOptions;

  App(App&&) = default;

  void spin(std::function<AppDirective(const AppProperties&)> on_update);

  const Window& window() const { return window_; }

  static expected<App, AppError> create(Window&& window);

  static expected<App, AppError> create(const WindowOptions& window_options);

private:
  explicit App(Window&& window);
  App(const App&) = delete;
  Window window_;
};

}  // namespace sde
