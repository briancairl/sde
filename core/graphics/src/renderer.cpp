// C++ Standard Library
#include <ostream>
#include <vector>

// Backend
#include "opengl.inl"

// SDE
#include "sde/graphics/renderer.hpp"
#include "sde/graphics/typedef.hpp"
#include "sde/logging.hpp"

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
  static constexpr std::size_t kIndex = Index;

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
  std::size_t& offset_bytes,
  const VertexAttribute<Index, ElementT, ElementCount, InstanceDivisor, AccessMode>& attribute)
{
  static constexpr std::uint8_t* kOffsetStart = nullptr;
  static constexpr std::size_t kBytesPerVertex = ElementCount * sizeof(ElementT);

  const std::size_t total_bytes = kBytesPerVertex * attribute.length;

  glEnableVertexAttribArray(Index);

  glVertexAttribPointer(
    Index,  // layout index
    ElementCount,  // elementcount
    opengl::to_native_typecode(typecode<ElementT>()),  // typecode
    opengl::to_native_bool(AccessMode == VertexAccessMode::kNormalized),  // normalized
    kBytesPerVertex,  // stride
    static_cast<const GLvoid*>(kOffsetStart + offset_bytes)  // offset in buffer
  );

  glVertexAttribDivisor(Index, InstanceDivisor);

  auto attribute_offset = offset_bytes;
  offset_bytes += total_bytes;
  return attribute_offset;
}

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
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elements());
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, total_bytes, nullptr, GL_DYNAMIC_DRAW);
      SDE_LOG_DEBUG_FMT("Allocated element mem: %lu bytes", total_bytes);
    }

    // Allocate attribute buffers
    {
      glBindBuffer(GL_ARRAY_BUFFER, attributes());
      std::size_t total_bytes = 0;
      vab_attribute_byte_offets_[0] =
        setAttribute(total_bytes, PositionAttribute{3UL * options.max_triangle_count_per_layer});
      vab_attribute_byte_offets_[1] =
        setAttribute(total_bytes, TexCoordAttribute{3UL * options.max_triangle_count_per_layer});
      vab_attribute_byte_offets_[2] =
        setAttribute(total_bytes, TintColorAttribute{3UL * options.max_triangle_count_per_layer});
      vab_attribute_byte_offets_[3] =
        setAttribute(total_bytes, TexUnitAttribute{3UL * options.max_triangle_count_per_layer});
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

  void draw(const Layer& layer) {}

  native_vertex_buffer_id_t attributes() const { return vab_[0]; }
  native_vertex_buffer_id_t elements() const { return vab_[1]; }

  native_vertex_buffer_id_t vao_;
  native_vertex_buffer_id_t vab_[2];
  std::size_t vab_attribute_byte_offets_[kAttributeCount];
};

void Renderer2D::Layer::reset()
{
  quads.clear();
  textured_quads.clear();
}

Renderer2D::~Renderer2D() = default;

Renderer2D::Renderer2D(std::unique_ptr<Backend> backend, std::vector<Layer> layers) :
    layers_{std::move(layers)}, backend_{std::move(backend)}
{}

void Renderer2D::submit(std::size_t layer, const Quad& quad) { layers_[layer].quads.push_back(quad); }

void Renderer2D::submit(std::size_t layer, const TexturedQuad& quad) { layers_[layer].textured_quads.push_back(quad); }

void Renderer2D::update()
{
  for (const auto& layer : layers_)
  {
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
  return Renderer2D{std::make_unique<Backend>(options), std::move(layers)};
}

std::ostream& operator<<(std::ostream& os, const Renderer2D& renderer) { return os; }

}  // namespace sde::graphics
