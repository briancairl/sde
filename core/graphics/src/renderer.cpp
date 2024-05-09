// C++ Standard Library
#include <ostream>

// Backend
#include "opengl.inl"

// SDE
#include "sde/graphics/renderer.hpp"
#include "sde/graphics/typedef.hpp"

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
  typename ElementT,
  std::size_t ElementCount,
  std::size_t InstanceDivisor = 0,
  VertexAccessMode AccessMode = VertexAccessMode::kDirect>
struct VertexAttribute
{
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
  const VertexAttribute<ElementT, ElementCount, InstanceDivisor, AccessMode>& attribute)
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

  return offset_bytes + total_bytes;
}

}  // namespace

std::ostream& operator<<(std::ostream& os, Renderer2DError error) { return os; }

class Renderer2D::Backend
{
public:
  Backend(const Renderer2DOptions& options)
  {
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);

    std::size_t offset = 0;
    offset = setAttribute<0>(offset, VertexAttribute<float, 2>{4UL * options.max_quad_count});
    offset = setAttribute<1>(offset, VertexAttribute<float, 2>{4UL * options.max_quad_count});

    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  ~Backend()
  {
    glDeleteBuffers(1, &vbo_);
    glDeleteVertexArrays(1, &vao_);
  }

  native_vertex_buffer_id_t vao_;
  native_vertex_buffer_id_t vbo_;
};

Renderer2D::~Renderer2D() = default;

Renderer2D::Renderer2D(std::unique_ptr<Backend> backend) : backend_{std::move(backend)} {}

expected<Renderer2D, Renderer2DError> Renderer2D::create(const Renderer2DOptions& options)
{
  return Renderer2D{std::make_unique<Backend>(options)};
}

std::ostream& operator<<(std::ostream& os, const Renderer2D& renderer) { return os; }

}  // namespace sde::graphics
