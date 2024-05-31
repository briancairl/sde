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

// SDE
#include "sde/expected.hpp"
#include "sde/graphics/render_target.hpp"
#include "sde/graphics/shader_fwd.hpp"
#include "sde/graphics/shader_handle.hpp"
#include "sde/graphics/shapes.hpp"
#include "sde/graphics/texture_fwd.hpp"
#include "sde/graphics/texture_handle.hpp"
#include "sde/graphics/texture_units.hpp"
#include "sde/graphics/typedef.hpp"
#include "sde/view.hpp"

namespace sde::graphics
{
class TileSet;
struct TileMap;

/**
 * @brief Resources used during a render pass
 */
struct RenderResources
{
  ShaderHandle shader = ShaderHandle::null();
  TextureUnits textures = {};
  bool isValid() const { return shader.isValid(); }
};

std::ostream& operator<<(std::ostream& os, const RenderResources& resources);


/**
 * @brief Standard values passed to shaders on render pass
 */
struct RenderAttributes
{
  Mat3f world_from_camera = Mat3f::Identity();
  float scaling = 1.0F;
  float time = 0.0F;
  float time_delta = 0.0F;
  Mat3f getWorldFromViewportMatrix(const RenderTarget& target) const;
};

std::ostream& operator<<(std::ostream& os, const RenderAttributes& attributes);


struct RenderPass;


/**
 * @brief Texture creation options
 */
struct Renderer2DOptions
{
  std::size_t max_triangle_count_per_render_pass = 10000UL;
};

struct RenderBackend
{};

enum class RendererError
{
  kRendererPreviouslyInitialized,
};

/**
 * @brief High-level interface into the rendering backend (for 2D objects)
 */
class Renderer2D
{
public:
  ~Renderer2D();

  Renderer2D(Renderer2D&&);

  static expected<Renderer2D, RendererError>
  create(const ShaderCache* shader_cache, const TextureCache* texture_cache, const Renderer2DOptions& options = {});

  void rebind(const ShaderCache* shader_cache) { shader_cache_ = shader_cache; }
  void rebind(const TextureCache* texture_cache) { texture_cache_ = texture_cache; }

  void flush();

  Mat3f refresh(RenderTarget& target, const RenderAttributes& attributes, const RenderResources& resources);

private:
  Renderer2D() = default;
  Renderer2D(const Renderer2D&) = delete;

  const ShaderCache* shader_cache_ = nullptr;
  const TextureCache* texture_cache_ = nullptr;
  RenderResources active_resources_;
  RenderBackend* backend_ = nullptr;
};


enum class RenderPassError
{
  kRenderPassActive,
  kMaxVertexCountExceeded,
  kMaxElementCountExceeded,
};

/**
 * @brief Encapsulates a single render pass
 */
struct RenderPass
{
public:
  ~RenderPass();

  RenderPass(RenderPass&& other);

  expected<void, RenderPassError> submit(View<const Quad> quads);
  expected<void, RenderPassError> submit(View<const Circle> circles);
  expected<void, RenderPassError> submit(View<const TexturedQuad> quads);
  expected<void, RenderPassError> submit(View<const TileMap> tile_maps, const TileSet& tile_set);

  static expected<RenderPass, RenderPassError> create(
    RenderTarget& target,
    Renderer2D& renderer,
    const RenderAttributes& attributes,
    const RenderResources& resources);

  const Mat3f& getWorldFromViewportMatrix() const { return world_from_viewport_; };
  const Bounds2f& getViewportInWorldBounds() const { return viewport_in_world_bounds_; };

private:
  RenderPass() = default;
  RenderPass(const RenderPass&) = delete;

  Renderer2D* renderer_;
  Mat3f world_from_viewport_;
  Bounds2f viewport_in_world_bounds_;
};

std::ostream& operator<<(std::ostream& os, const RenderPass& layer);


}  // namespace sde::graphics
