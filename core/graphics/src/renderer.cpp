// C++ Standard Library
#include <algorithm>
#include <array>
#include <cmath>
#include <mutex>
#include <ostream>
#include <type_traits>
#include <vector>

// Backend
#include "opengl.inl"

// SDE
#include "sde/build.hpp"
#include "sde/geometry_types.hpp"
#include "sde/geometry_utils.hpp"
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

constexpr std::size_t kVertexAttributeIndex = 0;
constexpr std::size_t kVertexElementIndex = 1;

constexpr std::size_t kBufferCount = 2;
constexpr std::size_t kAttributeCount = 4;

using PositionAttribute = VertexAttribute<0, float, 2>;
using TexCoordAttribute = VertexAttribute<1, float, 2>;
using TexUnitAttribute = VertexAttribute<2, float, 1>;
using TintColorAttribute = VertexAttribute<3, float, 4>;

constexpr std::size_t kElementsPerTriangle = 3UL;

constexpr std::size_t kVerticesPerQuad = 4UL;

constexpr std::size_t kVerticesPerCircle = 17UL;

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

Mat3f getInverseCameraMatrix(float scaling, float aspect)
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

}  // namespace

class Renderer2D::Backend
{
public:
  Backend(const Renderer2DOptions& options)
  {
    {
      GLint actual_texture_unit_count;
      glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &actual_texture_unit_count);
      SDE_ASSERT_GE(actual_texture_unit_count, TextureUnits::kAvailable);
    }

    glGenVertexArrays(1, &vao_);
    glGenBuffers(2, vab_);

    glBindVertexArray(vao_);

    // Allocate element buffer
    {
      static constexpr std::size_t kBytesPerIndex = sizeof(GLuint);

      const std::size_t total_bytes = 3UL * options.max_triangle_count_per_layer * kBytesPerIndex;

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vab_[kVertexElementIndex]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, total_bytes, nullptr, GL_DYNAMIC_DRAW);

      SDE_LOG_DEBUG_FMT("Allocated element mem: %lu bytes", total_bytes);
    }

    // Allocate attribute buffers
    {
      glBindBuffer(GL_ARRAY_BUFFER, vab_[kVertexAttributeIndex]);

      std::size_t total_bytes = 0;

      vab_attribute_byte_offets_[PositionAttribute::kIndex] = total_bytes;
      total_bytes += setAttribute(total_bytes, PositionAttribute{3UL * options.max_triangle_count_per_layer});

      vab_attribute_byte_offets_[TexCoordAttribute::kIndex] = total_bytes;
      total_bytes += setAttribute(total_bytes, TexCoordAttribute{3UL * options.max_triangle_count_per_layer});

      vab_attribute_byte_offets_[TexUnitAttribute::kIndex] = total_bytes;
      total_bytes += setAttribute(total_bytes, TexUnitAttribute{3UL * options.max_triangle_count_per_layer});

      vab_attribute_byte_offets_[TintColorAttribute::kIndex] = total_bytes;
      total_bytes += setAttribute(total_bytes, TintColorAttribute{3UL * options.max_triangle_count_per_layer});

      glBufferData(GL_ARRAY_BUFFER, total_bytes, nullptr, GL_DYNAMIC_DRAW);

      SDE_LOG_DEBUG_FMT("Allocated attribute mem: %lu bytes", total_bytes);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  ~Backend()
  {
    glDeleteBuffers(2, vab_);
    glDeleteVertexArrays(1, &vao_);
  }

  void draw(const Layer& layer, const Bounds2f clipping_bounds)
  {
    glBindVertexArray(vao_);

    struct Submissions
    {
      std::size_t vertices = 0;
      std::size_t quads = 0;
      std::size_t circles = 0;
      std::size_t tile_maps = 0;
    };

    // Fill vertex attributes
    const auto submissions = [this, &layer, &clipping_bounds] {
      Submissions submissions;
      auto buffers = getVertexAttributeBuffers();
      submissions.quads += addQuadAttributes(buffers, clipping_bounds, layer);
      submissions.quads += addTexturedQuadAttributes(buffers, clipping_bounds, layer);
      submissions.circles += addCircleAttributes(buffers, clipping_bounds, layer);
      submissions.tile_maps += addTileMapAttributes(buffers, clipping_bounds, layer);
      submissions.vertices = buffers.vertex_count;
      return submissions;
    }();

    // Fill vertex elements
    const std::size_t element_count = [this, &submissions] {
      auto buffers = getVertexElementBuffer();
      addQuadElements(buffers, submissions.quads);
      addCircleElements(buffers, submissions.circles);
      addQuadElements(buffers, submissions.tile_maps * TileMap::kTileCount);
      SDE_ASSERT_EQ(submissions.vertices, buffers.vertex_count);
      return buffers.element_count;
    }();

    draw_triangle_elements(vao_, element_count);

    glBindVertexArray(0);
  }

private:
  static void addQuadElements(ElementBuffer& buffer, std::size_t quad_count)
  {
    auto* p = buffer.indices + buffer.element_count;
    for (std::size_t n = 0; n < quad_count; ++n)
    {
      // Lower face
      p[0] = buffer.vertex_count + 0;
      p[1] = buffer.vertex_count + 1;
      p[2] = buffer.vertex_count + 2;
      p += kElementsPerTriangle;
      buffer.element_count += kElementsPerTriangle;

      // Upper face
      p[0] = buffer.vertex_count + 2;
      p[1] = buffer.vertex_count + 3;
      p[2] = buffer.vertex_count + 0;
      p += kElementsPerTriangle;
      buffer.element_count += kElementsPerTriangle;

      buffer.vertex_count += kVerticesPerQuad;
    }
  }

  static void addCircleElements(ElementBuffer& buffer, std::size_t circle_count)
  {
    for (std::size_t n = 0; n < circle_count; ++n)
    {
      auto* p = buffer.indices + buffer.element_count;

      const unsigned center_vertex_index = buffer.vertex_count;
      for (std::size_t e = 1; e < kVerticesPerCircle - 1; ++e)
      {
        p[0] = center_vertex_index;
        p[1] = p[0] + e;
        p[2] = p[1] + 1;

        p += kElementsPerTriangle;
        buffer.element_count += kElementsPerTriangle;
      }
      p[2] = center_vertex_index + 1;
      buffer.vertex_count += kVerticesPerCircle;
    }
  }

  static std::size_t addQuadAttributes(AttributeBuffers& buffers, const Bounds2f& clipping_bounds, const Layer& layer)
  {
    std::size_t submission_count = 0;
    for (const auto& q : layer.quads)
    {
      if (intersectsAABB(clipping_bounds, q))
      {
        fillQuadPositions(buffers.position + buffers.vertex_count, q.rect.min, q.rect.max);
        std::fill_n(buffers.texcoord + buffers.vertex_count, kVerticesPerQuad, Vec2f::Zero());
        std::fill_n(buffers.texunit + buffers.vertex_count, kVerticesPerQuad, -1);
        std::fill_n(buffers.tint + buffers.vertex_count, kVerticesPerQuad, q.color);
        buffers.vertex_count += kVerticesPerQuad;
        ++submission_count;
      }
    }
    return submission_count;
  }

  static std::size_t
  addTexturedQuadAttributes(AttributeBuffers& buffers, const Bounds2f& clipping_bounds, const Layer& layer)
  {
    std::size_t submission_count = 0;
    for (const auto& q : layer.textured_quads)
    {
      if (intersectsAABB(clipping_bounds, q))
      {
        fillQuadPositions(buffers.position + buffers.vertex_count, q.rect.min, q.rect.max);
        fillQuadPositions(buffers.texcoord + buffers.vertex_count, q.rect_texture.min, q.rect_texture.max);
        std::fill_n(buffers.texunit + buffers.vertex_count, kVerticesPerQuad, static_cast<float>(q.texture_unit));
        std::fill_n(buffers.tint + buffers.vertex_count, kVerticesPerQuad, q.color);
        buffers.vertex_count += kVerticesPerQuad;
        ++submission_count;
      }
    }
    return submission_count;
  }

  static std::size_t
  addTileMapAttributes(AttributeBuffers& buffers, const Bounds2f& clipping_bounds, const Layer& layer)
  {
    std::size_t submission_count = 0;
    for (const auto& tm : layer.tile_maps)
    {
      SDE_ASSERT_TRUE(tm.info.isValid());
      if (intersectsAABB(clipping_bounds, tm))
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

  static std::size_t addCircleAttributes(AttributeBuffers& buffers, const Bounds2f& clipping_bounds, const Layer& layer)
  {
    static_assert(kVerticesPerCircle > 4);

    static std::array<Vec2f, kVerticesPerCircle> kUnitLookup;
    static std::once_flag kInitLookup;
    std::call_once(kInitLookup, [] {
      constexpr float kAngleStamp = static_cast<float>(2.0 * M_PI) / static_cast<float>(kVerticesPerCircle - 2);
      kUnitLookup[0] = {0.0F, 0.0F};
      for (std::size_t v = 1; v < kVerticesPerCircle; ++v)
      {
        const float angle = ((v - 1) * kAngleStamp);
        kUnitLookup[v] = {std::cos(angle), std::sin(angle)};
      }
    });

    std::size_t submission_count = 0;
    for (const auto& c : layer.circles)
    {
      if (intersectsAABB(clipping_bounds, c))
      {
        // clang-format off
        std::transform(
          std::begin(kUnitLookup),
          std::end(kUnitLookup),
          buffers.position + buffers.vertex_count,
          [&c](const Vec2f& unit)
          {
           return c.center + c.radius * unit;
          });
        std::copy(
          std::begin(kUnitLookup),
          std::end(kUnitLookup),
          buffers.texcoord + buffers.vertex_count
        );
        std::fill_n(buffers.texunit + buffers.vertex_count, kVerticesPerCircle, -1);
        std::fill_n(buffers.tint + buffers.vertex_count, kVerticesPerCircle, c.color);
        buffers.vertex_count += kVerticesPerCircle;
        // clang-format on

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
      reinterpret_cast<std::uint8_t*>(mapped_buffer) + vab_attribute_byte_offets_[VertexAttributeT::kIndex]);
  }

  AttributeBuffers getVertexAttributeBuffers()
  {
    auto* mapped_ptr = map_write_only_vertex_buffer(vab_[kVertexAttributeIndex]);
    return {
      .position = getVertexAttributeBuffer<Vec2f, PositionAttribute>(mapped_ptr),
      .texcoord = getVertexAttributeBuffer<Vec2f, TexCoordAttribute>(mapped_ptr),
      .texunit = getVertexAttributeBuffer<float, TexUnitAttribute>(mapped_ptr),
      .tint = getVertexAttributeBuffer<Vec4f, TintColorAttribute>(mapped_ptr)};
  }

  ElementBuffer getVertexElementBuffer()
  {
    auto* mapped_ptr = map_write_only_element_buffer(vab_[kVertexElementIndex]);
    return {.indices = reinterpret_cast<unsigned*>(mapped_ptr)};
  }

  GLuint vao_;
  GLuint vab_[kBufferCount];
  std::size_t vab_attribute_byte_offets_[kAttributeCount];
};

void Layer::reset()
{
  if (is_static)
  {
    return;
  }
  quads.clear();
  textured_quads.clear();
  circles.clear();
  tile_maps.clear();
}

Mat3f LayerSettings::getWorldFromViewportMatrix() const
{
  return getInverseCameraMatrix(this->scaling, this->aspect_ratio) * this->world_from_camera;
}

bool Layer::drawable() const
{
  return resources.isValid() and !(quads.empty() and textured_quads.empty() and circles.empty() and tile_maps.empty());
}

Renderer2D::~Renderer2D() = default;

Renderer2D::Renderer2D(const Renderer2DOptions& options) :
    active_resources_{}, backend_{std::make_unique<Backend>(options)}
{}

void Renderer2D::submit(const ShaderCache& shader_cache, const TextureCache& texture_cache, Layer& layer)
{
  if (layer.drawable())
  {
    const auto* shader = shader_cache.get(layer.resources.shader);
    SDE_ASSERT_NE(shader, nullptr);

    // Set active shader
    if (layer.resources.shader != active_resources_.shader)
    {
      SDE_ASSERT_NE(shader, nullptr);
      glUseProgram(shader->native_id);
      active_resources_.textures.reset();
    }

    // Set active texture units
    for (std::size_t u = 0; u < layer.resources.textures.slots.size(); ++u)
    {
      if (layer.resources.textures[u] and layer.resources.textures[u] != active_resources_.textures[u])
      {
        const auto* texture = texture_cache.get(layer.resources.textures[u]);
        SDE_ASSERT_NE(texture, nullptr);

        glActiveTexture(GL_TEXTURE0 + u);
        glBindTexture(GL_TEXTURE_2D, texture->native_id);
        glUniform1i(glGetUniformLocation(shader->native_id, "uTexture"), u);
      }
    }

    // Apply other variables
    glUniform1f(glGetUniformLocation(shader->native_id, "uTime"), layer.settings.time);
    glUniform1f(glGetUniformLocation(shader->native_id, "uTimeDelta"), layer.settings.time_delta);

    const Mat3f world_from_viewport = layer.settings.getWorldFromViewportMatrix();
    const Mat3f viewport_from_world = world_from_viewport.inverse();

    glUniformMatrix3fv(
      glGetUniformLocation(shader->native_id, "uCameraTransform"), 1, GL_FALSE, viewport_from_world.data());

    // Pre-sort tilemaps by texture unit
    std::sort(std::begin(layer.tile_maps), std::end(layer.tile_maps), [](const auto& lhs, const auto& rhs) {
      return lhs.info.getTextureUnit() < rhs.info.getTextureUnit();
    });

    // Call backend
    backend_->draw(layer, transform(world_from_viewport, Bounds2f{-Vec2f::Ones(), Vec2f::Ones()}));
  }

  layer.reset();
}

std::ostream& operator<<(std::ostream& os, const LayerResources& resources)
{
  return os << "shader: " << resources.shader << "\ntexture-units:\n" << resources.textures;
}

std::ostream& operator<<(std::ostream& os, const LayerSettings& settings)
{
  return os << "world_from_camera:\n"
            << settings.world_from_camera << "\ntime: " << settings.time << " (delta: " << settings.time_delta << ')'
            << "\nscaling: " << settings.scaling << "\naspect: " << settings.aspect_ratio;
}

std::ostream& operator<<(std::ostream& os, const Layer& layer)
{
  return os << layer.resources << layer.settings << "\nquads: " << layer.quads.size()
            << "\ntextured-quads: " << layer.textured_quads.size() << "\ncircles: " << layer.circles.size()
            << "\ntile-maps: " << layer.tile_maps.size() << "\nstatic: " << std::boolalpha << layer.is_static;
}

}  // namespace sde::graphics
