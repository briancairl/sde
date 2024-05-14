// C++ Standard Library
#include <atomic>
#include <cstdio>

// GLAD
#include <glad/glad.h>

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

// SDE
#include "sde/graphics/platform.hpp"
#include "sde/logging.hpp"

namespace sde::graphics
{
namespace
{
std::atomic_flag glfw_is_initialized = {false};

#if defined(SDE_GLFW_DEBUG) && SDE_GLFW_DEBUG
void glfw_error_callback(int error, const char* description)
{
  std::fprintf(stderr, "[GLFW] %d : %s\n", error, description);
}
#endif  // SDE_GLFW_DEBUG

WindowHandle glfw_try_init(const WindowOptions& options)
{
  SDE_ASSERT(!glfw_is_initialized.test_and_set(), "Graphics already initialized!");

#if defined(SDE_GLFW_DEBUG) && SDE_GLFW_DEBUG
  glfwSetErrorCallback(glfw_error_callback);
  SDE_LOG_INFO("Initialized GLFW error callback");
#endif  // SDE_GLFW_DEBUG

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

  // Create window with graphics context
  GLFWwindow* window = glfwCreateWindow(options.initial_size.x(), options.initial_size.y(), options.title, NULL, NULL);

  SDE_ASSERT((window != nullptr), "Failed to create GLFW window");
  SDE_LOG_INFO("Created GLFW window");

  glfwMakeContextCurrent(window);

  SDE_ASSERT(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), "Failed to load OpenGL (via glad)");
  SDE_LOG_INFO("Loaded OpenGL (via glad)");

  static constexpr int kBufferSwapInterval_EnableVSync = 1;
  glfwSwapInterval(kBufferSwapInterval_EnableVSync);

  return WindowHandle{reinterpret_cast<void*>(window)};
}
}  // namespace

WindowHandle initialize(const WindowOptions& options) { return glfw_try_init(options); }

WindowHandle::WindowHandle(WindowHandle&& other) : p_{other.p_} { other.p_ = nullptr; }

WindowHandle::~WindowHandle()
{
  if (p_ != nullptr)
  {
    glfwDestroyWindow(reinterpret_cast<GLFWwindow*>(p_));
  }
}

void WindowHandle::spin(std::function<void(const WindowProperties&)> on_update)
{
  WindowProperties window_properties;

  auto* window = reinterpret_cast<GLFWwindow*>(p_);
  while (!glfwWindowShouldClose(window))
  {
    glfwGetFramebufferSize(window, (window_properties.size.data() + 0), (window_properties.size.data() + 1));
    glfwGetCursorPos(
      window, (window_properties.mouse_position_px.data() + 0), (window_properties.mouse_position_px.data() + 1));
    window_properties.mouse_position_vp.x() = static_cast<float>(
      2.0 * window_properties.mouse_position_px.x() / static_cast<double>(window_properties.size.x()) - 1.0);
    window_properties.mouse_position_vp.y() = static_cast<float>(
      1.0 - 2.0 * window_properties.mouse_position_px.y() / static_cast<double>(window_properties.size.y()));

    glfwPollEvents();
    on_update(window_properties);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(0, 0, window_properties.size.x(), window_properties.size.y());
    glfwSwapBuffers(window);
  }
}

}  // namespace sde::graphics
