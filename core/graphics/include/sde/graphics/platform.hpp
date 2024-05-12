/**
 * @copyright 2024-present Brian Cairl
 *
 * @file platform.hpp
 */
#pragma once

// C++ Stnadard Library
#include <functional>

namespace sde::graphics
{

// TODO() move to window module
struct WindowOptions
{
  const char* title = "sde";
  int initial_width = 640;
  int initial_height = 480;
};

struct WindowProperties
{
  int width = 640;
  int height = 480;
  double mouse_x = 0;
  double mouse_y = 0;
};

// TODO() move to window module
struct WindowHandle
{
public:
  constexpr operator bool() const { return p_ != nullptr; }
  constexpr explicit WindowHandle(void* p) : p_{p} {}

  ~WindowHandle();

  WindowHandle(WindowHandle&&);

  void spin(std::function<void(const WindowProperties&)> on_update);

private:
  WindowHandle(const WindowHandle&) = delete;
  void* p_ = nullptr;
};

// TODO() move to window module; pass in window as target (fwd decl)
WindowHandle initialize(const WindowOptions& options = {});

}  // namespace sde::graphics
