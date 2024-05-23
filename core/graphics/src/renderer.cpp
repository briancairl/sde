// C++ Standard Library
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
  glBindVertexArray(vao);
  glDrawElements(GL_TRIANGLES, element_count, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
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
  std::size_t length;

  explicit VertexAttribute(const std::size_t _length) : length{_length} {}
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

  const std::size_t total_bytes = kBytesPerVertex * attribute.length;

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

constexpr std::size_t kBufferCount = 2;
constexpr std::size_t kAttributeCount = 4;
using PositionAttribute = VertexAttribute<0, float, 2>;
using TexCoordAttribute = VertexAttribute<1, float, 2>;
using TintColorAttribute = VertexAttribute<2, float, 4>;
using TexUnitAttribute = VertexAttribute<3, float, 1>;

}  // namespace

std::ostream& operator<<(std::ostream& os, Renderer2DError error) { return os; }

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

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, getVertexElementsID());
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, total_bytes, nullptr, GL_DYNAMIC_DRAW);

      SDE_LOG_DEBUG_FMT("Allocated element mem: %lu bytes", total_bytes);
    }

    // Allocate attribute buffers
    {
      glBindBuffer(GL_ARRAY_BUFFER, getVertexAttributesID());

      std::size_t total_bytes = 0;

      vab_attribute_byte_offets_[PositionAttribute::kIndex] = total_bytes;
      total_bytes += setAttribute(total_bytes, PositionAttribute{3UL * options.max_triangle_count_per_layer});
      SDE_LOG_INFO_FMT("%lu", total_bytes);

      vab_attribute_byte_offets_[TexCoordAttribute::kIndex] = total_bytes;
      total_bytes += setAttribute(total_bytes, TexCoordAttribute{3UL * options.max_triangle_count_per_layer});

      vab_attribute_byte_offets_[TintColorAttribute::kIndex] = total_bytes;
      total_bytes += setAttribute(total_bytes, TintColorAttribute{3UL * options.max_triangle_count_per_layer});

      vab_attribute_byte_offets_[TexUnitAttribute::kIndex] = total_bytes;
      total_bytes += setAttribute(total_bytes, TexUnitAttribute{3UL * options.max_triangle_count_per_layer});

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

  void draw(const ShaderInfo& shader, const TextureCache& texture_cache, const Layer& layer)
  {
    static constexpr std::size_t kElementsPerTriangle = 3UL;
    static constexpr std::size_t kElementsPerQuad = 2 * kElementsPerTriangle;

    // Fill vertex attributes
    {
      auto* mapped_attr_buffer = map_write_only_vertex_buffer(getVertexAttributesID());
      auto* positions = getVertexAttributes<Vec2f, PositionAttribute>(mapped_attr_buffer);
      auto* colors = getVertexAttributes<Vec4f, TintColorAttribute>(mapped_attr_buffer);

      for (const auto& quads : layer.quads)
      {
        positions[0] = quads.min;
        positions[1] = {quads.min.x(), quads.max.y()};
        positions[2] = quads.max;
        positions[3] = {quads.max.x(), quads.min.y()};
        positions += Quad::kVertexCount;

        std::fill(colors, colors + Quad::kVertexCount, quads.color);
        colors += Quad::kVertexCount;
      }

      unmap_vertex_buffer();
    }

    // Fill vertex elements
    std::size_t element_count = 0;
    {
      auto* mapped_element_buffer = map_write_only_element_buffer(getVertexElementsID());
      auto* elements = getVertexElements(mapped_element_buffer);
      for (std::size_t quad_index = 0; quad_index < layer.quads.size();
           ++quad_index, elements += kElementsPerQuad, element_count += kElementsPerQuad)
      {
        // Lower face
        elements[0] = (quad_index * kElementsPerQuad) + 0;
        elements[1] = (quad_index * kElementsPerQuad) + 1;
        elements[2] = (quad_index * kElementsPerQuad) + 2;

        // Upper face
        elements[3] = (quad_index * kElementsPerQuad) + 2;
        elements[4] = (quad_index * kElementsPerQuad) + 3;
        elements[5] = (quad_index * kElementsPerQuad) + 0;
      }
      unmap_element_buffer();
    }

    glUseProgram(shader.native_id);
    draw_triangle_elements(vao_, element_count);
  }

  GLuint getVertexAttributesID() const { return vab_[0]; }

  GLuint getVertexElementsID() const { return vab_[1]; }

  template <typename R, typename VertexAttributeT> [[nodiscard]] R* getVertexAttributes(void* mapped_buffer)
  {
    static_assert(sizeof(R) == VertexAttributeT::kBytesPerVertex);
    return reinterpret_cast<R*>(
      reinterpret_cast<std::uint8_t*>(mapped_buffer) + vab_attribute_byte_offets_[VertexAttributeT::kIndex]);
  }

  [[nodiscard]] GLuint* getVertexElements(void* mapped_buffer) { return reinterpret_cast<GLuint*>(mapped_buffer); }

  GLuint vao_;
  GLuint vab_[kBufferCount];
  std::size_t vab_attribute_byte_offets_[kAttributeCount];
};

void Renderer2D::Layer::reset()
{
  quads.clear();
  textured_quads.clear();
}

bool Renderer2D::Layer::drawable() const { return shader.is_valid() and !(quads.empty() and textured_quads.empty()); }

Renderer2D::~Renderer2D() = default;

Renderer2D::Renderer2D(std::unique_ptr<Backend> backend, std::vector<Layer> layers) :
    layers_{std::move(layers)}, backend_{std::move(backend)}
{}


void Renderer2D::set(ShaderHandle shader)
{
  for (auto& layer : layers_)
  {
    layer.shader = shader;
  }
}

void Renderer2D::set(std::size_t layer, ShaderHandle shader)
{
  SDE_ASSERT_LT(layer, layers_.size());
  layers_[layer].shader = shader;
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

void Renderer2D::update(const ShaderCache& shader_cache, const TextureCache& texture_cache)
{
  for (auto& layer : layers_)
  {
    if (layer.drawable())
    {
      SDE_ASSERT_TRUE(layer.shader.is_valid());
      const auto* shader_info = shader_cache.get(layer.shader);
      SDE_ASSERT_NE(shader_info, nullptr);
      SDE_ASSERT_TRUE(hasLayout(*shader_info, "vPosition", ShaderVariableType::kVec2, PositionAttribute::kIndex));
      SDE_ASSERT_TRUE(hasLayout(*shader_info, "vTexCoord", ShaderVariableType::kVec2, TexCoordAttribute::kIndex));
      SDE_ASSERT_TRUE(hasLayout(*shader_info, "vTintColor", ShaderVariableType::kVec4, TintColorAttribute::kIndex));
      SDE_ASSERT_TRUE(hasLayout(*shader_info, "vTexUnit", ShaderVariableType::kFloat, TexUnitAttribute::kIndex));
      backend_->draw(*shader_info, texture_cache, layer);
    }
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
  return Renderer2D{std::make_unique<Backend>(options), std::move(layers)};
}

std::ostream& operator<<(std::ostream& os, const Renderer2D& renderer) { return os; }

}  // namespace sde::graphics
