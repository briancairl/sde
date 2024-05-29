#ifdef SDE_GRAPHICS_GL_INL
#error("Detected double include of gl.inl")
#else
#define SDE_GRAPHICS_GL_INL
#endif  // SDE_GRAPHICS_GL_INL

// clang-format off

// GLAD
#include "glad/glad.h"

// GL
#include "GL/gl.h"

// clang-format on

// C++ Standard Library
#include <cstdint>
#include <type_traits>

// SDE
#include "sde/graphics/typecode.hpp"
#include "sde/graphics/typedef.hpp"

namespace sde::graphics
{

static inline GLenum to_native_typecode(TypeCode code)
{
  switch (code)
  {
  case TypeCode::kSInt8:
    return GL_BYTE;
  case TypeCode::kUInt8:
    return GL_UNSIGNED_BYTE;
  case TypeCode::kSInt16:
    return GL_SHORT;
  case TypeCode::kUInt16:
    return GL_UNSIGNED_SHORT;
  case TypeCode::kFloat32:
    return GL_FLOAT;
  case TypeCode::kFloat64:
    return GL_DOUBLE;
  case TypeCode::kSInt32:
    return GL_INT;
  case TypeCode::kUInt32:
    return GL_UNSIGNED_INT;
  }
  return GL_FLOAT;
}

template<typename T>
static inline GLenum to_native_typecode()
{
  return to_native_typecode(typecode<T>());
}

static inline TypeCode from_native_typecode(const GLenum code)
{
  switch (code)
  {
  case GL_BYTE:
    return TypeCode::kSInt8;
  case GL_UNSIGNED_BYTE:
    return TypeCode::kUInt8;
  case GL_SHORT:
    return TypeCode::kSInt16;
  case GL_UNSIGNED_SHORT:
    return TypeCode::kUInt16;
  case GL_FLOAT:
    return TypeCode::kFloat32;
  case GL_DOUBLE:
    return TypeCode::kFloat64;
  case GL_INT:
    return TypeCode::kSInt32;
  case GL_UNSIGNED_INT:
    return TypeCode::kUInt32;
  default:
    break;
  }
  return TypeCode::kSInt8;
}

constexpr GLboolean to_native_bool(const bool value) { return value ? GL_TRUE : GL_FALSE; }

constexpr bool from_bool(const GLboolean value) { return value == GL_TRUE; }

static_assert(std::is_same<GLint, int>());

static_assert(std::is_same<GLenum, enum_t>());

static_assert(std::is_same<GLuint, native_shader_id_t>());

static_assert(std::is_same<GLuint, native_texture_id_t>());

static_assert(std::is_same<GLuint, native_vertex_buffer_id_t>());

static_assert(sizeof(std::uint8_t) == byte_count<TypeCode::kSInt8>());
static_assert(sizeof(std::uint8_t) == byte_count<TypeCode::kUInt8>());
static_assert(sizeof(std::uint16_t) == byte_count<TypeCode::kSInt16>());
static_assert(sizeof(std::uint16_t) == byte_count<TypeCode::kUInt16>());
static_assert(sizeof(float) == byte_count<TypeCode::kFloat32>());
static_assert(sizeof(double) == byte_count<TypeCode::kFloat64>());
static_assert(sizeof(int) == byte_count<TypeCode::kSInt32>());
static_assert(sizeof(unsigned) == byte_count<TypeCode::kUInt32>());

inline GLenum has_active_error()
{
  GLenum last_err{GL_NO_ERROR};
  while (true)
  {
    if (const auto next_err = glGetError(); next_err != GL_NO_ERROR)
    {
      last_err = next_err;
    }
    else
    {
      break;
    }
  }
  return last_err;
}

}  // sde::graphics
