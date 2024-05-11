/**
 * @copyright 2024-present Brian Cairl
 *
 * @file platform.hpp
 */
#pragma once

namespace sde::graphics
{

// TODO() move to window module
struct WindowOptions
{
  const char* title = "sde";
  int initial_width = 640;
  int initial_height = 480;
};

// TODO() move to window module
struct WindowHandle
{
public:
  constexpr operator bool() const { return p_ != nullptr; }
  constexpr explicit WindowHandle(void* p) : p_{p} {}

  ~WindowHandle();

  WindowHandle(WindowHandle&&);

private:
  WindowHandle(const WindowHandle&) = delete;
  void* p_ = nullptr;
};

// TODO() move to window module; pass in window as target (fwd decl)
WindowHandle initialize(const WindowOptions& options = {});

}  // namespace sde::graphics
