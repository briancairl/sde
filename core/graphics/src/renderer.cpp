// C++ Standard Library
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <optional>
#include <ostream>
#include <type_traits>

// Backend
#include "opengl.inl"

// SDE
#include "sde/build.hpp"
#include "sde/format.hpp"
#include "sde/geometry.hpp"
#include "sde/geometry_utils.hpp"
#include "sde/graphics/assets.hpp"
#include "sde/graphics/render_buffer.hpp"
#include "sde/graphics/render_target.hpp"
#include "sde/graphics/renderer.hpp"
#include "sde/graphics/shader.hpp"
#include "sde/graphics/shapes.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/graphics/tile_map.hpp"
#include "sde/graphics/tile_set.hpp"
#include "sde/graphics/typedef.hpp"
#include "sde/logging.hpp"
#include "sde/time_ostream.hpp"
#include "sde/vector.hpp"

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
  typename ValueT = ElementT,
  std::size_t InstanceDivisor = 0,
  VertexAccessMode AccessMode = VertexAccessMode::kDirect>
struct VertexAttribute
{
  static_assert(std::is_same<ElementT, ValueT>() or (sizeof(ValueT) == ElementCount * sizeof(ElementT)));

  static constexpr std::size_t kElementCount{ElementCount};
  static constexpr std::size_t kBytesPerVertex{ElementCount * sizeof(ElementT)};

  using ValueType = ValueT;

  static void setup(std::size_t layout_index, std::size_t offset_bytes)
  {
    static constexpr std::uint8_t* kOffsetStart{nullptr};

    glEnableVertexAttribArray(layout_index);

    glVertexAttribPointer(
      layout_index,  // layout index
      kElementCount,  // elementcount
      to_native_typecode(typecode<ElementT>()),  // typecode
      to_native_bool(AccessMode == VertexAccessMode::kNormalized),  // normalized
      kBytesPerVertex,  // stride
      static_cast<const GLvoid*>(kOffsetStart + offset_bytes)  // offset in buffer
    );

    glVertexAttribDivisor(layout_index, InstanceDivisor);
  }
};

constexpr std::size_t kElementsPerTriangle{3UL};
constexpr std::size_t kVerticesPerQuad{4UL};
constexpr std::size_t kVerticesPerCircleOuter{16UL};
constexpr std::size_t kVerticesPerCircle{1UL + kVerticesPerCircleOuter};


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
  target[0] = max;
  target[1] = {max.x(), min.y()};
  target[2] = min;
  target[3] = {min.x(), max.y()};
  return target + kVerticesPerQuad;
}

Vec2f* fillQuadPositionsT(Vec2f* target, const Vec2f& min, const Vec2f& max)
{
  target[0] = {max.x(), min.y()};
  target[1] = max;
  target[2] = {min.x(), max.y()};
  target[3] = min;
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

template <typename ShapeT> constexpr std::size_t vertex_count_of(const View<const ShapeT>& shapes)
{
  return vertex_count_of<ShapeT>(shapes.size());
}


enum class ElementLayout
{
  kQuad,
  kCircle,
};


template <typename ShapeT> ElementLayout element_layout_of();

template <> ElementLayout element_layout_of<Quad>() { return ElementLayout::kQuad; }

template <> ElementLayout element_layout_of<TexturedQuad>() { return ElementLayout::kQuad; }

template <> ElementLayout element_layout_of<Circle>() { return ElementLayout::kCircle; }


struct ElementLayoutBuffer
{
  ElementLayout type;
  std::size_t count;
  ElementLayoutBuffer(ElementLayout _type, std::size_t _count) : type{_type}, count{_count} {}
};


[[nodiscard]] std::tuple<GLuint*, std::size_t>
addTriangleElementsQuad(GLuint* elements, std::size_t vertex_count, std::size_t n)
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
addTriangleElementsCircle(GLuint* elements, std::size_t vertex_count, std::size_t n)
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


[[nodiscard]] std::tuple<GLuint*, std::size_t>
addLineElementsQuad(GLuint* elements, std::size_t vertex_count, std::size_t n)
{
  for (std::size_t i = 0; i < n; ++i, vertex_count += kVerticesPerQuad, elements += 8)
  {
    elements[0] = vertex_count + 0;
    elements[1] = vertex_count + 1;

    elements[2] = vertex_count + 1;
    elements[3] = vertex_count + 2;

    elements[4] = vertex_count + 2;
    elements[5] = vertex_count + 3;

    elements[6] = vertex_count + 3;
    elements[7] = vertex_count + 0;
  }
  return {elements, vertex_count};
}

[[nodiscard]] std::tuple<GLuint*, std::size_t>
addLineElementsCircle(GLuint* elements, std::size_t vertex_count, std::size_t n)
{
  for (std::size_t i = 0; i < n; ++i, vertex_count += kVerticesPerCircle)
  {
    for (std::size_t j = 1; j < kVerticesPerCircle; ++j)
    {
      elements[0] = j - 1;
      elements[1] = j;
      elements += 2;
    }
    {
      elements[0] = kVerticesPerCircle - 1;
      elements[1] = 0;
      elements += 2;
    }
  }
  return {elements, vertex_count};
}

GLenum toGLDrawMode(VertexDrawMode mode)
{
  switch (mode)
  {
  case VertexDrawMode::kFilled:
    return GL_TRIANGLES;
  case VertexDrawMode::kWireFrame:
    return GL_LINES;
  }
  return GL_TRIANGLES;
}


GLenum toGLBufferMode(VertexBufferMode mode)
{
  switch (mode)
  {
  case VertexBufferMode::kStatic:
    return GL_STATIC_DRAW;
  case VertexBufferMode::kDynamic:
    return GL_DYNAMIC_DRAW;
  }
  return GL_DYNAMIC_DRAW;
}


template <typename... Attributes> class VertexArray
{
public:
  static constexpr std::size_t kVertexAttributeCount{sizeof...(Attributes)};

  VertexArray(std::size_t max_vertex_count, VertexBufferMode buffer_mode, VertexDrawMode draw_mode) :
      VertexArray{max_vertex_count, draw_mode}
  {
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    glGenBuffers(1, &vbo_);

    // Allocate vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    const auto total_bytes = std::apply(
      [&](auto... attrs) -> std::size_t {
        std::size_t total_bytes_accum = 0;
        std::size_t layout_index_accum = 0;
        auto fn = [&](auto attr) {
          using AttrType = bare_t<decltype(attr)>;
          AttrType::setup(layout_index_accum, total_bytes_accum);
          vertex_attribute_byte_offsets_[layout_index_accum] = total_bytes_accum;
          total_bytes_accum += AttrType::kBytesPerVertex * max_vertex_count;
          ++layout_index_accum;
          return 1;
        };
        [[maybe_unused]] const auto _ = (fn(attrs) + ...);
        return total_bytes_accum;
      },
      std::tuple<Attributes...>{});
    glBufferData(GL_ARRAY_BUFFER, total_bytes, nullptr, toGLBufferMode(buffer_mode));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  VertexArray(VertexArray&& other) :
      vao_{other.vao_},
      vbo_{other.vbo_},
      vertex_buffer_mapped_{other.vertex_buffer_mapped_},
      vertex_count_{other.vertex_count_},
      vertex_attribute_byte_offsets_{other.vertex_attribute_byte_offsets_}
  {
    other.vao_ = 0;
    other.vbo_ = 0;
    other.vertex_buffer_mapped_ = nullptr;
  }

  ~VertexArray()
  {
    if (vertex_buffer_mapped_ != nullptr)
    {
      unmap();
      vertex_buffer_mapped_ = nullptr;
    }
    if (vao_ != 0)
    {
      glDeleteBuffers(1, &vbo_);
      glDeleteVertexArrays(1, &vao_);
    }
  }

  template <typename ShapeT> void add(std::size_t shape_count)
  {
    vertex_count_ += vertex_count_of<ShapeT>(shape_count);
  }

  template <typename ShapeT> void add(const View<const ShapeT>& views) { add<ShapeT>(views.size()); }

  void reset() { vertex_count_ = 0; }

  std::size_t capacity() const { return vertex_count_max_; }

  constexpr std::size_t vertex_count() const { return vertex_count_; }

  VertexDrawMode draw_mode() const { return vertex_draw_mode_; }

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

  void draw() { glDrawArrays(toGLDrawMode(draw_mode()), 0, vertex_count_); }

  auto attributes()
  {
    auto offset_itr = std::begin(vertex_attribute_byte_offsets_);
    const auto get_offset = [&offset_itr]() -> std::size_t {
      auto curr_itr = offset_itr;
      ++offset_itr;
      return *curr_itr;
    };
    return std::make_tuple((mapped_attribute<Attributes>(get_offset(), vertex_buffer_mapped_) + vertex_count_)...);
  }

private:
  explicit VertexArray(std::size_t vertex_count_max, VertexDrawMode draw_mode) :
      vertex_count_max_{vertex_count_max}, vertex_draw_mode_{draw_mode}
  {}

  template <typename VertexAttributeT, typename ByteT>
  [[nodiscard]] static auto* mapped_attribute(const std::size_t offset, ByteT* mapped_buffer)
  {
    using R = typename VertexAttributeT::ValueType;
    static_assert(sizeof(R) == VertexAttributeT::kBytesPerVertex);
    return reinterpret_cast<R*>(reinterpret_cast<std::uint8_t*>(mapped_buffer) + offset);
  }

  GLuint vao_ = 0;
  GLuint vbo_ = 0;
  void* vertex_buffer_mapped_ = nullptr;
  std::size_t vertex_count_ = 0;
  std::size_t vertex_count_max_ = 0;
  VertexDrawMode vertex_draw_mode_ = VertexDrawMode::kFilled;
  std::array<std::size_t, kVertexAttributeCount> vertex_attribute_byte_offsets_;
};


template <typename... Attributes> class ElementVertexArray : public VertexArray<Attributes...>
{
public:
  using Base = VertexArray<Attributes...>;
  static constexpr std::size_t kVertexAttributeCount{sizeof...(Attributes)};

  ElementVertexArray(std::size_t max_vertex_count, VertexBufferMode buffer_mode, VertexDrawMode draw_mode) :
      Base{max_vertex_count, buffer_mode, draw_mode}
  {
    glGenBuffers(1, &ebo_);

    static constexpr std::size_t kBytesPerElement = sizeof(GLuint);
    const std::size_t total_bytes = max_vertex_count * kElementsPerTriangle * kBytesPerElement;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, total_bytes, nullptr, toGLBufferMode(buffer_mode));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  ElementVertexArray(ElementVertexArray&& other) :
      Base{std::move(static_cast<Base&>(other))},
      ebo_{other.ebo_},
      element_layout_buffer_{std::move(other.element_layout_buffer_)}
  {
    other.ebo_ = 0;
  }

  ~ElementVertexArray()
  {
    if (ebo_ != 0)
    {
      glDeleteBuffers(1, &ebo_);
    }
  }

  template <typename ShapeT> void add(std::size_t shape_count)
  {
    Base::template add<ShapeT>(shape_count);
    this->template add_element_layout<ShapeT>(shape_count);
  }

  template <typename ShapeT> void add(const View<const ShapeT>& views) { this->add<ShapeT>(views.size()); }

  void draw()
  {
    flush();
    glDrawElements(toGLDrawMode(this->draw_mode()), element_count_, GL_UNSIGNED_INT, 0);
  }

  constexpr std::size_t element_count() const { return element_count_; }

private:
  using Base::add;
  using Base::draw;

  template <typename ShapeT> void add_element_layout(std::size_t count)
  {
    if (element_layout_buffer_.empty() or (element_layout_buffer_.back().type != element_layout_of<ShapeT>()))
    {
      element_layout_buffer_.emplace_back(element_layout_of<ShapeT>(), count);
    }
    else
    {
      element_layout_buffer_.back().count += count;
    }
  }

  void flush()
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
    switch (this->draw_mode())
    {
    case VertexDrawMode::kFilled: {
      for (const auto& [type, n] : element_layout_buffer_)
      {
        switch (type)
        {
        case ElementLayout::kQuad:
          std::tie(elements_end, vertex_count) = addTriangleElementsQuad(elements_end, vertex_count, n);
          break;
        case ElementLayout::kCircle:
          std::tie(elements_end, vertex_count) = addTriangleElementsCircle(elements_end, vertex_count, n);
          break;
        }
      }
      break;
    }
    case VertexDrawMode::kWireFrame: {
      for (const auto& [type, n] : element_layout_buffer_)
      {
        switch (type)
        {
        case ElementLayout::kQuad:
          std::tie(elements_end, vertex_count) = addLineElementsQuad(elements_end, vertex_count, n);
          break;
        case ElementLayout::kCircle:
          std::tie(elements_end, vertex_count) = addLineElementsCircle(elements_end, vertex_count, n);
          break;
        }
      }
      break;
    }
    }
    SDE_ASSERT_EQ(vertex_count, Base::vertex_count());

    // Count the number of added vertex elements
    element_count_ = static_cast<std::size_t>(std::distance(elements_begin, elements_end));

    // Un-map vertex element buffer to make it inactive
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

    // Remove elements to flush
    element_layout_buffer_.clear();
  }

  GLuint ebo_ = 0;
  std::size_t element_count_ = 0;
  sde::vector<ElementLayoutBuffer> element_layout_buffer_ = {};
};

using BatchVertexArray = ElementVertexArray<
  VertexAttribute<float, 2, Vec2f>,  // position
  VertexAttribute<float, 2, Vec2f>,  // tex-coord
  VertexAttribute<float, 1, float>,  // tex-unit
  VertexAttribute<float, 4, Vec4f>  // tint color
  >;

class OpenGLBackend : public RenderBackend
{
public:
  OpenGLBackend(const Renderer2DOptions& options) : va_active_{nullptr}
  {
    va_.reserve(options.buffers.size());
    for (const auto& options : options.buffers)
    {
      va_.emplace_back(
        kElementsPerTriangle * options.max_triangle_count_per_render_pass, options.buffer_mode, options.draw_mode);
    }
  }

  ~OpenGLBackend() = default;

  void start(std::size_t active_buffer_index)
  {
    SDE_ASSERT_LT(active_buffer_index, va_.size());
    va_active_ = (va_.data() + active_buffer_index);
    va_active_->reset();
    va_active_->map();
  }

  void finish(RenderStats& stats)
  {
    // Un-map vertex attribute buffer to make it inactive
    va_active_->unmap();
    // Draw elements
    va_active_->draw();

    // Keep statistics
    stats.max_vertex_count = std::max(stats.max_vertex_count, va_active_->vertex_count());
    stats.max_element_count = std::max(stats.max_element_count, va_active_->element_count());
  }

  expected<void, RenderPassError> submit(View<const Quad> quads)
  {
    // Check that submission doesn't go over capacity
    if ((va_active_->vertex_count() + vertex_count_of(quads)) > va_active_->capacity())
    {
      return make_unexpected(RenderPassError::kMaxVertexCountExceeded);
    }

    // Add vertex attribute data
    auto [position, texcoord, texunit, tint] = va_active_->attributes();
    for (const auto& q : quads)
    {
      static constexpr float kNoTextureUnitAssigned = -1.0F;

      // clang-format off
      position = fillQuadPositions(position, q.rect.pt0, q.rect.pt1);
      texcoord = std::fill_n(texcoord, kVerticesPerQuad, Vec2f::Zero());
      texunit = std::fill_n(texunit, kVerticesPerQuad, kNoTextureUnitAssigned);
      tint = std::fill_n(tint, kVerticesPerQuad, q.color);
      // clang-format on
    }

    // Add vertex + element information
    va_active_->add(quads);
    return {};
  }

  expected<void, RenderPassError> submit(View<const TexturedQuad> textured_quads)
  {
    // Check that submission doesn't go over capacity
    if ((va_active_->vertex_count() + vertex_count_of(textured_quads)) > va_active_->capacity())
    {
      return make_unexpected(RenderPassError::kMaxVertexCountExceeded);
    }

    // Add vertex attribute data
    auto [position, texcoord, texunit, tint] = va_active_->attributes();
    for (const auto& tq : textured_quads)
    {
      // clang-format off
      position = fillQuadPositions(position, tq.rect.pt0, tq.rect.pt1);
      texcoord = fillQuadPositionsT(texcoord, tq.rect_texture.pt0, tq.rect_texture.pt1);
      texunit = std::fill_n(texunit, kVerticesPerQuad, static_cast<float>(tq.texture_unit));
      tint = std::fill_n(tint, kVerticesPerQuad, tq.color);
      // clang-format on
    }

    // Add vertex + element information
    va_active_->add(textured_quads);
    return {};
  }

  expected<void, RenderPassError> submit(View<const Circle> circles)
  {
    // Check that submission doesn't go over capacity
    if ((va_active_->vertex_count() + vertex_count_of(circles)) > va_active_->capacity())
    {
      return make_unexpected(RenderPassError::kMaxVertexCountExceeded);
    }

    // Add vertex attribute data
    auto [position, texcoord, texunit, tint] = va_active_->attributes();
    for (const auto& c : circles)
    {
      static constexpr float kNoTextureUnitAssigned = -1.0F;

      // clang-format off
      position = std::transform(std::begin(kUnitCircleLookup), std::end(kUnitCircleLookup), position,
                                        [&c](const Vec2f& unit) { return c.center + c.radius * unit; });
      texcoord = std::copy(std::begin(kUnitCircleLookup), std::end(kUnitCircleLookup), texcoord);
      texunit = std::fill_n(texunit, kVerticesPerCircle, kNoTextureUnitAssigned);
      tint = std::fill_n(tint, kVerticesPerCircle, c.color);
      // clang-format on
    }

    // Add vertex + element information
    va_active_->add(circles);
    return {};
  }

private:
  BatchVertexArray* va_active_ = nullptr;
  sde::vector<BatchVertexArray> va_;
};

std::optional<OpenGLBackend> backend__opengl;
std::atomic_flag backend__render_pass_active;

float toAspectRatio(const Vec2i viewport_size)
{
  return static_cast<float>(viewport_size.x()) / static_cast<float>(viewport_size.y());
}

}  // namespace

Mat3f RenderUniforms::getWorldFromViewportMatrix(const Vec2i& viewport_size) const
{
  return world_from_camera * toInverseCameraMatrix(scaling, toAspectRatio(viewport_size));
}

expected<Renderer2D, RendererError> Renderer2D::create(const Renderer2DOptions& options)
{
  if (backend__opengl.has_value())
  {
    return make_unexpected(RendererError::kRendererPreviouslyInitialized);
  }

  // Initialize rendering backend
  backend__opengl.emplace(options);

  Renderer2D renderer;
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
    last_active_resources_{std::move(other.last_active_resources_)},
    next_active_resources_{std::move(other.next_active_resources_)},
    last_active_textures_{std::move(other.last_active_textures_)},
    next_active_textures_{std::move(other.next_active_textures_)},
    backend_{other.backend_}
{
  other.backend_ = nullptr;
}


std::optional<std::size_t> Renderer2D::assign(const TextureHandle& texture)
{
  std::size_t first_inactive_unit = TextureUnits::kAvailable;
  for (std::size_t unit = 0; unit < TextureUnits::kAvailable; ++unit)
  {
    // Texture is already assigned to a unit
    if (next_active_textures_[unit] == texture)
    {
      return unit;
    }
    // Find the first unassigned unit
    else if (next_active_textures_[unit].isNull() and (first_inactive_unit == TextureUnits::kAvailable))
    {
      first_inactive_unit = unit;
    }
  }
  // Assign texture to the first unassigned unit
  if (first_inactive_unit != TextureUnits::kAvailable)
  {
    assign(first_inactive_unit, texture);
    return first_inactive_unit;
  }
  // All unit in use
  return std::nullopt;
}

void Renderer2D::refresh(const RenderResources& resources)
{
  SDE_ASSERT_TRUE(resources.isValid());
  last_active_resources_ = next_active_resources_;
  next_active_resources_ = resources;
  backend__opengl->start(next_active_resources_.buffer_group);
}

void Renderer2D::flush(const Assets& assets, const RenderUniforms& uniforms, const Mat3f& viewport_from_world)
{
  const auto shader = assets.shaders(next_active_resources_.shader);
  SDE_ASSERT_TRUE(shader);

  // Set active shader
  if (next_active_resources_.shader != last_active_resources_.shader)
  {
    glUseProgram(shader->native_id);
    // Make sure all texture units are set ???
    // last_active_texture_units_.reset();
  }

  // Set active texture units
  for (std::size_t u = 0; u < TextureUnits::kAvailable; ++u)
  {
    if (next_active_textures_[u] and next_active_textures_[u] != last_active_textures_[u])
    {
      const auto texture = assets.textures(next_active_textures_[u]);
      SDE_ASSERT_TRUE(texture);

      glActiveTexture(GL_TEXTURE0 + u);
      glBindTexture(GL_TEXTURE_2D, texture->native_id);
      glUniform1i(glGetUniformLocation(shader->native_id, format("uTexture[%lu]", u)), u);
    }
  }

  // clang-format off
  // Apply other variables
  glUniform1f(glGetUniformLocation(shader->native_id, "uTime"), toSeconds(uniforms.time));
  glUniform1f(glGetUniformLocation(shader->native_id, "uTimeDelta"), toSeconds(uniforms.time_delta));
  glUniformMatrix3fv(glGetUniformLocation(shader->native_id, "uCameraTransform"), 1, GL_FALSE, viewport_from_world.data());
  // clang-format on

  backend__opengl->finish(stats_);
}

expected<void, RenderPassError> RenderPass::submit(View<const Quad> quads) { return backend__opengl->submit(quads); }

expected<void, RenderPassError> RenderPass::submit(View<const Circle> circles)
{
  return backend__opengl->submit(circles);
}

expected<void, RenderPassError> RenderPass::submit(View<const TexturedQuad> quads)
{
  return backend__opengl->submit(quads);
}

RenderPass::RenderPass(RenderPass&& other) :
    renderer_{other.renderer_},
    buffer_{other.buffer_},
    assets_{other.assets_},
    uniforms_{other.uniforms_},
    world_from_viewport_{other.world_from_viewport_},
    viewport_from_world_{other.viewport_from_world_},
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

  submit(make_const_view(buffer_->circles));
  submit(make_const_view(buffer_->quads));
  submit(make_const_view(buffer_->textured_quads));
  buffer_->reset();

  renderer_->flush(*assets_, *uniforms_, viewport_from_world_);
  backend__render_pass_active.clear();
}

void RenderPass::clear(const Vec4f& color)
{
  glClearColor(color[0], color[1], color[2], color[3]);
  glClear(GL_COLOR_BUFFER_BIT);
}

bool RenderPass::retarget(Vec2i& viewport_size, RenderTargetHandle render_target, const Assets& assets)
{
  // Pick render target
  const auto* render_target_info = assets.render_targets.get_if(render_target);
  if (render_target_info == nullptr)
  {
    return false;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, render_target_info->native_id);

  // Render target is the screen
  if (render_target_info->native_id.isNull())
  {
    return true;
  }

  // If render target is an off-screen target, update viewport size
  const auto* color_attachment_texture = assets.textures.get_if(render_target_info->color_attachment);
  if (color_attachment_texture == nullptr)
  {
    return false;
  }
  viewport_size.x() = std::max(viewport_size.x(), color_attachment_texture->shape.value.x());
  viewport_size.y() = std::max(viewport_size.y(), color_attachment_texture->shape.value.y());

  return true;
}

expected<RenderPass, RenderPassError> RenderPass::create(
  RenderBuffer& buffer,
  Renderer2D& renderer,
  const Assets& assets,
  const RenderUniforms& uniforms,
  const RenderResources& resources,
  Vec2i viewport_size)
{
  if (backend__render_pass_active.test_and_set())
  {
    return make_unexpected(RenderPassError::kRenderPassActive);
  }

  if (!retarget(viewport_size, resources.target, assets))
  {
    return make_unexpected(RenderPassError::kInvalidRenderTarget);
  }

  glViewport(0, 0, viewport_size.x(), viewport_size.y());

  renderer.refresh(resources);

  RenderPass render_pass;
  render_pass.renderer_ = std::addressof(renderer);
  render_pass.buffer_ = std::addressof(buffer);
  render_pass.assets_ = std::addressof(assets);
  render_pass.uniforms_ = std::addressof(uniforms);
  render_pass.world_from_viewport_ = uniforms.getWorldFromViewportMatrix(viewport_size);
  render_pass.viewport_from_world_ = render_pass.world_from_viewport_.inverse();
  render_pass.viewport_in_world_bounds_ =
    transform(render_pass.world_from_viewport_, Bounds2f{-Vec2f::Ones(), Vec2f::Ones()});

  return render_pass;
}

}  // namespace sde::graphics
