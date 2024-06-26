/**
 * @copyright 2024-present Brian Cairl
 *
 * @file window.hpp
 */
#pragma once

// SDE
#include "sde/expected.hpp"
#include "sde/geometry_types.hpp"
#include "sde/graphics/image_fwd.hpp"
#include "sde/graphics/window_fwd.hpp"
#include "sde/resource_wrapper.hpp"

namespace sde::graphics
{

struct WindowOptions
{
  const char* title = "sde";
  Vec2i initial_size = {640, 480};
  Image* icon = nullptr;
  Image* cursor = nullptr;
};

enum class WindowError
{
  kWindowCreationFailed,
  kWindowIconInvalidPixelFormat,
  kWindowIconInvalidSize,
  kWindowCursorInvalidPixelFormat,
  kWindowCursorInvalidSize,
};

struct WindowDeleter
{
  void operator()(WindowNativeHandle id) const;
};

class Window : public UniqueResource<WindowNativeHandle, WindowDeleter>
{
public:
  static expected<Window, WindowError> create(const WindowOptions& options);

  void activate() const;

private:
  explicit Window(WindowNativeHandle native_handle);
};

}  // namespace sde::graphics
