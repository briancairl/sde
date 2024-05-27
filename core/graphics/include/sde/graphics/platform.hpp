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
#include "sde/geometry_types.hpp"

namespace sde::graphics
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

struct WindowKeyStates
{
  std::bitset<kKeyCount> down{0};
  std::bitset<kKeyCount> pressed{0};
  std::bitset<kKeyCount> released{0};

  constexpr bool isDown(KeyCode code) const { return down[static_cast<std::size_t>(code)]; }
  constexpr bool isPressed(KeyCode code) const { return pressed[static_cast<std::size_t>(code)]; }
  constexpr bool isReleased(KeyCode code) const { return released[static_cast<std::size_t>(code)]; }
};

// TODO() move to window module
struct WindowOptions
{
  const char* title = "sde";
  Vec2i initial_size = {640, 480};
};

struct WindowProperties
{
  using Clock = std::chrono::steady_clock;
  using Duration = Clock::duration;

  Duration time = Duration::zero();
  Duration time_delta = Duration::zero();

  Vec2i size = {640, 480};
  Vec2d mouse_position_px = {0.0, 0.0};
  Vec2f mouse_position_vp = {0.0, 0.0};

  Vec2d mouse_scroll = {0.0, 0.0};

  WindowKeyStates keys;
};

enum class WindowDirective
{
  kContinue,
  kReset,
  kClose
};

// TODO() move to window module
struct WindowHandle
{
public:
  constexpr operator bool() const { return p_ != nullptr; }
  constexpr explicit WindowHandle(void* p) : p_{p} {}

  ~WindowHandle();

  WindowHandle(WindowHandle&&);

  void spin(std::function<WindowDirective(const WindowProperties&)> on_update);

private:
  WindowHandle(const WindowHandle&) = delete;
  void* p_ = nullptr;
};

// TODO() move to window module; pass in window as target (fwd decl)
WindowHandle initialize(const WindowOptions& options = {});

}  // namespace sde::graphics
