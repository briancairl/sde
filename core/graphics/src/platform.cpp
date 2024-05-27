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
  SDE_LOG_INFO("Initializing GLFW...");
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

  SDE_ASSERT_NE_MSG(window, nullptr, "Failed to create GLFW window");
  SDE_LOG_INFO("Created GLFW window");

  glfwMakeContextCurrent(window);

  SDE_ASSERT(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), "Failed to load OpenGL (via glad)");
  SDE_LOG_INFO("Loaded OpenGL (via glad)");

  enable_native_debug_logs();
  enable_native_error_logs();

  static constexpr int kBufferSwapInterval_EnableVSync = 1;
  glfwSwapInterval(kBufferSwapInterval_EnableVSync);
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  return WindowHandle{reinterpret_cast<void*>(window)};
}

constexpr std::array<std::pair<int, std::size_t>, static_cast<size_t>(KeyCode::_Count_)> kKeyScanPattern{{
  {GLFW_KEY_1, static_cast<std::size_t>(KeyCode::kNum1)},
  {GLFW_KEY_2, static_cast<std::size_t>(KeyCode::kNum2)},
  {GLFW_KEY_3, static_cast<std::size_t>(KeyCode::kNum3)},
  {GLFW_KEY_4, static_cast<std::size_t>(KeyCode::kNum4)},
  {GLFW_KEY_5, static_cast<std::size_t>(KeyCode::kNum5)},
  {GLFW_KEY_6, static_cast<std::size_t>(KeyCode::kNum6)},
  {GLFW_KEY_7, static_cast<std::size_t>(KeyCode::kNum7)},
  {GLFW_KEY_8, static_cast<std::size_t>(KeyCode::kNum8)},
  {GLFW_KEY_9, static_cast<std::size_t>(KeyCode::kNum9)},
  {GLFW_KEY_0, static_cast<std::size_t>(KeyCode::kNum0)},
  {GLFW_KEY_Q, static_cast<std::size_t>(KeyCode::kQ)},
  {GLFW_KEY_W, static_cast<std::size_t>(KeyCode::kW)},
  {GLFW_KEY_E, static_cast<std::size_t>(KeyCode::kE)},
  {GLFW_KEY_A, static_cast<std::size_t>(KeyCode::kA)},
  {GLFW_KEY_S, static_cast<std::size_t>(KeyCode::kS)},
  {GLFW_KEY_D, static_cast<std::size_t>(KeyCode::kD)},
  {GLFW_KEY_Z, static_cast<std::size_t>(KeyCode::kZ)},
  {GLFW_KEY_X, static_cast<std::size_t>(KeyCode::kX)},
  {GLFW_KEY_C, static_cast<std::size_t>(KeyCode::kC)},
  {GLFW_KEY_SPACE, static_cast<std::size_t>(KeyCode::kSpace)},
  {GLFW_KEY_LEFT_SHIFT, static_cast<std::size_t>(KeyCode::kLShift)},
  {GLFW_KEY_RIGHT_SHIFT, static_cast<std::size_t>(KeyCode::kRShift)},
  {GLFW_KEY_LEFT_CONTROL, static_cast<std::size_t>(KeyCode::kLCtrl)},
  {GLFW_KEY_RIGHT_CONTROL, static_cast<std::size_t>(KeyCode::kRCtrl)},
  {GLFW_KEY_LEFT_ALT, static_cast<std::size_t>(KeyCode::kLAlt)},
  {GLFW_KEY_RIGHT_ALT, static_cast<std::size_t>(KeyCode::kRAlt)},
}};

void glfwScanKeyStates(GLFWwindow* window, WindowKeyStates& curr)
{
  auto prev_down = curr.down;
  for (const auto& [keycode, index] : kKeyScanPattern)
  {
    switch (glfwGetKey(window, keycode))
    {
    case GLFW_PRESS:
      curr.down.set(index, true);
      break;
    case GLFW_RELEASE:
      curr.down.set(index, false);
      break;
    }
  }
  curr.pressed = (curr.down & (curr.down ^ prev_down));
  curr.released = (prev_down & ~curr.down);
}

void glfwScrollEventHandler(GLFWwindow* window, double xoffset, double yoffset)
{
  auto* window_properties = reinterpret_cast<WindowProperties*>(glfwGetWindowUserPointer(window));
  window_properties->mouse_scroll.x() = xoffset;
  window_properties->mouse_scroll.y() = yoffset;
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

void WindowHandle::spin(std::function<WindowDirective(const WindowProperties&)> on_update)
{
  static constexpr double kLoopRate = 60.0;

  WindowProperties window_properties;

  auto* window = reinterpret_cast<GLFWwindow*>(p_);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  const auto t_advance =
    std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>(1.0 / kLoopRate));

  auto t_start = std::chrono::steady_clock::now();
  auto t_prev = t_start;
  auto t_next = t_start + t_advance;

  glfwSetWindowUserPointer(window, reinterpret_cast<void*>(&window_properties));
  glfwSetScrollCallback(window, glfwScrollEventHandler);

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

    glfwScanKeyStates(window, window_properties.keys);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    switch (on_update(window_properties))
    {
    case WindowDirective::kContinue:
      break;
    case WindowDirective::kReset:
      t_start = std::chrono::steady_clock::now();
      t_prev = t_start;
      break;
    case WindowDirective::kClose:
      return;
    }

    glViewport(0, 0, window_properties.size.x(), window_properties.size.y());
    glfwSwapBuffers(window);

    const auto t_now = std::chrono::steady_clock::now();
    if (t_now > t_next)
    {
      SDE_LOG_WARN_FMT("loop rate %e Hz not met", kLoopRate);
      t_next = t_now + t_advance;
    }
    else
    {
      std::this_thread::sleep_until(t_next);
      t_next += t_advance;
    }

    window_properties.mouse_scroll.setZero();
    window_properties.time = (t_now - t_start);
    window_properties.time_delta = (t_now - t_prev);
    t_prev = t_now;
  }
  glfwSetWindowUserPointer(window, nullptr);
}

}  // namespace sde::graphics
