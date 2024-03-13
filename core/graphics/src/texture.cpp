// C++ Standard Library
#include <iomanip>
#include <ostream>

// Backend
#include "opengl.inl"

// SDE
#include "sde/graphics/texture.hpp"

namespace sde::graphics
{
namespace opengl
{

enum_t to_native_layout_enum(const TextureLayout channels)
{
  switch (channels)
  {
  case TextureLayout::kR:
    return GL_RED;
  case TextureLayout::kRG:
    return GL_RG;
  case TextureLayout::kRGB:
    return GL_RGB;
  case TextureLayout::kRGBA:
    return GL_RGBA;
  }
  return GL_RED;
}

TextureLayout from_native_layout_enum(const enum_t count)
{
  switch (count)
  {
  case GL_RED:
    return TextureLayout::kR;
  case GL_RG:
    return TextureLayout::kRG;
  case GL_RGB:
    return TextureLayout::kRGB;
  case GL_RGBA:
    return TextureLayout::kRGBA;
  }
  return TextureLayout::kR;
}

enum_t to_native_wrapping_mode_enum(const TextureWrapping mode)
{
  switch (mode)
  {
  case TextureWrapping::kClampToBorder:
    return GL_CLAMP_TO_BORDER;
  case TextureWrapping::kRepeat:
    return GL_REPEAT;
  default:
    break;
  }
  return GL_CLAMP_TO_BORDER;
}

enum_t to_native_sampling_mode_enum(const TextureSampling mode)
{
  switch (mode)
  {
  case TextureSampling::kLinear:
    return GL_LINEAR;
  case TextureSampling::kNearest:
    return GL_NEAREST;
  default:
    break;
  }
  return GL_NEAREST;
}

expected<native_texture_id_t, TextureError> create_empty_texture_2D_and_bind(const TextureOptions& options)
{
  native_texture_id_t texture_id;
  glGenTextures(1, &texture_id);
  glBindTexture(GL_TEXTURE_2D, texture_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, to_native_wrapping_mode_enum(options.u_wrapping));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, to_native_wrapping_mode_enum(options.v_wrapping));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, to_native_sampling_mode_enum(options.min_sampling));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, to_native_sampling_mode_enum(options.mag_sampling));

  if (options.flags.unpack_alignment)
  {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  }

  if (has_active_error())
  {
    return make_unexpected(TextureError::kBackendCreationFailure);
  }

  return texture_id;
}

expected<void, TextureError> generate_texture_mipmap()
{
  glGenerateMipmap(GL_TEXTURE_2D);

  if (has_active_error())
  {
    return make_unexpected(TextureError::kBackendMipMapGenerationFailure);
  }
  return {};
}

expected<void, TextureError> upload_texture_2D(
  const void* const data,
  const TextureShape& shape,
  const TextureLayout layout,
  const TextureOptions& options,
  const TypeCode type)
{
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    to_native_layout_enum(layout),
    shape.height,
    shape.width,
    0,
    to_native_layout_enum(layout),
    to_native_typecode(type),
    reinterpret_cast<const void*>(data));

  if (has_active_error())
  {
    return make_unexpected(TextureError::kBackendTransferFailure);
  }
  else if (options.flags.generate_mip_map)
  {
    return generate_texture_mipmap();
  }

  return {};
}

expected<native_texture_id_t, TextureError> create_texture_2D(
  const void* const data,
  const TextureShape& shape,
  const TextureLayout layout,
  const TextureOptions& options,
  const TypeCode type)
{
  if (const auto id_or_error = create_empty_texture_2D_and_bind(options); !id_or_error.has_value())
  {
    return make_unexpected(id_or_error.error());
  }
  else if (const auto ok_or_error = upload_texture_2D(data, shape, layout, options, type); !ok_or_error.has_value())
  {
    glDeleteTextures(1, &(*id_or_error));
    return make_unexpected(ok_or_error.error());
  }
  else
  {
    return std::move(id_or_error).value();
  }
}

}  // namespace opengl

namespace
{

std::size_t to_channel_count(const TextureLayout channels)
{
  switch (channels)
  {
  case TextureLayout::kR:
    return 1;
  case TextureLayout::kRG:
    return 2;
  case TextureLayout::kRGB:
    return 3;
  case TextureLayout::kRGBA:
    return 4;
  }
  return 0;
}

template <typename T>
expected<TextureInfo, TextureError> create_texture_impl(
  ContinuousView<T> data,
  const TextureShape& shape,
  TextureLayout layout,
  const TextureOptions& options)
{
  if (!data)
  {
    return make_unexpected(TextureError::kInvalidDataValue);
  }
  else if (shape.height == 0 or shape.width == 0)
  {
    return make_unexpected(TextureError::kInvalidDimensions);
  }
  else if (const std::size_t required_size = (shape.height * shape.width * to_channel_count(layout));
           data.size() != required_size)
  {
    return make_unexpected(TextureError::kInvalidDataLength);
  }
  else if (const auto texture_or_error = opengl::create_texture_2D(
             reinterpret_cast<const void*>(data.data()), shape, layout, options, opengl::to_native_typecode<T>());
           !texture_or_error.has_value())
  {
    return make_unexpected(texture_or_error.error());
  }
  else
  {
    return TextureInfo{.layout = layout, .shape = shape, .options = options, .native_id = *texture_or_error};
  }
}

}  // namespace

std::ostream& operator<<(std::ostream& os, TextureHandle handle) { return os << "{ id: " << handle.id << " }"; }

std::ostream& operator<<(std::ostream& os, const TextureShape& shape)
{
  return os << "{ height: " << shape.height << ", width: " << shape.height << " }";
}

std::ostream& operator<<(std::ostream& os, const TextureInfo& info)
{
  return os << "{ shape: " << info.shape << ", layout: " << info.layout << ", options: " << info.options
            << ", native_id: " << info.native_id << " }";
}

std::ostream& operator<<(std::ostream& os, TextureWrapping wrapping)
{
  switch (wrapping)
  {
  case TextureWrapping::kClampToBorder:
    return os << "ClampToBorder";
  case TextureWrapping::kRepeat:
    return os << "Repeat";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, TextureSampling sampling)
{
  switch (sampling)
  {
  case TextureSampling::kLinear:
    return os << "Linear";
  case TextureSampling::kNearest:
    return os << "Nearest";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, TextureFlags flags)
{
  return os << std::boolalpha << "{ unpack_alignment: " << static_cast<bool>(flags.unpack_alignment)
            << ", generate_mip_map: " << static_cast<bool>(flags.generate_mip_map) << " }";
}

std::ostream& operator<<(std::ostream& os, const TextureOptions& options)
{
  return os << "{ u_wrapping: " << options.u_wrapping << ", v_wrapping: " << options.v_wrapping
            << ", min_sampling: " << options.min_sampling << ", mag_sampling: " << options.mag_sampling
            << ", flags: " << options.flags << " }";
}

expected<void, TextureError>
TextureCache::create(TextureHandle texture, const Image& image, const TextureOptions& options)
{}

expected<void, TextureError> TextureCache::create(
  TextureHandle texture,
  ContinuousView<std::uint8_t> data,
  const TextureShape& shape,
  TextureLayout layout,
  const TextureOptions& options)
{
  if (auto texture_info_or_error = create_texture_impl(data, shape, layout, options);
      !texture_info_or_error.has_value())
  {
    return make_unexpected(texture_info_or_error.error());
  }
}

}  // namespace sde::graphics
