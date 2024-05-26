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
#include "sde/geometry_types.hpp"
#include "sde/graphics/shader_fwd.hpp"
#include "sde/graphics/shader_handle.hpp"
#include "sde/graphics/texture_fwd.hpp"
#include "sde/graphics/texture_handle.hpp"
#include "sde/graphics/typedef.hpp"

namespace sde::graphics
{

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
  Rect rect_texture;
  Vec4f color = Vec4f::Ones();
  std::size_t texture_unit;
};

struct Circle
{
  Vec2f center = {};
  float radius = 1.0F;
  Vec4f color = Vec4f::Ones();
};

struct LayerResources
{
  static constexpr std::size_t kTextureUnits = 16UL;
  ShaderHandle shader = ShaderHandle::null();
  std::array<TextureHandle, kTextureUnits> textures;

  bool is_valid() const { return shader.is_valid(); }

  LayerResources();
};

struct LayerSettings
{
  Mat3f screen_from_world = Mat3f::Identity();

  float time = 0.0F;
  float time_delta = 0.0F;

  float scaling = 1.0F;
  float aspect_ratio = 1.0F;

  void setAspectRatio(Vec2i frame_buffer_dimensions)
  {
    aspect_ratio = static_cast<float>(frame_buffer_dimensions.x()) / static_cast<float>(frame_buffer_dimensions.y());
  }
};

struct Layer
{
  LayerSettings settings;
  LayerResources resources;
  std::vector<Quad> quads;
  std::vector<TexturedQuad> textured_quads;
  std::vector<Circle> circles;

  void reset();
  bool drawable() const;
};

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
