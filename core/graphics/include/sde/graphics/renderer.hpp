/**
 * @copyright 2024-present Brian Cairl
 *
 * @file renderer.hpp
 */
#pragma once

// C++ Standard Library
#include <array>
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
#include "sde/graphics/typedef.hpp"

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

struct Rect
{
  Vec2f min = Vec2f::Zero();
  Vec2f max = Vec2f::Zero();
};

struct Quad
{
  Rect rect = {};
  Vec4f color = Vec4f::Ones();
};

struct TexturedQuad
{
  Rect rect;
  Rect texrect;
  Vec4f color = Vec4f::Ones();
  std::size_t texture_unit;
};

struct LayerSettings
{
  static constexpr std::size_t kTextureUnits = 16UL;
  ShaderHandle shader = ShaderHandle::null();
  std::array<native_texture_id_t, kTextureUnits> textures;
};

class Renderer2D
{
public:
  static constexpr std::size_t kDefaultLayer = 0UL;

  ~Renderer2D();

  Renderer2D(Renderer2D&& other) = default;

  LayerSettings& layer(std::size_t layer);

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
  void update(const ShaderCache& shader_cache);

  static expected<Renderer2D, Renderer2DError> create(const Renderer2DOptions& options = {});

private:
  struct Layer
  {
    static constexpr std::size_t kTextureUnits = 16UL;

    LayerSettings settings;
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
