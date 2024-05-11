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
  SDE_LOG_INFO("Set window hints");

  // Create window with graphics context
  GLFWwindow* window = glfwCreateWindow(options.initial_width, options.initial_height, options.title, NULL, NULL);

  SDE_ASSERT((window != nullptr), "Failed to create GLFW window");
  SDE_LOG_INFO("Created GLFW window");

  glfwMakeContextCurrent(window);

  SDE_ASSERT(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), "Failed to load OpenGL (via glad)");
  SDE_LOG_INFO("Loaded OpenGL (via glad)");

  return WindowHandle{reinterpret_cast<void*>(window)};
}
}  // namespace

WindowHandle initialize(const WindowOptions& options) { return glfw_try_init(options); }

WindowHandle::~WindowHandle() {}

}  // namespace sde::graphics
