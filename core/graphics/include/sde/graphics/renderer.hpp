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
#include "sde/geometry_types.hpp"
#include "sde/graphics/assets_fwd.hpp"
#include "sde/graphics/render_buffer_fwd.hpp"
#include "sde/graphics/render_target_handle.hpp"
#include "sde/graphics/shader_handle.hpp"
#include "sde/graphics/shapes_fwd.hpp"
#include "sde/graphics/texture_units.hpp"
#include "sde/graphics/typedef.hpp"
#include "sde/time.hpp"
#include "sde/view.hpp"

namespace sde::graphics
{
/**
 * @brief Resources used during a render pass
 */
struct RenderResources
{
  RenderTargetHandle target = RenderTargetHandle::null();
  ShaderHandle shader = ShaderHandle::null();
  std::size_t buffer_group = 0UL;
  bool isValid() const { return shader.isValid(); }
};

std::ostream& operator<<(std::ostream& os, const RenderResources& resources);


/**
 * @brief Standard values passed to shaders on render pass
 */
struct RenderUniforms
{
  Mat3f world_from_camera = Mat3f::Identity();
  float scaling = 1.0F;
  TimeOffset time = TimeOffset::zero();
  TimeOffset time_delta = TimeOffset::zero();
  Mat3f getWorldFromViewportMatrix(const Vec2i& viewport_size) const;
};

std::ostream& operator<<(std::ostream& os, const RenderUniforms& uniforms);


/**
 * @brief Buffer mode
 */
enum class VertexBufferMode
{
  kStatic,
  kDynamic
};

/**
 * @brief Texture creation options
 */
struct VertexBufferOptions
{
  std::size_t max_triangle_count_per_render_pass = 10000UL;
  VertexBufferMode mode = VertexBufferMode::kDynamic;
};


/**
 * @brief Texture creation options
 */
struct Renderer2DOptions
{
  static constexpr std::size_t kVetexArrayCount = 4;
  std::array<VertexBufferOptions, kVetexArrayCount> buffers;
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

  void flush(const Assets& assets, const RenderUniforms& uniforms, const Mat3f& viewport_from_world);

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
  kInvalidRenderTarget,
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
    RenderBuffer& buffer,
    Renderer2D& renderer,
    const Assets& assets,
    const RenderUniforms& uniforms,
    const RenderResources& resources,
    Vec2i viewport_size = Vec2i::Zero());

  const Mat3f& getWorldFromViewportMatrix() const { return world_from_viewport_; };
  const Mat3f& getViewportFromWorldMatrix() const { return viewport_from_world_; };
  const Bounds2f& getViewportInWorldBounds() const { return viewport_in_world_bounds_; };

  constexpr RenderBuffer* operator->() { return buffer_; }

  void clear(const Vec4f& color = Vec4f::Zero());
  bool visible(const Bounds2f& query_aabb) const { return getViewportInWorldBounds().intersects(query_aabb); }

private:
  RenderPass() = default;
  RenderPass(const RenderPass&) = delete;

  static bool retarget(Vec2i& viewport_size, RenderTargetHandle render_target, const Assets& assets);

  Renderer2D* renderer_;
  RenderBuffer* buffer_;
  const Assets* assets_;
  const RenderUniforms* uniforms_;
  Mat3f world_from_viewport_;
  Mat3f viewport_from_world_;
  Bounds2f viewport_in_world_bounds_;
};

std::ostream& operator<<(std::ostream& os, const RenderPass& layer);


}  // namespace sde::graphics
