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
#include "sde/graphics/shader_fwd.hpp"
#include "sde/graphics/shader_handle.hpp"
#include "sde/graphics/texture_fwd.hpp"
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
  kNonEnoughMemory,
};

std::ostream& operator<<(std::ostream& os, Renderer2DError error);

struct Quad
{
  static constexpr std::size_t kVertexCount = 4UL;
  Vec2f min = Vec2f::Zero();
  Vec2f max = Vec2f::Zero();
  Vec4f color = Vec4f::Ones();
};

struct TexturedQuad
{
  Quad quad;
  Quad texquad;
  TextureHandle texture;
};

class Renderer2D
{
public:
  static constexpr std::size_t kDefaultLayer = 0UL;

  ~Renderer2D();

  Renderer2D(Renderer2D&& other) = default;

  /**
   * @brief Set a shader to use for all layers
   */
  void set(ShaderHandle shader);

  /**
   * @brief Set a shader to use for a specific layer
   */
  void set(std::size_t layer, ShaderHandle shader);

  /**
   * @brief Add a quad to a specific layer
   */
  void submit(std::size_t layer, const Quad& quad);

  /**
   * @brief Add a textured to a specific layer
   */
  void submit(std::size_t layer, const TexturedQuad& quad);

  /**
   * @brief Add quad to the default layer
   */
  void submit(const Quad& quad) { this->submit(kDefaultLayer, quad); }

  /**
   * @brief Add textured quad to the default layer
   */
  void submit(const TexturedQuad& quad) { this->submit(kDefaultLayer, quad); }

  /**
   * @brief Draws buffered shapes
   */
  void update(const ShaderCache& shader_cache, const TextureCache& texture_cache);

  static expected<Renderer2D, Renderer2DError> create(const Renderer2DOptions& options = {});

private:
  struct Layer
  {
    bool y_sorted = false;
    ShaderHandle shader = ShaderHandle::null();
    std::vector<Quad> quads;
    std::vector<TexturedQuad> textured_quads;

    void reset();
    bool drawable() const;
  };

  std::vector<Layer> layers_;

  class Backend;
  std::unique_ptr<Backend> backend_;

  explicit Renderer2D(std::unique_ptr<Backend> backend, std::vector<Layer> layers);
};

std::ostream& operator<<(std::ostream& os, const Renderer2D& renderer);

}  // namespace sde::graphics
