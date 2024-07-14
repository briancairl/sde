// C++ Standard Library
#include <array>
#include <chrono>
#include <cstdio>
#include <ostream>
#include <thread>

// GLWF
#include <GLFW/glfw3.h>

// SDE
#include "sde/app.hpp"
#include "sde/graphics/window.hpp"
#include "sde/logging.hpp"

namespace sde
{
namespace
{

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

void glfwScanKeyStates(GLFWwindow* glfw_window, KeyStates& curr)
{
  auto prev_down = curr.down;
  for (const auto& [keycode, index] : kKeyScanPattern)
  {
    switch (glfwGetKey(glfw_window, keycode))
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

void glfwScrollEventHandler(GLFWwindow* glfw_window, double xoffset, double yoffset)
{
  auto* app_properties = reinterpret_cast<AppProperties*>(glfwGetWindowUserPointer(glfw_window));
  app_properties->mouse_scroll.x() = xoffset;
  app_properties->mouse_scroll.y() = yoffset;
}

}  // namespace

expected<App, AppError> App::create(Window&& window)
{
  if (window.isNull())
  {
    SDE_LOG_DEBUG("WindowInvalid");
    return make_unexpected(AppError::kWindowInvalid);
  }
  return App{std::move(window)};
}

expected<App, AppError> App::create(const WindowOptions& options)
{
  auto window_or_error = Window::create(options);
  if (!window_or_error.has_value())
  {
    SDE_LOG_DEBUG("WindowCreationFailure");
    return make_unexpected(AppError::kWindowCreationFailure);
  }
  return App{std::move(window_or_error).value()};
}

App::App(Window&& window) : window_{std::move(window)} {}

void App::spin(OnUpdate on_update, const Rate spin_rate)
{
  AppState app_state;
  AppProperties app_properties;
  app_properties.window = window_.value();

  auto* glfw_window = reinterpret_cast<GLFWwindow*>(window_.value());

  auto t_start = Clock::now();
  auto t_prev = t_start;
  auto t_next = t_start + spin_rate.period();

  glfwSetWindowUserPointer(glfw_window, reinterpret_cast<void*>(&app_properties));
  glfwSetScrollCallback(glfw_window, glfwScrollEventHandler);

  while (!glfwWindowShouldClose(glfw_window))
  {
    glfwGetFramebufferSize(
      glfw_window, (app_properties.viewport_size.data() + 0), (app_properties.viewport_size.data() + 1));

    glfwGetCursorPos(
      glfw_window, (app_properties.mouse_position_px.data() + 0), (app_properties.mouse_position_px.data() + 1));

    glfwPollEvents();

    glfwScanKeyStates(glfw_window, app_properties.keys);

    switch (on_update(app_state, app_properties))
    {
    case AppDirective::kContinue:
      break;
    case AppDirective::kReset:
      t_start = Clock::now();
      t_prev = t_start;
      break;
    case AppDirective::kClose:
      return;
    }

    glfwSwapBuffers(glfw_window);

    const auto t_now = Clock::now();
    if (t_now > t_next)
    {
      SDE_LOG_WARN_FMT("loop rate %e Hz not met (behind by %e s)", toHertz(spin_rate), toSeconds(t_now - t_next));
      t_next = t_now + spin_rate.period();
    }
    else
    {
      std::this_thread::sleep_until(t_next);
      t_next += spin_rate.period();
    }

    app_properties.mouse_scroll.setZero();
    app_properties.time = (t_now - t_start);
    app_properties.time_delta = (t_now - t_prev);
    t_prev = t_now;
  }
  glfwSetWindowUserPointer(glfw_window, nullptr);
}

}  // namespace sde
