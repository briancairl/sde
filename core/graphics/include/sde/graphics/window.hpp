/**
 * @copyright 2024-present Brian Cairl
 *
 * @file window.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>

// SDE
#include "sde/expected.hpp"
#include "sde/geometry.hpp"
#include "sde/graphics/image_ref.hpp"
#include "sde/graphics/window_fwd.hpp"
#include "sde/unique_resource.hpp"

namespace sde::graphics
{

struct WindowOptions
{
  const char* title = "sde";
  Vec2i initial_size = {640, 480};
};

enum class WindowError
{
  kWindowCreationFailed,
  kWindowIconInvalidPixelFormat,
  kWindowIconInvalidSize,
  kWindowCursorInvalidPixelFormat,
  kWindowCursorInvalidSize,
};

std::ostream& operator<<(std::ostream& os, WindowError error);

struct WindowDeleter
{
  void operator()(NativeWindowHandle id) const;
};

class Window : public UniqueResource<NativeWindowHandle, WindowDeleter>
{
public:
  static bool backend_initialized();
  static bool try_backend_initialization();

  static expected<Window, WindowError> create(const WindowOptions& options);

  void activate() const;

  bool poll() const;

  Vec2i size() const;

  expected<void, WindowError> setWindowIcon(ImageRef icon) const;

  expected<void, WindowError> setCursorIcon(ImageRef icon) const;

private:
  explicit Window(NativeWindowHandle native_handle);
};

}  // namespace sde::graphics
