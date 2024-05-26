/**
 * @copyright 2024-present Brian Cairl
 *
 * @file platform.hpp
 */
#pragma once

// C++ Standard Library
#include <chrono>
#include <functional>

// SDE
#include "sde/geometry_types.hpp"

namespace sde::graphics
{

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
