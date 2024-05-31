// C++ Standard Library
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <optional>
#include <ostream>
#include <type_traits>
#include <vector>

// Backend
#include "opengl.inl"

// SDE
#include "sde/build.hpp"
#include "sde/format.hpp"
#include "sde/geometry_types.hpp"
#include "sde/geometry_utils.hpp"
#include "sde/graphics/render_target.hpp"
#include "sde/graphics/renderer.hpp"
#include "sde/graphics/shader.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/graphics/tile_map.hpp"
#include "sde/graphics/tile_set.hpp"
#include "sde/graphics/typedef.hpp"
#include "sde/logging.hpp"

#include <iostream>

namespace sde::graphics
{

namespace
{

template <GLenum Target, GLenum Access> auto* map_buffer(GLuint id)
{
  SDE_ASSERT_NE(id, 0);
  glBindBuffer(Target, id);
  return glMapBuffer(Target, Access);
}

template <GLenum Target> auto* map_write_only_buffer(GLuint id) { return map_buffer<Target, GL_WRITE_ONLY>(id); }

auto* map_write_only_vertex_buffer(GLuint id) { return map_write_only_buffer<GL_ARRAY_BUFFER>(id); }

auto* map_write_only_element_buffer(GLuint id) { return map_write_only_buffer<GL_ELEMENT_ARRAY_BUFFER>(id); }

template <GLenum Target> void unmap_buffer() { glUnmapBuffer(Target); }

void unmap_vertex_buffer() { unmap_buffer<GL_ARRAY_BUFFER>(); }

void unmap_element_buffer() { unmap_buffer<GL_ELEMENT_ARRAY_BUFFER>(); }

enum class VertexAccessMode
{
  kDirect,  ///< Specifies that fixed-point data values converted directly as fixed-point values when they are accessed
  kNormalized  ///< Specifies that fixed-point data values should be normalized when they are accessed
};

template <
  std::size_t Index,
  typename ElementT,
  std::size_t ElementCount,
  std::size_t InstanceDivisor = 0,
  VertexAccessMode AccessMode = VertexAccessMode::kDirect>
struct VertexAttribute
{
  static constexpr std::size_t kElementCount = ElementCount;
  static constexpr std::size_t kIndex = Index;
  static constexpr std::size_t kBytesPerVertex = ElementCount * sizeof(ElementT);

  /// Vertex count
  std::size_t vertex_count;

  explicit VertexAttribute(const std::size_t _vertex_count) : vertex_count{_vertex_count} {}
};

template <
  std::size_t Index,
  typename ElementT,
  std::size_t ElementCount,
  std::size_t InstanceDivisor,
  VertexAccessMode AccessMode>
std::size_t setAttribute(
  std::size_t offset_bytes,
  const VertexAttribute<Index, ElementT, ElementCount, InstanceDivisor, AccessMode>& attribute)
{
  static constexpr std::uint8_t* kOffsetStart = nullptr;
  static constexpr std::size_t kBytesPerVertex = ElementCount * sizeof(ElementT);

  const std::size_t total_bytes = kBytesPerVertex * attribute.vertex_count;

  glEnableVertexAttribArray(Index);

  glVertexAttribPointer(
    Index,  // layout index
    ElementCount,  // elementcount
    to_native_typecode(typecode<ElementT>()),  // typecode
    to_native_bool(AccessMode == VertexAccessMode::kNormalized),  // normalized
    kBytesPerVertex,  // stride
    static_cast<const GLvoid*>(kOffsetStart + offset_bytes)  // offset in buffer
  );

  glVertexAttribDivisor(Index, InstanceDivisor);

  return total_bytes;
}


constexpr std::size_t kElementsPerTriangle{3UL};
constexpr std::size_t kVerticesPerQuad{4UL};
constexpr std::size_t kVerticesPerCircle{17UL};


constexpr std::size_t kVertexAttributeIndex{0};
constexpr std::size_t kVertexElementIndex{1};
constexpr std::size_t kVertexBufferCount{2};


constexpr std::size_t kVertexAttributeCount{4};
using PositionAttribute = VertexAttribute<0, float, 2>;
using TexCoordAttribute = VertexAttribute<1, float, 2>;
using TexUnitAttribute = VertexAttribute<2, float, 1>;
using TintColorAttribute = VertexAttribute<3, float, 4>;


std::array<Vec2f, kVerticesPerCircle> kUnitCircleLookup{[] {
  std::array<Vec2f, kVerticesPerCircle> lookup;
  constexpr float kAngleStamp = static_cast<float>(2.0 * M_PI) / static_cast<float>(kVerticesPerCircle - 2);
  lookup[0] = {0.0F, 0.0F};
  for (std::size_t v = 1; v < kVerticesPerCircle; ++v)
  {
    const float angle = ((v - 1) * kAngleStamp);
    lookup[v] = {std::cos(angle), std::sin(angle)};
  }
  return lookup;
}()};


Vec2f* fillQuadPositions(Vec2f* target, const Vec2f min, const Vec2f max)
{
  target[0] = {min.x(), min.y()};
  target[1] = {min.x(), max.y()};
  target[2] = {max.x(), max.y()};
  target[3] = {max.x(), min.y()};
  return target + kVerticesPerQuad;
}


struct AttributeBuffers
{
  Vec2f* position = nullptr;
  Vec2f* texcoord = nullptr;
  float* texunit = nullptr;
  Vec4f* tint = nullptr;
  ~AttributeBuffers() { unmap_vertex_buffer(); }
};


struct ElementBuffer
{
  unsigned* indices = nullptr;
  ~ElementBuffer() { unmap_element_buffer(); }
};


Mat3f toInverseCameraMatrix(float scaling, float aspect)
{
  const float rxx = scaling * aspect;
  const float ryy = scaling;

  Mat3f m;
  // clang-format off
  m << rxx, 0.f, 0.f,
       0.f, ryy, 0.f,
       0.f, 0.f, 1.f;
  // clang-format on
  return m;
}


bool intersectsAABB(const Bounds2f& bounds, const Rect& rect)
{
  return bounds.intersects(Bounds2f{rect.min, rect.max});
}

bool intersectsAABB(const Bounds2f& bounds, const Quad& quad) { return intersectsAABB(bounds, quad.rect); }

bool intersectsAABB(const Bounds2f& bounds, const Circle& circle)
{
  const Vec2f extents{circle.radius, circle.radius};
  return bounds.intersects(Bounds2f{circle.center - extents, circle.center + extents});
}

bool intersectsAABB(const Bounds2f& bounds, const TexturedQuad& quad) { return intersectsAABB(bounds, quad.rect); }

bool intersectsAABB(const Bounds2f& bounds, const TileMap& tile_map)
{
  const Vec2f p_min = tile_map.position;
  const Vec2f p_max =
    (tile_map.position +
     Vec2f{tile_map.tile_size.x() * tile_map.tiles.cols(), -tile_map.tile_size.y() * tile_map.tiles.rows()});
  return bounds.intersects(toBounds(p_min, p_max));
}

class OpenGLBackend : public RenderBackend
{
public:
  OpenGLBackend(const Renderer2DOptions& options) :
      vao_main_vertex_count_max_{3UL * options.max_triangle_count_per_render_pass},
      vao_main_element_count_max_{3UL * options.max_triangle_count_per_render_pass},
      vao_main_vertex_count_{0},
      vao_main_element_count_{0}
  {
    {
      GLint actual_texture_unit_count;
      glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &actual_texture_unit_count);
      SDE_ASSERT_GE(actual_texture_unit_count, TextureUnits::kAvailable);
    }

    glGenVertexArrays(1, &vao_main_);
    glGenBuffers(2, vab_main_);

    glBindVertexArray(vao_main_);

    // Allocate element buffer
    {
      static constexpr std::size_t kBytesPerIndex = sizeof(GLuint);
      const std::size_t total_bytes = vao_main_element_count_max_ * kBytesPerIndex;
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vab_main_[kVertexElementIndex]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, total_bytes, nullptr, GL_DYNAMIC_DRAW);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      SDE_LOG_DEBUG_FMT("Allocated element mem: %lu bytes", total_bytes);
    }

    // Allocate attribute buffers
    {
      glBindBuffer(GL_ARRAY_BUFFER, vab_main_[kVertexAttributeIndex]);
      std::size_t total_bytes = 0;
      vab_main_attribute_byte_offets_[PositionAttribute::kIndex] = total_bytes;
      total_bytes += setAttribute(total_bytes, PositionAttribute{3UL * options.max_triangle_count_per_render_pass});
      vab_main_attribute_byte_offets_[TexCoordAttribute::kIndex] = total_bytes;
      total_bytes += setAttribute(total_bytes, TexCoordAttribute{3UL * options.max_triangle_count_per_render_pass});
      vab_main_attribute_byte_offets_[TexUnitAttribute::kIndex] = total_bytes;
      total_bytes += setAttribute(total_bytes, TexUnitAttribute{3UL * options.max_triangle_count_per_render_pass});
      vab_main_attribute_byte_offets_[TintColorAttribute::kIndex] = total_bytes;
      total_bytes += setAttribute(total_bytes, TintColorAttribute{3UL * options.max_triangle_count_per_render_pass});
      glBufferData(GL_ARRAY_BUFFER, total_bytes, nullptr, GL_DYNAMIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      SDE_LOG_DEBUG_FMT("Allocated attribute mem: %lu bytes", total_bytes);
    }
  }

  ~OpenGLBackend()
  {
    glDeleteBuffers(2, vab_main_);
    glDeleteVertexArrays(1, &vao_main_);
  }

  void start()
  {
    vao_main_vertex_count_ = 0;
    vao_main_element_count_ = 0;

    glBindVertexArray(vao_main_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vab_main_[kVertexElementIndex]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vab_main_[kVertexAttributeIndex]);
  }

  void finish()
  {
    glDrawElements(GL_TRIANGLES, vao_main_element_count_, GL_UNSIGNED_INT, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  expected<void, RenderPassError> submit(const Bounds2f& visible_bounds, View<const Quad> quads)
  {
    // Get vertex count before adding attributes
    const std::size_t start_vertex_count = vao_main_vertex_count_;

    // Add vertex attribute data
    std::size_t quad_count = 0;
    {
      auto b = getMainVertexAttributeBuffers();
      for (const auto& q : quads)
      {
        if (!intersectsAABB(visible_bounds, q))
        {
          continue;
        }

        const std::size_t next_vertex_count = vao_main_vertex_count_ + kVerticesPerQuad;

        if (next_vertex_count > vao_main_vertex_count_max_)
        {
          return make_unexpected(RenderPassError::kMaxVertexCountExceeded);
        }

        ++quad_count;

        static constexpr float kNoTextureUnitAssigned = -1.0F;

        // clang-format off
        b.position = fillQuadPositions(b.position, q.rect.min, q.rect.max);
        b.texcoord = std::fill_n(b.texcoord, kVerticesPerQuad, Vec2f::Zero());
        b.texunit = std::fill_n(b.texunit, kVerticesPerQuad, kNoTextureUnitAssigned);
        b.tint = std::fill_n(b.tint, kVerticesPerQuad, q.color);
        // clang-format on

        vao_main_vertex_count_ = next_vertex_count;
      }
    }

    // Add vertex element data
    if (quad_count == 0)
    {
      return {};
    }

    if (auto b = getMainVertexElementBuffer(); addQuadElements(b, quad_count, start_vertex_count))
    {
      return {};
    }

    return make_unexpected(RenderPassError::kMaxElementCountExceeded);
  }

  expected<void, RenderPassError> submit(const Bounds2f& visible_bounds, View<const TexturedQuad> quads)
  {
    // Get vertex count before adding attributes
    const std::size_t start_vertex_count = vao_main_vertex_count_;

    // Add vertex attribute data
    std::size_t quad_count = 0;
    {
      auto b = getMainVertexAttributeBuffers();
      for (const auto& q : quads)
      {
        if (!intersectsAABB(visible_bounds, q))
        {
          continue;
        }

        const std::size_t next_vertex_count = vao_main_vertex_count_ + kVerticesPerQuad;

        if (next_vertex_count > vao_main_vertex_count_max_)
        {
          return make_unexpected(RenderPassError::kMaxVertexCountExceeded);
        }

        ++quad_count;

        // clang-format off
        b.position = fillQuadPositions(b.position, q.rect.min, q.rect.max);
        b.texcoord = fillQuadPositions(b.texcoord, q.rect_texture.min, q.rect_texture.max);
        b.texunit = std::fill_n(b.texunit, kVerticesPerQuad, static_cast<float>(q.texture_unit));
        b.tint = std::fill_n(b.tint, kVerticesPerQuad, q.color);
        // clang-format on

        vao_main_vertex_count_ = next_vertex_count;
      }
    }

    // Add vertex element data
    if (quad_count == 0)
    {
      return {};
    }

    if (auto b = getMainVertexElementBuffer(); addQuadElements(b, quad_count, start_vertex_count))
    {
      return {};
    }

    return make_unexpected(RenderPassError::kMaxElementCountExceeded);
  }

  expected<void, RenderPassError> submit(const Bounds2f& visible_bounds, View<const Circle> circles)
  {
    // Get vertex count before adding attributes
    const std::size_t start_vertex_count = vao_main_vertex_count_;

    // Add vertex attribute data
    std::size_t circle_count = 0;
    {
      auto b = getMainVertexAttributeBuffers();
      for (const auto& c : circles)
      {
        if (!intersectsAABB(visible_bounds, c))
        {
          continue;
        }

        const std::size_t next_vertex_count = vao_main_vertex_count_ + kVerticesPerCircle;

        if (next_vertex_count > vao_main_vertex_count_max_)
        {
          return make_unexpected(RenderPassError::kMaxVertexCountExceeded);
        }

        ++circle_count;

        static constexpr float kNoTextureUnitAssigned = -1.0F;

        // clang-format off
        b.position = std::transform(std::begin(kUnitCircleLookup), std::end(kUnitCircleLookup), b.position,
                                    [&c](const Vec2f& unit) { return c.center + c.radius * unit; });
        b.texcoord = std::copy(std::begin(kUnitCircleLookup), std::end(kUnitCircleLookup), b.texcoord);
        b.texunit = std::fill_n(b.texunit, kVerticesPerCircle, kNoTextureUnitAssigned);
        b.tint = std::fill_n(b.tint, kVerticesPerCircle, c.color);
        // clang-format on

        vao_main_vertex_count_ = next_vertex_count;
      }
    }

    // Add vertex element data
    if (circle_count == 0)
    {
      return {};
    }

    if (auto b = getMainVertexElementBuffer(); addCircleElements(b, circle_count, start_vertex_count))
    {
      return {};
    }

    return make_unexpected(RenderPassError::kMaxElementCountExceeded);
  }

  expected<void, RenderPassError>
  submit(const Bounds2f& visible_bounds, View<const TileMap> tile_maps, const TileSet& tile_set)
  {
    // Get vertex count before adding attributes
    const std::size_t start_vertex_count = vao_main_vertex_count_;

    // Add vertex attribute data
    std::size_t quad_count = 0;
    {
      auto b = getMainVertexAttributeBuffers();
      for (const auto& tm : tile_maps)
      {
        if (!intersectsAABB(visible_bounds, tm))
        {
          continue;
        }

        const std::size_t next_vertex_count =
          vao_main_vertex_count_ + (kVerticesPerQuad * static_cast<std::size_t>(tm.tiles.size()));

        if (next_vertex_count > vao_main_vertex_count_max_)
        {
          return make_unexpected(RenderPassError::kMaxVertexCountExceeded);
        }

        for (int j = 0; j < tm.tiles.cols(); ++j)
        {
          for (int i = 0; i < tm.tiles.rows(); ++i)
          {
            const auto& texcoord = tile_set[tm.tiles(i, j)];
            const Vec2f pos_rect_max = tm.position + Vec2f{tm.tile_size.x() * j, -tm.tile_size.y() * i};
            const Vec2f pos_rect_min = pos_rect_max + Vec2f{tm.tile_size.x(), -tm.tile_size.y()};

            // clang-format off
            b.position = fillQuadPositions(b.position, pos_rect_min, pos_rect_max);
            b.texcoord = fillQuadPositions(b.texcoord, texcoord.min, texcoord.max);
            b.texunit = std::fill_n(b.texunit, kVerticesPerQuad, static_cast<float>(tm.texture_unit));
            b.tint = std::fill_n(b.tint, kVerticesPerQuad, tm.color);
            // clang-format on

            vao_main_vertex_count_ += kVerticesPerQuad;
          }
        }

        quad_count += static_cast<std::size_t>(tm.tiles.size());
      }
    }

    // Add vertex element data
    if (quad_count == 0)
    {
      return {};
    }

    if (auto b = getMainVertexElementBuffer(); addQuadElements(b, quad_count, start_vertex_count))
    {
      return {};
    }

    return make_unexpected(RenderPassError::kMaxElementCountExceeded);
  }

private:
  bool addQuadElements(ElementBuffer& buffer, const std::size_t n, std::size_t start_vertex_count)
  {
    auto* p = buffer.indices + vao_main_element_count_;
    for (std::size_t i = 0; i < n; ++i, start_vertex_count += kVerticesPerQuad)
    {
      const std::size_t next_element_count = vao_main_element_count_ + 2UL * kElementsPerTriangle;
      if (next_element_count > vao_main_element_count_max_)
      {
        return false;
      }

      // Lower face
      p[0] = start_vertex_count + 0;
      p[1] = start_vertex_count + 1;
      p[2] = start_vertex_count + 2;
      p += kElementsPerTriangle;

      // Upper face
      p[0] = start_vertex_count + 2;
      p[1] = start_vertex_count + 3;
      p[2] = start_vertex_count + 0;
      p += kElementsPerTriangle;


      vao_main_element_count_ = next_element_count;
    }
    return true;
  }

  bool addCircleElements(ElementBuffer& buffer, const std::size_t n, std::size_t start_vertex_count)
  {
    auto* p = buffer.indices + vao_main_element_count_;
    for (std::size_t i = 0; i < n; ++i)
    {
      const std::size_t next_element_count = vao_main_element_count_ + kElementsPerTriangle * (kVerticesPerCircle - 2);
      if (next_element_count > vao_main_element_count_max_)
      {
        return false;
      }

      const unsigned center_vertex_index = start_vertex_count;
      for (std::size_t e = 1; e < kVerticesPerCircle - 1; ++e)
      {
        p[0] = center_vertex_index;
        p[1] = p[0] + e;
        p[2] = p[1] + 1;

        p += kElementsPerTriangle;
      }
      p[2] = center_vertex_index + 1;

      start_vertex_count += kVerticesPerCircle;
      vao_main_element_count_ = next_element_count;
    }
    return true;
  }

  template <typename R, typename VertexAttributeT, typename ByteT>
  [[nodiscard]] R* getVertexAttributeBuffer(ByteT* mapped_buffer)
  {
    static_assert(sizeof(R) == VertexAttributeT::kBytesPerVertex);
    return reinterpret_cast<R*>(
      reinterpret_cast<std::uint8_t*>(mapped_buffer) + vab_main_attribute_byte_offets_[VertexAttributeT::kIndex]);
  }

  AttributeBuffers getMainVertexAttributeBuffers()
  {
    auto* mapped_ptr = map_write_only_vertex_buffer(vab_main_[kVertexAttributeIndex]);
    return {
      .position = getVertexAttributeBuffer<Vec2f, PositionAttribute>(mapped_ptr) + vao_main_vertex_count_,
      .texcoord = getVertexAttributeBuffer<Vec2f, TexCoordAttribute>(mapped_ptr) + vao_main_vertex_count_,
      .texunit = getVertexAttributeBuffer<float, TexUnitAttribute>(mapped_ptr) + vao_main_vertex_count_,
      .tint = getVertexAttributeBuffer<Vec4f, TintColorAttribute>(mapped_ptr) + vao_main_vertex_count_};
  }

  ElementBuffer getMainVertexElementBuffer()
  {
    auto* mapped_ptr = map_write_only_element_buffer(vab_main_[kVertexElementIndex]);
    return {.indices = (reinterpret_cast<unsigned*>(mapped_ptr) + vao_main_element_count_)};
  }

  std::size_t vao_main_vertex_count_max_;
  std::size_t vao_main_element_count_max_;
  std::size_t vao_main_vertex_count_;
  std::size_t vao_main_element_count_;
  GLuint vao_main_;
  GLuint vab_main_[kVertexBufferCount];
  std::size_t vab_main_attribute_byte_offets_[kVertexAttributeCount];
};

std::optional<OpenGLBackend> backend__opengl;
std::atomic_flag backend__render_pass_active;

}  // namespace

Mat3f RenderAttributes::getWorldFromViewportMatrix(const RenderTarget& target) const
{
  return toInverseCameraMatrix(scaling, target.getLastAspectRatio()) * world_from_camera;
}

expected<Renderer2D, RendererError>
Renderer2D::create(const ShaderCache* shader_cache, const TextureCache* texture_cache, const Renderer2DOptions& options)
{
  if (backend__opengl.has_value())
  {
    return make_unexpected(RendererError::kRendererPreviouslyInitialized);
  }

  SDE_ASSERT_NE(shader_cache, nullptr);
  SDE_ASSERT_NE(texture_cache, nullptr);

  // Initialize rendering backend
  backend__opengl.emplace(options);

  Renderer2D renderer;
  renderer.shader_cache_ = shader_cache;
  renderer.texture_cache_ = texture_cache;
  renderer.backend_ = std::addressof(*backend__opengl);
  return renderer;
}

Renderer2D::~Renderer2D()
{
  if ((backend_ == nullptr) or (!backend__opengl.has_value()))
  {
    return;
  }
  backend__opengl.reset();
}

Renderer2D::Renderer2D(Renderer2D&& other) :
    shader_cache_{other.shader_cache_},
    texture_cache_{other.texture_cache_},
    active_resources_{other.active_resources_},
    backend_{other.backend_}
{
  other.backend_ = nullptr;
}

Mat3f Renderer2D::refresh(RenderTarget& target, const RenderAttributes& attributes, const RenderResources& resources)
{
  target.refresh(Vec4f::Zero());

  SDE_ASSERT_TRUE(resources.isValid());

  const auto* shader = shader_cache_->get(resources.shader);

  SDE_ASSERT_NE(shader, nullptr);

  // Set active shader
  if (resources.shader != active_resources_.shader)
  {
    SDE_ASSERT_NE(shader, nullptr);
    glUseProgram(shader->native_id);
    active_resources_.textures.reset();
  }

  // Set active texture units
  for (std::size_t u = 0; u < resources.textures.slots.size(); ++u)
  {
    if (resources.textures[u] and resources.textures[u] != active_resources_.textures[u])
    {
      const auto* texture = texture_cache_->get(resources.textures[u]);
      SDE_ASSERT_NE(texture, nullptr);

      glActiveTexture(GL_TEXTURE0 + u);
      glBindTexture(GL_TEXTURE_2D, texture->native_id);
      glUniform1i(glGetUniformLocation(shader->native_id, format("uTexture[%lu]", u)), u);
    }
  }

  // Apply other variables
  glUniform1f(glGetUniformLocation(shader->native_id, "uTime"), attributes.time);
  glUniform1f(glGetUniformLocation(shader->native_id, "uTimeDelta"), attributes.time_delta);

  const Mat3f world_from_viewport = attributes.getWorldFromViewportMatrix(target);
  const Mat3f viewport_from_world = world_from_viewport.inverse();

  glUniformMatrix3fv(
    glGetUniformLocation(shader->native_id, "uCameraTransform"), 1, GL_FALSE, viewport_from_world.data());

  backend__opengl->start();

  return world_from_viewport;
}

void Renderer2D::flush() { backend__opengl->finish(); }

expected<void, RenderPassError> RenderPass::submit(View<const Quad> quads)
{
  return backend__opengl->submit(viewport_in_world_bounds_, quads);
}

expected<void, RenderPassError> RenderPass::submit(View<const Circle> circles)
{
  return backend__opengl->submit(viewport_in_world_bounds_, circles);
}

expected<void, RenderPassError> RenderPass::submit(View<const TexturedQuad> quads)
{
  return backend__opengl->submit(viewport_in_world_bounds_, quads);
}

expected<void, RenderPassError> RenderPass::submit(View<const TileMap> tile_maps, const TileSet& tile_set)
{
  return backend__opengl->submit(viewport_in_world_bounds_, tile_maps, tile_set);
}

RenderPass::RenderPass(RenderPass&& other) :
    renderer_{other.renderer_},
    world_from_viewport_{other.world_from_viewport_},
    viewport_in_world_bounds_{other.viewport_in_world_bounds_}
{
  other.renderer_ = nullptr;
}

RenderPass::~RenderPass()
{
  if (renderer_ == nullptr)
  {
    return;
  }
  backend__opengl->finish();
  backend__render_pass_active.clear();
}

expected<RenderPass, RenderPassError> RenderPass::create(
  RenderTarget& target,
  Renderer2D& renderer,
  const RenderAttributes& attributes,
  const RenderResources& resources)
{
  if (backend__render_pass_active.test_and_set())
  {
    return make_unexpected(RenderPassError::kRenderPassActive);
  }

  RenderPass render_pass;
  render_pass.renderer_ = std::addressof(renderer);
  render_pass.world_from_viewport_ = renderer.refresh(target, attributes, resources);
  render_pass.viewport_in_world_bounds_ =
    transform(render_pass.world_from_viewport_, Bounds2f{-Vec2f::Ones(), Vec2f::Ones()});

  return render_pass;
}

std::ostream& operator<<(std::ostream& os, const RenderResources& resources)
{
  return os << "shader: " << resources.shader << "\ntexture-units:\n" << resources.textures;
}

std::ostream& operator<<(std::ostream& os, const RenderAttributes& attributes)
{
  // clang-format off
  return os << "world_from_camera:\n" << attributes.world_from_camera
            << "\ntime: " << attributes.time
            << " (delta: " << attributes.time_delta << ')'
            << "\nscaling: " << attributes.scaling;
  // clang-format on
}

std::ostream& operator<<(std::ostream& os, const RenderPass& render_pass) { return os; }

}  // namespace sde::graphics
