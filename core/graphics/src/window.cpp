// C++ Standard Library
#include <array>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <thread>

// GLAD
#include <glad/glad.h>

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

// SDE
#include "sde/graphics/debug.hpp"
#include "sde/graphics/image.hpp"
#include "sde/graphics/window.hpp"
#include "sde/logging.hpp"

namespace sde::graphics
{
namespace
{
std::atomic_bool glfw_is_initialized = {false};

#if defined(SDE_GLFW_DEBUG) && SDE_GLFW_DEBUG
void glfwErrorCallback(int error, const char* description)
{
  std::fprintf(stderr, "[GLFW] %d : %s\n", error, description);
}
#endif  // SDE_GLFW_DEBUG

bool glfwTryFirstInit()
{
  if (glfw_is_initialized.exchange(true))
  {
    SDE_LOG_DEBUG() << "GLFW previously initialized";
    return false;
  }

#if defined(SDE_GLFW_DEBUG) && SDE_GLFW_DEBUG
  glfwSetErrorCallback(glfwErrorCallback);
#endif  // SDE_GLFW_DEBUG

  SDE_LOG_INFO() << "Initializing GLFW...";
  SDE_ASSERT(glfwInit()) << "Failed to initialize GLFW";
  SDE_LOG_INFO() << "Initialized GLFW";

  // Decide GL+GLSL versions
#if __APPLE__
  // GL 3.2 + GLSL 150
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);  // Required on Mac
#else
  // GL 3.0 + GLSL 130
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
  // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif
  SDE_LOG_DEBUG() << "Set window hints";
  return true;
}

}  // namespace

void WindowDeleter::operator()(NativeWindowHandle native_handle) const
{
  SDE_LOG_DEBUG() << "glfwDestroyWindow";
  glfwDestroyWindow(reinterpret_cast<GLFWwindow*>(native_handle));
}

Window::Window(NativeWindowHandle native_handle) : UniqueResource<NativeWindowHandle, WindowDeleter>{native_handle} {}

void Window::activate() const { glfwMakeContextCurrent(reinterpret_cast<GLFWwindow*>(value())); }

bool Window::backend_initialized() { return glfw_is_initialized.load(); }

bool Window::try_backend_initialization() { return gladLoadGLLoader((GLADloadproc)glfwGetProcAddress); }

expected<Window, WindowError> Window::create(const WindowOptions& options)
{
  const bool glfw_initialized_on_this_call = glfwTryFirstInit();

  // Create window with graphics context
  GLFWwindow* glfw_window =
    glfwCreateWindow(options.initial_size.x(), options.initial_size.y(), options.title, NULL, NULL);

  if (glfw_window == nullptr)
  {
    return make_unexpected(WindowError::kWindowCreationFailed);
  }

  // Wrap GLFW window in resource wrapper for auto-cleanup on failure
  Window window{reinterpret_cast<NativeWindowHandle>(glfw_window)};

  SDE_LOG_INFO() << "Created GLFW window";
  glfwMakeContextCurrent(glfw_window);

  if (glfw_initialized_on_this_call)
  {
    SDE_ASSERT(Window::try_backend_initialization()) << "Failed to load OpenGL (via glad)";
    SDE_LOG_INFO() << "Loaded OpenGL (via glad)";
  }
  else
  {
    SDE_LOG_DEBUG() << "Previously loaded OpenGL (via glad)";
  }

  window.activate();

#if defined(SDE_GLFW_DEBUG) && SDE_GLFW_DEBUG
  enable_native_debug_logs();
  enable_native_error_logs();
#endif  // SDE_GLFW_DEBUG

  static constexpr int kBufferSwapInterval_EnableVSync = 1;
  glfwSwapInterval(kBufferSwapInterval_EnableVSync);
  glfwSetInputMode(glfw_window, GLFW_STICKY_KEYS, GLFW_TRUE);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  return window;
}

expected<void, WindowError> Window::setIcon(ImageRef icon) const
{
  auto* glfw_window = reinterpret_cast<GLFWwindow*>(this->value());

  // Handle window icon setting
  if (!icon.isValid())
  {
    SDE_LOG_DEBUG() << "No icon set to window";
    glfwSetWindowIcon(glfw_window, 0, nullptr);
    return {};
  }
  else if (icon.channels != ImageChannels::kRGBA)
  {
    SDE_LOG_DEBUG() << "WindowIconInvalidPixelFormat";
    return make_unexpected(WindowError::kWindowIconInvalidPixelFormat);
  }
  else if (icon.pixels() == 0)
  {
    SDE_LOG_DEBUG() << "WindowIconInvalidSize";
    return make_unexpected(WindowError::kWindowIconInvalidSize);
  }

  const GLFWimage glfw_image{
    .width = icon.width, .height = icon.height, .pixels = reinterpret_cast<std::uint8_t*>(icon.data)};

  glfwSetWindowIcon(glfw_window, 1, &glfw_image);

  return {};
}

expected<void, WindowError> Window::setCursor(ImageRef cursor) const
{
  auto* glfw_window = reinterpret_cast<GLFWwindow*>(this->value());

  // Handle window cursor setting
  if (!cursor.isValid())
  {
    SDE_LOG_DEBUG() << "No cursor set to window";
    glfwSetCursor(glfw_window, glfwCreateStandardCursor(GLFW_ARROW_CURSOR));
    return {};
  }
  else if (cursor.channels != ImageChannels::kRGBA)
  {
    SDE_LOG_DEBUG() << "WindowCursorInvalidPixelFormat";
    return make_unexpected(WindowError::kWindowCursorInvalidPixelFormat);
  }
  else if (cursor.pixels() == 0)
  {
    SDE_LOG_DEBUG() << "WindowCursorInvalidSize";
    return make_unexpected(WindowError::kWindowCursorInvalidSize);
  }

  const GLFWimage glfw_image{
    .width = cursor.width, .height = cursor.height, .pixels = reinterpret_cast<std::uint8_t*>(cursor.data)};

  glfwSetCursor(glfw_window, glfwCreateCursor(&glfw_image, 0, 0));

  return {};
}


}  // namespace sde::graphics
