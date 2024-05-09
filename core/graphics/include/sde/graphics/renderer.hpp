/**
 * @copyright 2024-present Brian Cairl
 *
 * @file renderer.hpp
 */
#pragma once

// C++ Standard Library
#include <cstdint>
#include <memory>

// SDE
#include "sde/expected.hpp"

namespace sde::graphics
{

/**
 * @brief Texture creation options
 */
struct Renderer2DOptions
{
  std::size_t max_quad_count = 10000;
};

enum class Renderer2DError
{
};

std::ostream& operator<<(std::ostream& os, Renderer2DError error);

class Renderer2D
{
public:
  ~Renderer2D();

  Renderer2D(Renderer2D&& other) = default;

  static expected<Renderer2D, Renderer2DError> create(const Renderer2DOptions& options);

private:
  class Backend;
  std::unique_ptr<Backend> backend_;

  explicit Renderer2D(std::unique_ptr<Backend> backend);
};

std::ostream& operator<<(std::ostream& os, const Renderer2D& renderer);

}  // namespace sde::graphics
