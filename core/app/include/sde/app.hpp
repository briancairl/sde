/**
 * @copyright 2024-present Brian Cairl
 *
 * @file app.hpp
 */
#pragma once

// C++ Standard Library
#include <chrono>
#include <functional>

// SDE
#include "sde/app_properties.hpp"
#include "sde/expected.hpp"
#include "sde/geometry.hpp"
#include "sde/graphics/window.hpp"
#include "sde/keyboard.hpp"
#include "sde/time.hpp"

namespace sde
{

enum class AppDirective
{
  kContinue,
  kReset,
  kClose
};

enum class AppError
{
  kWindowInvalid,
  kWindowCreationFailure,
};

class App
{
public:
  using Window = graphics::Window;
  using WindowOptions = graphics::WindowOptions;
  using OnUpdate = std::function<AppDirective(AppState&, const AppProperties&)>;

  App(App&&) = default;

  void spin(OnUpdate on_update, const Rate spin_rate = Hertz(60.0F));

  const Window& window() const { return window_; }

  static expected<App, AppError> create(Window&& window);

  static expected<App, AppError> create(const WindowOptions& window_options);

private:
  explicit App(Window&& window);
  App(const App&) = delete;
  Window window_;
};

}  // namespace sde
