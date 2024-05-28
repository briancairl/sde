/**
 * @copyright 2024-present Brian Cairl
 *
 * @file renderer.hpp
 */
#pragma once

// C++ Standard Library
#include <array>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <vector>

// SDE
#include "sde/graphics/shader_fwd.hpp"
#include "sde/graphics/shader_handle.hpp"
#include "sde/graphics/shapes.hpp"
#include "sde/graphics/texture_fwd.hpp"
#include "sde/graphics/texture_handle.hpp"
#include "sde/graphics/texture_units.hpp"
#include "sde/graphics/tile_map.hpp"
#include "sde/graphics/typedef.hpp"

namespace sde::graphics
{

struct LayerResources
{
  static constexpr std::size_t kTextureUnits = 16UL;
  ShaderHandle shader = ShaderHandle::null();
  TextureUnits textures;

  bool isValid() const { return shader.isValid(); }
};

std::ostream& operator<<(std::ostream& os, const LayerResources& resources);

struct LayerAttributes
{
  Mat3f world_from_camera = Mat3f::Identity();

  float time = 0.0F;
  float time_delta = 0.0F;

  float scaling = 1.0F;

  Vec2i frame_buffer_dimensions;

  float getViewportAspectRatio() const;
  Mat3f getWorldFromViewportMatrix() const;
};

std::ostream& operator<<(std::ostream& os, const LayerAttributes& attributes);

struct Layer
{
  LayerAttributes attributes;
  LayerResources resources;

  std::vector<Quad> quads;
  std::vector<TexturedQuad> textured_quads;
  std::vector<Circle> circles;
  std::vector<TileMap> tile_maps;

  /**
   *
   */
  bool is_static = false;

  void reset();

  bool isValid() const;
};

std::ostream& operator<<(std::ostream& os, const Layer& layer);

/**
 * @brief Texture creation options
 */
struct Renderer2DOptions
{
  std::size_t max_triangle_count_per_layer = 10000UL;
};

class Renderer2D
{
public:
  static constexpr std::size_t kDefaultLayer = 0UL;

  ~Renderer2D();

  Renderer2D(Renderer2D&& other) = default;

  explicit Renderer2D(const Renderer2DOptions& options = {});

  /**
   * @brief Draws buffered shapes
   */
  void submit(const ShaderCache& shader_cache, const TextureCache& texture_cache, Layer& layer);

private:
  LayerResources active_resources_;

  class Backend;
  std::unique_ptr<Backend> backend_;
};

}  // namespace sde::graphics
