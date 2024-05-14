/**
 * @copyright 2024-present Brian Cairl
 *
 * @file renderer.hpp
 */
#pragma once

// C++ Standard Library
#include <cstdint>
#include <memory>
#include <vector>

// SDE
#include "sde/expected.hpp"
#include "sde/geometry_types.hpp"
#include "sde/graphics/texture_handle.hpp"

namespace sde::graphics
{

/**
 * @brief Texture creation options
 */
struct Renderer2DOptions
{
  std::size_t max_layers = 3UL;
  std::size_t max_triangle_count_per_layer = 10000UL;
};

enum class Renderer2DError
{
};

std::ostream& operator<<(std::ostream& os, Renderer2DError error);

struct Quad
{
  Vec2f min;
  Vec2f max;
  Vec4f color;
};

struct TexturedQuad
{
  Quad quad;
  TextureHandle texture;
};

class Renderer2D
{
public:
  static constexpr std::size_t kDefaultLayer = 0UL;

  ~Renderer2D();

  Renderer2D(Renderer2D&& other) = default;

  void submit(std::size_t layer, const Quad& quad);

  void submit(std::size_t layer, const TexturedQuad& quad);

  void submit(const Quad& quad) { this->submit(kDefaultLayer, quad); }

  void submit(const TexturedQuad& quad) { this->submit(kDefaultLayer, quad); }

  void update();

  static expected<Renderer2D, Renderer2DError> create(const Renderer2DOptions& options = {});

private:
  struct Layer
  {
    bool y_sorted = false;
    std::vector<Quad> quads;
    std::vector<TexturedQuad> textured_quads;
    void reset();
  };

  std::vector<Layer> layers_;

  class Backend;
  std::unique_ptr<Backend> backend_;

  explicit Renderer2D(std::unique_ptr<Backend> backend, std::vector<Layer> layers);
};

std::ostream& operator<<(std::ostream& os, const Renderer2D& renderer);

}  // namespace sde::graphics
