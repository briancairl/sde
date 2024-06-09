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
#include <string_view>

// SDE

#include "sde/expected.hpp"
#include "sde/graphics/assets_fwd.hpp"
#include "sde/graphics/render_target.hpp"
#include "sde/graphics/shader_handle.hpp"
#include "sde/graphics/shapes_fwd.hpp"
#include "sde/graphics/texture_units.hpp"
#include "sde/graphics/typedef.hpp"
#include "sde/view.hpp"

namespace sde::graphics
{
/**
 * @brief Resources used during a render pass
 */
struct RenderResources
{
  ShaderHandle shader = ShaderHandle::null();
  std::size_t buffer_group = 0UL;
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


/**
 * @brief Buffer mode
 */
enum class RenderBufferMode
{
  kStatic,
  kDynamic
};

/**
 * @brief Texture creation options
 */
struct RenderBufferOptions
{
  std::size_t max_triangle_count_per_render_pass = 10000UL;
  RenderBufferMode mode = RenderBufferMode::kDynamic;
};


/**
 * @brief Texture creation options
 */
struct Renderer2DOptions
{
  static constexpr std::size_t kVetexArrayCount = 4;
  std::array<RenderBufferOptions, kVetexArrayCount> buffers;
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

  static expected<Renderer2D, RendererError> create(const Renderer2DOptions& options = {});

  void flush(const Assets& assets, const RenderAttributes& attributes, const Mat3f& viewport_from_world);

  void refresh(const RenderResources& resources);

  void assign(std::size_t unit, const TextureHandle& texture) { next_active_textures_[unit] = texture; }

  std::optional<std::size_t> assign(const TextureHandle& texture);

private:
  Renderer2D() = default;
  Renderer2D(const Renderer2D&) = delete;

  RenderResources last_active_resources_;
  RenderResources next_active_resources_;
  TextureUnits last_active_textures_;
  TextureUnits next_active_textures_;
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
class RenderPass
{
public:
  ~RenderPass();

  RenderPass(RenderPass&& other);

  expected<void, RenderPassError> submit(View<const Quad> quads);
  expected<void, RenderPassError> submit(View<const Circle> circles);
  expected<void, RenderPassError> submit(View<const TexturedQuad> quads);

  const Assets& assets() const { return *assets_; };
  std::optional<std::size_t> assign(const TextureHandle& texture) { return renderer_->assign(texture); }

  static expected<RenderPass, RenderPassError> create(
    RenderTarget& target,
    Renderer2D& renderer,
    const Assets& assets,
    const RenderAttributes& attributes,
    const RenderResources& resources);

  const Mat3f& getWorldFromViewportMatrix() const { return world_from_viewport_; };
  const Mat3f& getViewportFromWorldMatrix() const { return viewport_from_world_; };
  const Bounds2f& getViewportInWorldBounds() const { return viewport_in_world_bounds_; };

private:
  RenderPass() = default;
  RenderPass(const RenderPass&) = delete;

  Renderer2D* renderer_;
  const Assets* assets_;
  const RenderAttributes* attributes_;
  Mat3f world_from_viewport_;
  Mat3f viewport_from_world_;
  Bounds2f viewport_in_world_bounds_;
};

std::ostream& operator<<(std::ostream& os, const RenderPass& layer);


}  // namespace sde::graphics
