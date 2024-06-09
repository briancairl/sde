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
std::atomic_flag STATIC__glfw_is_initialized = {false};

#if defined(SDE_GLFW_DEBUG) && SDE_GLFW_DEBUG
void glfwErrorCallback(int error, const char* description)
{
  std::fprintf(stderr, "[GLFW] %d : %s\n", error, description);
}
#endif  // SDE_GLFW_DEBUG

bool glfwTryFirstInit()
{
  if (STATIC__glfw_is_initialized.test_and_set())
  {
    SDE_LOG_DEBUG("GLFW previously initialized");
    return false;
  }

#if defined(SDE_GLFW_DEBUG) && SDE_GLFW_DEBUG
  glfwSetErrorCallback(glfwErrorCallback);
#endif  // SDE_GLFW_DEBUG

  SDE_LOG_INFO("Initializing GLFW...");
  SDE_ASSERT(glfwInit(), "Failed to initialize GLFW ");
  SDE_LOG_INFO("Initialized GLFW");

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
  SDE_LOG_DEBUG("Set window hints");
  return true;
}

}  // namespace

void WindowDeleter::operator()(WindowNativeHandle native_handle) const
{
  if (native_handle == nullptr)
  {
    return;
  }
  SDE_LOG_DEBUG("glfwDestroyWindow");
  glfwDestroyWindow(reinterpret_cast<GLFWwindow*>(native_handle));
}

Window::Window(WindowNativeHandle native_handle) : UniqueResource<WindowNativeHandle, WindowDeleter>{native_handle} {}

void Window::activate() const { glfwMakeContextCurrent(reinterpret_cast<GLFWwindow*>(value())); }

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
  Window window{reinterpret_cast<WindowNativeHandle>(glfw_window)};

  // Handle window icon setting
  if (options.icon == nullptr)
  {
    SDE_LOG_DEBUG("No icon set to window");
  }
  else if (options.icon->channels() != ImageChannels::kRGBA)
  {
    return make_unexpected(WindowError::kWindowIconInvalidPixelFormat);
  }
  else if (options.icon->shape().pixels() == 0)
  {
    return make_unexpected(WindowError::kWindowIconInvalidSize);
  }
  else
  {
    const GLFWimage icon_image{
      .width = options.icon->shape().width(),
      .height = options.icon->shape().height(),
      .pixels = const_cast<std::uint8_t*>(options.icon->data().data())};
    glfwSetWindowIcon(glfw_window, 1, &icon_image);
  }

  // Handle window cursor setting
  if (options.cursor == nullptr)
  {
    SDE_LOG_DEBUG("No cursor set to window");
  }
  else if (options.cursor->channels() != ImageChannels::kRGBA)
  {
    return make_unexpected(WindowError::kWindowCursorInvalidPixelFormat);
  }
  else if (options.cursor->shape().pixels() == 0)
  {
    return make_unexpected(WindowError::kWindowCursorInvalidSize);
  }
  else
  {
    const GLFWimage cursor_image{
      .width = options.cursor->shape().width(),
      .height = options.cursor->shape().height(),
      .pixels = const_cast<std::uint8_t*>(options.cursor->data().data())};
    glfwSetCursor(glfw_window, glfwCreateCursor(&cursor_image, 0, 0));
  }

  SDE_LOG_INFO("Created GLFW window");
  glfwMakeContextCurrent(glfw_window);

  if (glfw_initialized_on_this_call)
  {
    SDE_ASSERT(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), "Failed to load OpenGL (via glad)");
    SDE_LOG_INFO("Loaded OpenGL (via glad)");
  }
  else
  {
    SDE_LOG_DEBUG("Previously loaded OpenGL (via glad)");
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

}  // namespace sde::graphics
