/**
 * @copyright 2024-present Brian Cairl
 *
 * @file renderer.hpp
 */
#pragma once

// C++ Standard Library
#include <cstdint>
#include <iosfwd>
#include <string_view>

// SDE
#include "sde/expected.hpp"
#include "sde/geometry.hpp"
#include "sde/graphics/render_buffer_fwd.hpp"
#include "sde/graphics/render_target_fwd.hpp"
#include "sde/graphics/render_target_handle.hpp"
#include "sde/graphics/shader_fwd.hpp"
#include "sde/graphics/shader_handle.hpp"
#include "sde/graphics/shapes_fwd.hpp"
#include "sde/graphics/texture_fwd.hpp"
#include "sde/graphics/texture_units.hpp"
#include "sde/graphics/typedef.hpp"
#include "sde/resource.hpp"
#include "sde/resource_dependencies.hpp"
#include "sde/time.hpp"
#include "sde/vector.hpp"
#include "sde/view.hpp"

namespace sde::graphics
{
/**
 * @brief Resources used during a render pass
 */
struct RenderResources : Resource<RenderResources>
{
  /// Target to render to
  RenderTargetHandle target = RenderTargetHandle::null();

  /// Shader program to use
  ShaderHandle shader = ShaderHandle::null();

  /// Draw buffer to use
  std::size_t buffer = 0UL;

  auto field_list() { return FieldList(Field{"target", target}, Field{"shader", shader}, Field{"buffer", buffer}); }

  bool isValid() const { return shader.isValid(); }
};

/**
 * @brief Standard values passed to shaders on render pass
 */
struct RenderUniforms : Resource<RenderUniforms>
{
  Mat3f world_from_camera = Mat3f::Identity();
  float scaling = 1.0F;
  TimeOffset time = TimeOffset::zero();
  TimeOffset time_delta = TimeOffset::zero();

  auto field_list()
  {
    return FieldList(
      Field{"world_from_camera", world_from_camera},
      Field{"scaling", scaling},
      Field{"time", time},
      Field{"time_delta", time_delta});
  }

  Mat3f getWorldFromViewportMatrix(const Vec2i& viewport_size) const;
};

/**
 * @brief Buffer mode
 */
enum class VertexBufferMode
{
  kStatic,
  kDynamic
};

std::ostream& operator<<(std::ostream& os, VertexBufferMode mode);

/**
 * @brief Buffer mode
 */
enum class VertexDrawMode
{
  kFilled,
  kWireFrame,
};

std::ostream& operator<<(std::ostream& os, VertexDrawMode mode);

/**
 * @brief Texture creation options
 */
struct VertexBufferOptions : Resource<VertexBufferOptions>
{
  std::size_t max_triangle_count_per_render_pass = 1000UL;
  VertexBufferMode buffer_mode = VertexBufferMode::kDynamic;
  VertexDrawMode draw_mode = VertexDrawMode::kFilled;

  // clang-format off
  auto field_list()
  {
    return FieldList(
      Field{"max_triangle_count_per_render_pass", max_triangle_count_per_render_pass},
      Field{"buffer_mode", buffer_mode},
      Field{"draw_mode", draw_mode}
    );
  }
  // clang-format on
};


/**
 * @brief Texture creation options
 */
struct Renderer2DOptions : Resource<Renderer2DOptions>
{
  // clang-format off
  sde::vector<VertexBufferOptions> buffers = {
    VertexBufferOptions{.draw_mode=VertexDrawMode::kFilled},
    VertexBufferOptions{.draw_mode=VertexDrawMode::kWireFrame}
  };
  // clang-format on

  auto field_list() { return FieldList(Field{"buffers", buffers}); }
};

struct RenderBackend
{};

enum class RendererError
{
  kRendererPreviouslyInitialized,
};

std::ostream& operator<<(std::ostream& os, RendererError value_type);

struct RenderStats
{
  std::size_t max_vertex_count = 0;
  std::size_t max_element_count = 0;
};

std::ostream& operator<<(std::ostream& os, const RenderStats& stats);

/**
 * @brief High-level interface into the rendering backend (for 2D objects)
 */
class Renderer2D
{
public:
  using dependencies = ResourceDependencies<RenderTargetCache, ShaderCache, TextureCache>;

  ~Renderer2D();

  Renderer2D(Renderer2D&& other);
  Renderer2D& operator=(Renderer2D&& other);

  void swap(Renderer2D& other);

  static expected<Renderer2D, RendererError> create(const Renderer2DOptions& options = {});

  void flush(const dependencies& deps, const RenderUniforms& uniforms, const Mat3f& viewport_from_world);

  void refresh(const RenderResources& resources);

  void assign(std::size_t unit, const TextureHandle& texture) { next_active_textures_[unit] = texture; }

  std::optional<std::size_t> assign(const TextureHandle& texture);

  const RenderStats& stats() const { return stats_; }

private:
  Renderer2D() = default;

  Renderer2D(const Renderer2D&) = delete;
  Renderer2D& operator=(const Renderer2D&) = delete;

  RenderStats stats_ = {};
  RenderResources last_active_resources_ = {};
  RenderResources next_active_resources_ = {};
  TextureUnits last_active_textures_ = {};
  TextureUnits next_active_textures_ = {};
  RenderBackend* backend_ = nullptr;
};


enum class RenderPassError
{
  kRenderPassActive,
  kInvalidRenderTarget,
  kMaxVertexCountExceeded,
  kMaxElementCountExceeded,
};

std::ostream& operator<<(std::ostream& os, RenderPassError error);

/**
 * @brief Encapsulates a single render pass
 */
class RenderPass
{
public:
  ~RenderPass();

  RenderPass(RenderPass&& other);
  RenderPass& operator=(RenderPass&& other);

  void swap(RenderPass& other);

  expected<void, RenderPassError> submit(View<const Quad> quads);
  expected<void, RenderPassError> submit(View<const Circle> circles);
  expected<void, RenderPassError> submit(View<const TexturedQuad> quads);

  std::optional<std::size_t> assign(const TextureHandle& texture) { return renderer_->assign(texture); }

  static expected<RenderPass, RenderPassError> create(
    RenderBuffer& buffer,
    Renderer2D& renderer,
    const Renderer2D::dependencies& deps,
    const RenderUniforms& uniforms,
    const RenderResources& resources,
    Vec2i viewport_size = Vec2i::Zero());

  const Mat3f& getWorldFromViewportMatrix() const { return world_from_viewport_; };
  const Mat3f& getViewportFromWorldMatrix() const { return viewport_from_world_; };
  const Bounds2f& getViewportInWorldBounds() const { return viewport_in_world_bounds_; };

  constexpr RenderBuffer* operator->() { return buffer_; }

  bool visible(const Bounds2f& query_aabb) const { return getViewportInWorldBounds().intersects(query_aabb); }

private:
  RenderPass() = default;

  RenderPass(const RenderPass& other) = delete;
  RenderPass& operator=(const RenderPass& other) = delete;

  RenderPass(
    Renderer2D* renderer,
    RenderBuffer* buffer,
    const RenderUniforms* uniforms,
    const Renderer2D::dependencies& deps,
    const Mat3f& world_from_viewport,
    const Mat3f& viewport_from_world,
    const Bounds2f& viewport_in_world_bounds);

  static bool retarget(Vec2i& viewport_size, RenderTargetHandle render_target, const Renderer2D::dependencies& deps);

  Renderer2D* renderer_ = nullptr;
  RenderBuffer* buffer_ = nullptr;
  const RenderUniforms* uniforms_ = nullptr;
  Renderer2D::dependencies deps_ = {};
  Mat3f world_from_viewport_ = {};
  Mat3f viewport_from_world_ = {};
  Bounds2f viewport_in_world_bounds_ = {};
};

}  // namespace sde::graphics
