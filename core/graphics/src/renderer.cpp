// C++ Standard Library
#include <array>
#include <cmath>
#include <mutex>
#include <ostream>
#include <type_traits>
#include <vector>

// Backend
#include "opengl.inl"

// SDE
#include "sde/graphics/renderer.hpp"
#include "sde/graphics/shader.hpp"
#include "sde/graphics/texture.hpp"
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

}  // namespace

std::ostream& operator<<(std::ostream& os, Renderer2DError error) { return os; }

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

class Renderer2D::Backend
{
public:
  Backend(const Renderer2DOptions& options)
  {
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

  void draw(const Layer& layer)
  {
    glBindVertexArray(vao_);

    // Fill vertex attributes
    const std::size_t vertex_count = [this, &layer] {
      auto buffers = getVertexAttributeBuffers();
      addQuadAttributes(buffers, layer);
      addTexturedQuadAttributes(buffers, layer);
      addCircleAttributes(buffers, layer);
      return buffers.vertex_count;
    }();

    // Fill vertex elements
    const std::size_t element_count = [this, &layer, vertex_count] {
      auto buffers = getVertexElementBuffer();
      addQuadElements(buffers, layer.quads.size());
      addQuadElements(buffers, layer.textured_quads.size());
      addCircleElements(buffers, layer.circles.size());
      SDE_ASSERT_EQ(vertex_count, buffers.vertex_count);
      return buffers.element_count;
    }();

    draw_triangle_elements(vao_, element_count);

    glBindVertexArray(0);
  }

private:
  void addQuadElements(ElementBuffer& buffer, std::size_t quad_count)
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

  void addQuadAttributes(AttributeBuffers& buffers, const Layer& layer)
  {
    for (const auto& q : layer.quads)
    {
      fillQuadPositions(buffers.position + buffers.vertex_count, q.rect.min, q.rect.max);
      std::fill_n(buffers.texcoord + buffers.vertex_count, kVerticesPerQuad, Vec2f::Zero());
      std::fill_n(buffers.texunit + buffers.vertex_count, kVerticesPerQuad, -1);
      std::fill_n(buffers.tint + buffers.vertex_count, kVerticesPerQuad, q.color);
      buffers.vertex_count += kVerticesPerQuad;
    }
  }

  void addTexturedQuadAttributes(AttributeBuffers& buffers, const Layer& layer)
  {
    for (const auto& q : layer.textured_quads)
    {
      fillQuadPositions(buffers.position + buffers.vertex_count, q.rect.min, q.rect.max);
      fillQuadPositions(buffers.texcoord + buffers.vertex_count, q.texrect.min, q.texrect.max);
      std::fill_n(buffers.texunit + buffers.vertex_count, kVerticesPerQuad, static_cast<float>(q.texture_unit));
      std::fill_n(buffers.tint + buffers.vertex_count, kVerticesPerQuad, q.color);
      buffers.vertex_count += kVerticesPerQuad;
    }
  }

  void addCircleElements(ElementBuffer& buffer, std::size_t circle_count)
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

  void addCircleAttributes(AttributeBuffers& buffers, const Layer& layer)
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

    for (const auto& c : layer.circles)
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
    }
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

void Renderer2D::Layer::reset()
{
  quads.clear();
  textured_quads.clear();
  circles.clear();
}

bool Renderer2D::Layer::drawable() const
{
  return settings.shader.is_valid() and !(quads.empty() and textured_quads.empty() and circles.empty());
}

Renderer2D::~Renderer2D() = default;

Renderer2D::Renderer2D(std::unique_ptr<Backend> backend, std::vector<Layer> layers) :
    layers_{std::move(layers)}, backend_{std::move(backend)}
{
  GLint texture_units;
  glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
  SDE_ASSERT_GT(texture_units, LayerSettings::kTextureUnits);
}


LayerSettings& Renderer2D::layer(std::size_t layer)
{
  SDE_ASSERT_LT(layer, layers_.size());
  return layers_[layer].settings;
}

void Renderer2D::submit(std::size_t layer, const Quad& quad)
{
  SDE_ASSERT_LT(layer, layers_.size());
  layers_[layer].quads.push_back(quad);
}

void Renderer2D::submit(std::size_t layer, const TexturedQuad& quad)
{
  SDE_ASSERT_LT(layer, layers_.size());
  layers_[layer].textured_quads.push_back(quad);
}

void Renderer2D::submit(std::size_t layer, const Circle& circle)
{
  SDE_ASSERT_LT(layer, layers_.size());
  layers_[layer].circles.push_back(circle);
}

void Renderer2D::update(const ShaderCache& shader_cache)
{
  native_shader_id_t active_shader_id = 0;
  for (auto& layer : layers_)
  {
    if (!layer.drawable())
    {
      continue;
    }

    // Get shader from cacher
    SDE_ASSERT_TRUE(layer.settings.shader.is_valid());
    const auto* shader_info = shader_cache.get(layer.settings.shader);
    SDE_ASSERT_NE(shader_info, nullptr);

    // Set active shader
    if (active_shader_id != shader_info->native_id)
    {
      // clang-format off
      SDE_ASSERT_TRUE(hasLayout(*shader_info, "vPosition",  ShaderVariableType::kVec2,  PositionAttribute::kIndex ));
      SDE_ASSERT_TRUE(hasLayout(*shader_info, "vTexCoord",  ShaderVariableType::kVec2,  TexCoordAttribute::kIndex ));
      SDE_ASSERT_TRUE(hasLayout(*shader_info, "vTexUnit",   ShaderVariableType::kFloat, TexUnitAttribute::kIndex  ));
      SDE_ASSERT_TRUE(hasLayout(*shader_info, "vTintColor", ShaderVariableType::kVec4,  TintColorAttribute::kIndex));
      // clang-format on

      glUseProgram(shader_info->native_id);
      SDE_LOG_INFO_FMT("%u | %u", active_shader_id, shader_info->native_id);
      active_shader_id = shader_info->native_id;
    }

    // Set active texture units
    for (std::size_t unit = 0; unit < layer.settings.textures.size(); ++unit)
    {
      if (const native_shader_id_t texture_id = layer.settings.textures[unit]; texture_id > 0)
      {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glUniform1i(glGetUniformLocation(active_shader_id, "fTextureID"), unit);
      }
    }

    // Draw
    backend_->draw(layer);
  }

  for (auto& layer : layers_)
  {
    layer.reset();
  }
}

expected<Renderer2D, Renderer2DError> Renderer2D::create(const Renderer2DOptions& options)
{
  std::vector<Layer> layers;
  layers.resize(options.max_layers);
  for (auto& l : layers)
  {
    l.settings.textures.fill(0);
  }
  return Renderer2D{std::make_unique<Backend>(options), std::move(layers)};
}

std::ostream& operator<<(std::ostream& os, const Renderer2D& renderer) { return os; }

}  // namespace sde::graphics
