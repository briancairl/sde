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
#include "sde/graphics/text.hpp"
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
constexpr std::size_t kVerticesPerCircleOuter{16UL};
constexpr std::size_t kVerticesPerCircle{1UL + kVerticesPerCircleOuter};

constexpr std::size_t kVertexAttributeCount{4};
using PositionAttribute = VertexAttribute<0, float, 2>;
using TexCoordAttribute = VertexAttribute<1, float, 2>;
using TexUnitAttribute = VertexAttribute<2, float, 1>;
using TintColorAttribute = VertexAttribute<3, float, 4>;


std::array<Vec2f, kVerticesPerCircle> kUnitCircleLookup{[] {
  std::array<Vec2f, kVerticesPerCircle> lookup;
  constexpr float kAngleStep = static_cast<float>(2.0 * M_PI) / static_cast<float>(kVerticesPerCircleOuter - 1);

  auto* u_ptr = lookup.data();

  *(u_ptr++) = {0.0F, 0.0F};

  for (std::size_t v = 0; v < kVerticesPerCircleOuter; ++v, ++u_ptr)
  {
    const float angle = (static_cast<float>(v) * kAngleStep);
    *(u_ptr) = {std::cos(angle), std::sin(angle)};
  }

  return lookup;
}()};


Vec2f* fillQuadPositions(Vec2f* target, const Vec2f& min, const Vec2f& max)
{
  target[0] = min;
  target[1] = {min.x(), max.y()};
  target[2] = max;
  target[3] = {max.x(), min.y()};
  return target + kVerticesPerQuad;
}

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

template <typename ShapeT> constexpr std::size_t vertex_count_of(std::size_t shape_count);


template <> std::size_t vertex_count_of<Quad>(std::size_t shape_count) { return shape_count * kVerticesPerQuad; }

template <> std::size_t vertex_count_of<TexturedQuad>(std::size_t shape_count)
{
  return vertex_count_of<Quad>(shape_count);
}

template <> std::size_t vertex_count_of<Circle>(std::size_t shape_count) { return shape_count * kVerticesPerCircle; }

template <> std::size_t vertex_count_of<TileMap>(std::size_t shape_count)
{
  return vertex_count_of<Quad>(TileMap::kTileCount * shape_count);
}

template <typename ShapeT> constexpr std::size_t vertex_count_of(const View<const ShapeT>& shapes)
{
  return vertex_count_of<ShapeT>(shapes.size());
}


enum class ElementLayout
{
  kQuad,
  kCircle,
};


struct ElementLayoutBuffer
{
  ElementLayout type;
  std::size_t count;
  ElementLayoutBuffer(ElementLayout _type, std::size_t _count) : type{_type}, count{_count} {}
};


[[nodiscard]] std::tuple<GLuint*, std::size_t>
addElementsQuad(GLuint* elements, std::size_t vertex_count, std::size_t n)
{
  for (std::size_t i = 0; i < n; ++i, vertex_count += kVerticesPerQuad)
  {
    // Lower face
    elements[0] = vertex_count + 0;
    elements[1] = vertex_count + 1;
    elements[2] = vertex_count + 2;
    elements += kElementsPerTriangle;

    // Upper face
    elements[0] = vertex_count + 2;
    elements[1] = vertex_count + 3;
    elements[2] = vertex_count + 0;
    elements += kElementsPerTriangle;
  }
  return {elements, vertex_count};
}


[[nodiscard]] std::tuple<GLuint*, std::size_t>
addElementsCircle(GLuint* elements, std::size_t vertex_count, std::size_t n)
{
  for (std::size_t i = 0; i < n; ++i)
  {
    const GLuint center_vertex_index = vertex_count;
    for (std::size_t e = 1; e < kVerticesPerCircle - 1; ++e)
    {
      elements[0] = center_vertex_index;
      elements[1] = elements[0] + e;
      elements[2] = elements[1] + 1;
      elements += kElementsPerTriangle;
    }
    elements[2] = center_vertex_index + 1;
    vertex_count += kVerticesPerCircle;
  }
  return {elements, vertex_count};
}


struct VertexAttributeBuffer
{
  Vec2f* position = nullptr;
  Vec2f* texcoord = nullptr;
  float* texunit = nullptr;
  Vec4f* tint = nullptr;
};


class VertexArray
{
public:
  VertexArray(std::size_t max_vertex_count, bool is_static) : VertexArray{}
  {
    vertex_count_max_ = max_vertex_count;

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);

    // Allocate vertex buffer
    {
      glBindBuffer(GL_ARRAY_BUFFER, vbo_);
      std::size_t total_bytes = 0;
      vertex_attribute_byte_offsets_[PositionAttribute::kIndex] = total_bytes;
      total_bytes += setAttribute(total_bytes, PositionAttribute{max_vertex_count});
      vertex_attribute_byte_offsets_[TexCoordAttribute::kIndex] = total_bytes;
      total_bytes += setAttribute(total_bytes, TexCoordAttribute{max_vertex_count});
      vertex_attribute_byte_offsets_[TexUnitAttribute::kIndex] = total_bytes;
      total_bytes += setAttribute(total_bytes, TexUnitAttribute{max_vertex_count});
      vertex_attribute_byte_offsets_[TintColorAttribute::kIndex] = total_bytes;
      total_bytes += setAttribute(total_bytes, TintColorAttribute{max_vertex_count});
      glBufferData(GL_ARRAY_BUFFER, total_bytes, nullptr, is_static ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // Allocate element buffer
    {
      static constexpr std::size_t kBytesPerElement = sizeof(GLuint);
      const std::size_t total_bytes = vertex_count_max_ * kElementsPerTriangle * kBytesPerElement;
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, total_bytes, nullptr, is_static ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    glBindVertexArray(0);
  }

  VertexArray(VertexArray&& other) :
      vao_{other.vao_},
      vbo_{other.vbo_},
      ebo_{other.ebo_},
      vertex_buffer_mapped_{other.vertex_buffer_mapped_},
      vertex_count_{other.vertex_count_},
      vertex_attribute_byte_offsets_{other.vertex_attribute_byte_offsets_},
      element_layout_buffer_{std::move(other.element_layout_buffer_)}
  {
    other.vao_ = 0;
    other.vbo_ = 0;
    other.ebo_ = 0;
    other.vertex_buffer_mapped_ = nullptr;
  }

  ~VertexArray()
  {
    unmap_if_mapped();
    if (vao_ != 0)
    {
      glDeleteBuffers(1, &ebo_);
      glDeleteBuffers(1, &vbo_);
      glDeleteVertexArrays(1, &vao_);
    }
  }

  template <typename ShapeT> void size_after_add(std::size_t shape_count);

  template <typename ShapeT> void add(std::size_t shape_count);

  template <typename ShapeT> void add(const View<const ShapeT>& views) { add<ShapeT>(views.size()); }

  void reset()
  {
    element_layout_buffer_.clear();
    vertex_count_ = 0;
  }

  std::size_t size() const { return vertex_count_; }

  std::size_t capacity() const { return vertex_count_max_; }

  void map()
  {
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    vertex_buffer_mapped_ = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
  }

  void unmap()
  {
    glUnmapBuffer(GL_ARRAY_BUFFER);
    vertex_buffer_mapped_ = nullptr;
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  void unmap_if_mapped()
  {
    if (vertex_buffer_mapped_ == nullptr)
    {
      return;
    }
    unmap();
  }

  void draw()
  {
    digest();
    glDrawElements(GL_TRIANGLES, element_count_, GL_UNSIGNED_INT, 0);
  }

  template <typename R, typename VertexAttributeT, typename ByteT>
  [[nodiscard]] static R*
  getVertexAttributeBuffer(const std::array<std::size_t, kVertexAttributeCount>& byte_offsets, ByteT* mapped_buffer)
  {
    static_assert(sizeof(R) == VertexAttributeT::kBytesPerVertex);
    return reinterpret_cast<R*>(
      reinterpret_cast<std::uint8_t*>(mapped_buffer) + byte_offsets[VertexAttributeT::kIndex]);
  }

  VertexAttributeBuffer getVertexAttributeBuffer()
  {
    return {
      .position =
        getVertexAttributeBuffer<Vec2f, PositionAttribute>(vertex_attribute_byte_offsets_, vertex_buffer_mapped_) +
        vertex_count_,
      .texcoord =
        getVertexAttributeBuffer<Vec2f, TexCoordAttribute>(vertex_attribute_byte_offsets_, vertex_buffer_mapped_) +
        vertex_count_,
      .texunit =
        getVertexAttributeBuffer<float, TexUnitAttribute>(vertex_attribute_byte_offsets_, vertex_buffer_mapped_) +
        vertex_count_,
      .tint =
        getVertexAttributeBuffer<Vec4f, TintColorAttribute>(vertex_attribute_byte_offsets_, vertex_buffer_mapped_) +
        vertex_count_};
  }

private:
  VertexArray() = default;

  template <ElementLayout kLayout> void add_element_layout(std::size_t count)
  {
    if (element_layout_buffer_.empty() or (element_layout_buffer_.back().type != kLayout))
    {
      element_layout_buffer_.emplace_back(kLayout, count);
    }
    else
    {
      element_layout_buffer_.back().count += count;
    }
  }

  void digest()
  {
    if (element_layout_buffer_.empty())
    {
      return;
    }

    // Bind vertex element buffer to make it active
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);

    // Map element buffer and transfer all element data
    auto* elements_begin = reinterpret_cast<GLuint*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));
    auto* elements_end = elements_begin;
    std::size_t vertex_count = 0;
    for (const auto& [type, n] : element_layout_buffer_)
    {
      switch (type)
      {
      case ElementLayout::kQuad:
        std::tie(elements_end, vertex_count) = addElementsQuad(elements_end, vertex_count, n);
        break;
      case ElementLayout::kCircle:
        std::tie(elements_end, vertex_count) = addElementsCircle(elements_end, vertex_count, n);
        break;
      }
    }
    SDE_ASSERT_EQ(vertex_count, vertex_count_);

    // Count the number of added vertex elements
    element_count_ = static_cast<std::size_t>(std::distance(elements_begin, elements_end));

    // Un-map vertex element buffer to make it inactive
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

    // Remove elements to digest
    element_layout_buffer_.clear();
  }

  GLuint vao_ = 0;
  GLuint vbo_ = 0;
  GLuint ebo_ = 0;
  void* vertex_buffer_mapped_ = nullptr;
  std::size_t vertex_count_ = 0;
  std::size_t vertex_count_max_ = 0;
  std::size_t element_count_ = 0;
  std::array<std::size_t, kVertexAttributeCount> vertex_attribute_byte_offsets_;
  std::vector<ElementLayoutBuffer> element_layout_buffer_ = {};
};


template <> void VertexArray::add<Quad>(std::size_t shape_count)
{
  vertex_count_ += vertex_count_of<Quad>(shape_count);
  this->template add_element_layout<ElementLayout::kQuad>(shape_count);
}

template <> void VertexArray::add<TexturedQuad>(std::size_t shape_count)
{
  vertex_count_ += vertex_count_of<Quad>(shape_count);
  this->template add_element_layout<ElementLayout::kQuad>(shape_count);
}

template <> void VertexArray::add<Circle>(std::size_t shape_count)
{
  vertex_count_ += vertex_count_of<Circle>(shape_count);
  this->template add_element_layout<ElementLayout::kCircle>(shape_count);
}

template <> void VertexArray::add<TileMap>(std::size_t map_count)
{
  const auto quad_count = map_count * TileMap::kTileCount;
  vertex_count_ += quad_count * kVerticesPerQuad;
  this->template add_element_layout<ElementLayout::kQuad>(quad_count);
}

class OpenGLBackend : public RenderBackend
{
public:
  OpenGLBackend(const Renderer2DOptions& options) : va_{3UL * options.max_triangle_count_per_render_pass, false} {}

  ~OpenGLBackend() = default;

  void start()
  {
    va_.reset();
    va_.map();
  }

  void finish()
  {
    // Un-map vertex attribute buffer to make it inactive
    va_.unmap();

    // Draw elements
    va_.draw();
  }

  expected<void, RenderPassError> submit(View<const Quad> quads)
  {
    // Check that submission doesn't go over capacity
    if ((va_.size() + vertex_count_of(quads)) > va_.capacity())
    {
      return make_unexpected(RenderPassError::kMaxVertexCountExceeded);
    }

    // Add vertex attribute data
    auto buffers = va_.getVertexAttributeBuffer();
    for (const auto& q : quads)
    {
      static constexpr float kNoTextureUnitAssigned = -1.0F;

      // clang-format off
      buffers.position = fillQuadPositions(buffers.position, q.rect.min(), q.rect.max());
      buffers.texcoord = std::fill_n(buffers.texcoord, kVerticesPerQuad, Vec2f::Zero());
      buffers.texunit = std::fill_n(buffers.texunit, kVerticesPerQuad, kNoTextureUnitAssigned);
      buffers.tint = std::fill_n(buffers.tint, kVerticesPerQuad, q.color);
      // clang-format on
    }

    // Add vertex + element information
    va_.add(quads);
    return {};
  }

  expected<void, RenderPassError> submit(View<const TexturedQuad> textured_quads)
  {
    // Check that submission doesn't go over capacity
    if ((va_.size() + vertex_count_of(textured_quads)) > va_.capacity())
    {
      return make_unexpected(RenderPassError::kMaxVertexCountExceeded);
    }

    // Add vertex attribute data
    auto buffers = va_.getVertexAttributeBuffer();
    for (const auto& tq : textured_quads)
    {
      // clang-format off
      buffers.position = fillQuadPositions(buffers.position, tq.rect.min(), tq.rect.max());
      buffers.texcoord = fillQuadPositions(buffers.texcoord, tq.rect_texture.min(), tq.rect_texture.max());
      buffers.texunit = std::fill_n(buffers.texunit, kVerticesPerQuad, static_cast<float>(tq.texture_unit));
      buffers.tint = std::fill_n(buffers.tint, kVerticesPerQuad, tq.color);
      // clang-format on
    }

    // Add vertex + element information
    va_.add(textured_quads);
    return {};
  }

  expected<void, RenderPassError> submit(View<const Circle> circles)
  {
    // Check that submission doesn't go over capacity
    if ((va_.size() + vertex_count_of(circles)) > va_.capacity())
    {
      return make_unexpected(RenderPassError::kMaxVertexCountExceeded);
    }

    // Add vertex attribute data
    auto buffers = va_.getVertexAttributeBuffer();
    for (const auto& c : circles)
    {
      static constexpr float kNoTextureUnitAssigned = -1.0F;

      // clang-format off
      buffers.position = std::transform(std::begin(kUnitCircleLookup), std::end(kUnitCircleLookup), buffers.position,
                                        [&c](const Vec2f& unit) { return c.center + c.radius * unit; });
      buffers.texcoord = std::copy(std::begin(kUnitCircleLookup), std::end(kUnitCircleLookup), buffers.texcoord);
      buffers.texunit = std::fill_n(buffers.texunit, kVerticesPerCircle, kNoTextureUnitAssigned);
      buffers.tint = std::fill_n(buffers.tint, kVerticesPerCircle, c.color);
      // clang-format on
    }

    // Add vertex + element information
    va_.add(circles);
    return {};
  }

  expected<void, RenderPassError> submit(View<const TileMap>& tile_maps, const TileSet& tile_set)
  {
    // Check that submission doesn't go over capacity
    if ((va_.size() + vertex_count_of(tile_maps)) > va_.capacity())
    {
      return make_unexpected(RenderPassError::kMaxVertexCountExceeded);
    }

    // Add vertex attribute data
    auto b = va_.getVertexAttributeBuffer();
    for (const auto& tm : tile_maps)
    {
      // clang-format off
      for (int j = 0; j < tm.tiles.cols(); ++j)
      {
        for (int i = 0; i < tm.tiles.rows(); ++i)
        {
          const auto& tex_rect = tile_set[tm.tiles(i, j)];

          const Vec2f pos_rect_max = tm.position + Vec2f{tm.tile_size.x() * j, -tm.tile_size.y() * i};
          const Vec2f pos_rect_min = pos_rect_max + Vec2f{tm.tile_size.x(), -tm.tile_size.y()};

          // clang-format off
          b.position = fillQuadPositions(b.position, pos_rect_min,   pos_rect_max);
          b.texcoord = fillQuadPositions(b.texcoord, tex_rect.min(), tex_rect.max());
          b.texunit = std::fill_n(b.texunit, kVerticesPerQuad, static_cast<float>(tm.texture_unit));
          b.tint = std::fill_n(b.tint, kVerticesPerQuad, tm.color);
          // clang-format on
        }
      }
      // clang-format on
    }

    // Add vertex + element information
    va_.add(tile_maps);
    return {};
  }

  expected<void, RenderPassError> submit(const Text& text, const GlyphSet& glyphs)
  {
    // Check that submission doesn't go over capacity
    if ((va_.size() + vertex_count_of<Quad>(text.text.size())) > va_.capacity())
    {
      return make_unexpected(RenderPassError::kMaxVertexCountExceeded);
    }

    Vec2f text_pos = text.position;

    // Add vertex attribute data
    auto b = va_.getVertexAttributeBuffer();
    for (const char c : text.text)
    {
      const auto& glyph = glyphs[c];

      const Vec2f pos_rect_min =
        text_pos + Vec2f{glyph.bearing_px.x() * text.scale, (glyph.bearing_px.y() - glyph.size_px.y()) * text.scale};
      const Vec2f pos_rect_max = pos_rect_min + glyph.size_px * text.scale;

      // clang-format off
      b.position = fillQuadPositions(b.position, pos_rect_min, pos_rect_max);
      b.texcoord = fillQuadPositions(b.texcoord, glyph.tex_rect.min(), glyph.tex_rect.max());
      b.texunit = std::fill_n(b.texunit, kVerticesPerQuad, static_cast<float>(text.texture_unit));
      b.tint = std::fill_n(b.tint, kVerticesPerQuad, text.color);
      // clang-format on

      text_pos.x() += glyph.advance_px * text.scale;
    }

    // Add vertex + element information
    va_.add<Quad>(text.text.size());
    return {};
  }

private:
  VertexArray va_;
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

expected<void, RenderPassError> RenderPass::submit(View<const Quad> quads) { return backend__opengl->submit(quads); }

expected<void, RenderPassError> RenderPass::submit(View<const Circle> circles)
{
  return backend__opengl->submit(circles);
}

expected<void, RenderPassError> RenderPass::submit(View<const TexturedQuad> quads)
{
  return backend__opengl->submit(quads);
}

expected<void, RenderPassError> RenderPass::submit(View<const TileMap> tile_maps, const TileSet& tile_set)
{
  return backend__opengl->submit(tile_maps, tile_set);
}

expected<void, RenderPassError> RenderPass::submit(const Text& text, const GlyphSet& glyphs)
{
  return backend__opengl->submit(text, glyphs);
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

expected<RenderPass, RenderPassError> RenderPass::create(
  RenderTarget& target,
  Renderer2D& renderer,
  const RenderAttributes& attributes,
  const RenderResources& resources,
  const Vec4f& clear_color)
{
  if (backend__render_pass_active.test_and_set())
  {
    return make_unexpected(RenderPassError::kRenderPassActive);
  }

  target.refresh(clear_color);

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
