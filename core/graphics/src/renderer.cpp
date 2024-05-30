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

void draw_triangle_elements(GLuint vao, std::size_t element_count)
{
  glDrawElements(GL_TRIANGLES, element_count, GL_UNSIGNED_INT, 0);
}

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


std::array<Vec2f, kVerticesPerCircle> kUnitCircleLookup
{
  [] {
    std::array<Vec2f, kVerticesPerCircle> lookup;
    constexpr float kAngleStamp = static_cast<float>(2.0 * M_PI) / static_cast<float>(kVerticesPerCircle - 2);
    lookup[0] = {0.0F, 0.0F};
    for (std::size_t v = 1; v < kVerticesPerCircle; ++v)
    {
      const float angle = ((v - 1) * kAngleStamp);
      lookup[v] = {std::cos(angle), std::sin(angle)};
    }
    return lookup
  }()
}


void fillQuadPositions(Vec2f* target, const Vec2f min, const Vec2f max)
{
  target[0] = {min.x(), min.y()};
  target[1] = {min.x(), max.y()};
  target[2] = {max.x(), max.y()};
  target[3] = {max.x(), min.y()};
}


struct AttributeBuffers
{
  std::size_t vertex_count = 0;

  Vec2f* position = nullptr;
  Vec2f* texcoord = nullptr;
  float* texunit = nullptr;
  Vec4f* tint = nullptr;

  ~AttributeBuffers() { unmap_vertex_buffer(); }
};


struct ElementBuffer
{
  unsigned* indices = nullptr;
  std::size_t vertex_count = 0;
  std::size_t element_count = 0;

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


bool intersectsAABB(const Bounds2f& bounds, const Line& line)
{
  return bounds.intersects(toBounds(line.tail, line.head));
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
      vao_main_vertex_count_{0},
      vao_main_vertex_count_max_{3UL * options.max_triangle_count_per_render_pass},
      vao_main_element_count_{0},
      vao_main_element_count_max_{3UL * options.max_triangle_count_per_render_pass}
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
    glBindVertexArray(vao_main_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vab_main_[kVertexElementIndex]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vab_main_[kVertexAttributeIndex]);
  }

  void finish()
  {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    vao_main_vertex_count_ = 0;
    vao_main_element_count_ = 0;
  }

  void submit(const Bounds2f& visible_bounds, View<const Quad> quads)
  {
    // Add vertex attribute data
    {
      auto b = getMainVertexAttributeBuffers();
      std::size_t visible_count = 0;
      for (const auto& q : quads)
      {
        if (intersectsAABB(visible_bounds, q))
        {
          static constexpr float kNoTextureUnitAssigned = -1.0F;
          fillQuadPositions(b.position + vao_main_vertex_count_, q.rect.min, q.rect.max);
          b.texcoord = std::fill_n(b.texcoord, kVerticesPerQuad, Vec2f::Zero());
          b.texunit = std::fill_n(b.texunit, kVerticesPerQuad, kNoTextureUnitAssigned);
          b.tint = std::fill_n(b.tint, kVerticesPerQuad, q.color);
          vao_main_vertex_count_ += kVerticesPerQuad;
          ++visible_count;
        }
      }
    }

    // Add vertex element data
    if (visible_count > 0)
    {
      auto b = getMainVertexElementBuffer();
      addQuadElements(b, visible_count);
    }
  }

  void submit(const Bounds2f& visible_bounds, View<const TexturedQuad> quads)
  {
    // Add vertex attribute data
    {
      auto b = getMainVertexAttributeBuffers();
      std::size_t visible_count = 0;
      for (const auto& q : quads)
      {
        if (intersectsAABB(visible_bounds, q))
        {
          b.position = fillQuadPositions(b.position, q.rect.min, q.rect.max);
          b.texcoord = fillQuadPositions(b.texcoord, q.rect_texture.min, q.rect_texture.max);
          buffers.texunit = std::fill_n(buffers.texunit, kVerticesPerQuad, static_cast<float>(q.texture_unit));
          buffers.tint = std::fill_n(buffers.tint, kVerticesPerQuad, q.color);
          vao_main_vertex_count_ += kVerticesPerQuad;
          ++visible_count;
        }
      }
    }

    // Add vertex element data
    if (visible_count > 0)
    {
      auto b = getMainVertexElementBuffer();
      addQuadElements(b, visible_count);
    }
  }

  void submit(const Bounds2f& visible_bounds, View<const Circle> circles)
  {
    // Get vertex count before adding attributes
    const std::size_t = start_vertex_count = vao_main_vertex_count_;

    // Add vertex attribute data
    {
      auto b = getMainVertexAttributeBuffers();
      std::size_t visible_count = 0;
      for (const auto& c : circles)
      {
        if (intersectsAABB(visible_bounds, q))
        {
          static constexpr float kNoTextureUnitAssigned = -1.0F;
          // clang-format off
          b.position = std::transform(std::begin(kUnitCircleLookup), std::end(kUnitCircleLookup), b.position, [&c](const Vec2f& unit) { return c.center + c.radius * unit; });
          b.texcoord = std::copy(std::begin(kUnitCircleLookup), std::end(kUnitCircleLookup), b.texcoord);
          b.texunit = std::fill_n(b.texunit, kVerticesPerCircle, kNoTextureUnitAssigned);
          b.tint = std::fill_n(b.tint, kVerticesPerCircle, c.color);
          vao_main_vertex_count_ += kVerticesPerCircle;
          // clang-format on
          ++visible_count;
        }
      }
    }

    // Add vertex element data
    if (visible_count > 0)
    {
      auto b = getMainVertexElementBuffer();
      addCircleElements(b, visible_count, start_vertex_count);
    }
  }

private:
  static void addQuadElements(ElementBuffer& buffer, std::size_t quad_count)
  {
    auto* p = buffer.indices + vao_main_element_count_;
    for (std::size_t n = 0; n < quad_count; ++n)
    {
      // Lower face
      p[0] = buffer.vertex_count + 0;
      p[1] = buffer.vertex_count + 1;
      p[2] = buffer.vertex_count + 2;
      p += kElementsPerTriangle;
      vao_main_element_count_ += kElementsPerTriangle;

      // Upper face
      p[0] = buffer.vertex_count + 2;
      p[1] = buffer.vertex_count + 3;
      p[2] = buffer.vertex_count + 0;
      p += kElementsPerTriangle;
      vao_main_element_count_ += kElementsPerTriangle;
    }
  }

  static void addCircleElements(ElementBuffer& buffer, std::size_t circle_count, std::size_t start_vertex_count)
  {
    for (std::size_t n = 0; n < circle_count; ++n)
    {
      auto* p = buffer.indices + vao_main_element_count_;

      const unsigned center_vertex_index = buffer.vertex_count;
      for (std::size_t e = 1; e < kVerticesPerCircle - 1; ++e)
      {
        p[0] = center_vertex_index;
        p[1] = p[0] + e;
        p[2] = p[1] + 1;

        p += kElementsPerTriangle;
        vao_main_element_count_ += kElementsPerTriangle;
      }
      p[2] = center_vertex_index + 1;
    }
  }

  static std::size_t addTileMapAttributes(AttributeBuffers& buffers, const Bounds2f& visible_bounds, const Layer& layer)
  {
    std::size_t submission_count = 0;
    for (const auto& tm : layer.tile_maps)
    {
      SDE_ASSERT_TRUE(tm.info.isValid());
      if (intersectsAABB(visible_bounds, tm))
      {
        for (int j = 0; j < tm.tiles.cols(); ++j)
        {
          for (int i = 0; i < tm.tiles.rows(); ++i)
          {
            const auto tile_index = tm.tiles(i, j);
            const auto& tex = tm.info.getTileRects()[tile_index];
            const Vec2f p_max = tm.position + Vec2f{tm.tile_size.x() * j, -tm.tile_size.y() * i};
            const Vec2f p_min = p_max + Vec2f{tm.tile_size.x(), -tm.tile_size.y()};
            fillQuadPositions(buffers.position + buffers.vertex_count, p_min, p_max);
            fillQuadPositions(buffers.texcoord + buffers.vertex_count, tex.min, tex.max);
            std::fill_n(
              buffers.texunit + buffers.vertex_count, kVerticesPerQuad, static_cast<float>(tm.info.getTextureUnit()));
            std::fill_n(buffers.tint + buffers.vertex_count, kVerticesPerQuad, tm.color);
            buffers.vertex_count += kVerticesPerQuad;
          }
        }
        ++submission_count;
      }
    }
    return submission_count;
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
  std::size_t vao_main_vertex_count_;
  std::size_t vao_main_element_count_max_;
  std::size_t vao_main_element_count_;
  GLuint vao_main_;
  GLuint vab_main_[kVertexBufferCount];
  std::size_t vab_main_attribute_byte_offets_[kVertexAttributeCount];
};

std::optional<OpenGLBackend> backend__opengl;
std::atomic_flag backend__render_pass_active = false;

}  // namespace

Mat3f LayerAttributes::getWorldFromViewportMatrix(const RenderTarget& target) const
{
  return toInverseCameraMatrix(scaling, target.getLastAspectRatio()) * world_from_camera;
}

static expected<Renderer2D, RendererError>
Renderer2D::create(const ShaderCache* shader_cache, const TextureCache* texture_cache, const Renderer2DOptions& options)
{
  if (backend__opengl.has_value())
  {
    return make_unexpected(RendererError::kRendererAlreadyInitialized);
  }

  SDE_ASSERT_NE(shader_cache_, nullptr);
  SDE_ASSERT_NE(texture_cache_, nullptr);

  // Initialize rendering backend
  backend__opengl.emplace(options);

  Renderer2D renderer;
  renderer.shader_cache_ = shader_cache;
  renderer.texture_cache_ = texture_cache;
  renderer.backend_ = std::addressof(*backend__opengl);
  return renderer
}

Renderer2D::~Renderer2D()
{
  if ((backend == nullptr) or (!backend__opengl.has_value()))
  {
    return;
  }
  backend__opengl.reset();
}

Renderer2D Renderer2D(Renderer2D&& other) :
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

  const auto* shader = shader_cache.get(resources.shader);

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
      const auto* texture = texture_cache.get(layer.resources.textures[u]);
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

void RenderPass::submit(View<const Quad> quads) { backend__opengl->submit(rp->getViewportInWorldBounds(), quads); }

void RenderPass::submit(View<const Circle> circles)
{
  backend__opengl->submit(rp->getViewportInWorldBounds(), circles);
}

void RenderPass::submit(View<const TexutreQuad> quads)
{
  backend__opengl->submit(rp->getViewportInWorldBounds(), quads);
}

void RenderPass::submit(const TileMap& tile_map, const TileSet& tile_set)
{
  backend__opengl->submit(rp->getViewportInWorldBounds(), tile_map, tile_set);
}

RenderPass::~RenderPass()
{
  if (renderer_ == nullptr)
  {
    return;
  }
  backend__opengl->finish();
  backend__render_pass_active = false;
}

expected<RenderPass, RenderPassError> RenderPass::create(
  RenderTarget& target,
  Renderer2D& renderer,
  const RenderAttributes& attributes,
  const RenderResources& resources)
{
  if (backend__render_pass_active)
  {
    return make_unexpected(RenderPassError::kRenderPassActive);
  }

  RenderPass render_pass;
  render_pass.renderer = std::addressof(renderer);
  render_pass.world_from_viewport_ = renderer.refresh(target, attributes, resources);
  render_pass.viewport_in_world_bounds_ =
    transform(render_pass.world_from_viewport_, Bounds2f{-Vec2f::Ones(), Vec2f::Ones()});

  backend__render_pass_active = true;

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
