// C++ Standard Library
#include <algorithm>
#include <iomanip>
#include <iterator>
#include <ostream>

// Backend
#include "opengl.inl"

// SDE
#include "sde/graphics/image.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/logging.hpp"
#include "sde/resource.hpp"

namespace sde::graphics
{
namespace
{

struct TextureDeleter
{
  void operator()(native_texture_id_t id)
  {
    if (id != 0)
    {
      glDeleteTextures(1, &id);
    }
  }
};

constexpr native_texture_id_t kInvalidTextureID = 0;

using NativeTextureID = UniqueResource<native_texture_id_t, TextureDeleter>;

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

// TextureLayout from_native_layout_enum(const enum_t count)
// {
//   switch (count)
//   {
//   case GL_RED:
//     return TextureLayout::kR;
//   case GL_RG:
//     return TextureLayout::kRG;
//   case GL_RGB:
//     return TextureLayout::kRGB;
//   case GL_RGBA:
//     return TextureLayout::kRGBA;
//   }
//   return TextureLayout::kR;
// }

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

NativeTextureID allocate_texture_2D_and_bind(
  const TextureShape& shape,
  const TextureLayout layout,
  const TextureOptions& options,
  const TypeCode type)
{
  NativeTextureID texture_id{[] {
    native_texture_id_t id;
    glGenTextures(1, &id);
    return id;
  }()};

  glBindTexture(GL_TEXTURE_2D, texture_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, to_native_wrapping_mode_enum(options.u_wrapping));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, to_native_wrapping_mode_enum(options.v_wrapping));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, to_native_sampling_mode_enum(options.min_sampling));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, to_native_sampling_mode_enum(options.mag_sampling));

  if (options.flags.unpack_alignment)
  {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  }

  static constexpr GLint kDefaultLevelOfDetail = 0;
  static constexpr GLint kDefaultBorder = 0;
  glTexImage2D(
    GL_TEXTURE_2D,
    kDefaultLevelOfDetail,
    to_native_layout_enum(layout),
    shape.value.x(),
    shape.value.y(),
    kDefaultBorder,
    to_native_layout_enum(layout),
    to_native_typecode(type),
    nullptr);

  return texture_id;
}

expected<void, TextureError> upload_texture_2D(
  const void* const data,
  const TextureShape& shape,
  const TextureLayout layout,
  const TextureOptions& options,
  const TypeCode type)
{
  static constexpr GLint kDefaultLevelOfDetail = 0;
  static constexpr GLint kDefaultXOffset = 0;
  static constexpr GLint kDefaultYOffset = 0;

  glTexSubImage2D(
    GL_TEXTURE_2D,
    kDefaultLevelOfDetail,
    kDefaultXOffset,
    kDefaultYOffset,
    shape.value.x(),
    shape.value.y(),
    to_native_layout_enum(layout),
    to_native_typecode(type),
    reinterpret_cast<const void*>(data));

  if (has_active_error())
  {
    return make_unexpected(TextureError::kBackendTransferFailure);
  }

  glGenerateMipmap(GL_TEXTURE_2D);
  if (has_active_error())
  {
    return make_unexpected(TextureError::kBackendMipMapGenerationFailure);
  }

  return {};
}

expected<NativeTextureID, TextureError> create_native_texture_2D(
  const void* const data,
  const TextureShape& shape,
  const TextureLayout layout,
  const TextureOptions& options,
  const TypeCode type)
{
  auto texture_id = allocate_texture_2D_and_bind(shape, layout, options, type);

  if (has_active_error())
  {
    return make_unexpected(TextureError::kBackendCreationFailure);
  }

  if (const auto ok_or_error = upload_texture_2D(data, shape, layout, options, type); !ok_or_error.has_value())
  {
    return make_unexpected(ok_or_error.error());
  }
  return texture_id;
}

expected<NativeTextureID, TextureError> create_native_texture_2D(
  const TextureShape& shape,
  const TextureLayout layout,
  const TextureOptions& options,
  const TypeCode type)
{
  auto texture_id = allocate_texture_2D_and_bind(shape, layout, options, type);

  if (has_active_error())
  {
    return make_unexpected(TextureError::kBackendCreationFailure);
  }

  return texture_id;
}

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

TextureLayout layout_from_channel_count(std::size_t channel_count)
{
  switch (channel_count)
  {
  case 1:
    return TextureLayout::kR;
  case 2:
    return TextureLayout::kRG;
  case 3:
    return TextureLayout::kRGB;
  case 4:
    return TextureLayout::kRGBA;
  }
  return TextureLayout::kR;
}

template <typename T>
expected<TextureInfo, TextureError>
create_texture_impl(View<T> data, const TextureShape& shape, TextureLayout layout, const TextureOptions& options)
{
  if (!data)
  {
    return make_unexpected(TextureError::kInvalidDataValue);
  }
  else if (shape.height() == 0 or shape.width() == 0)
  {
    return make_unexpected(TextureError::kInvalidDimensions);
  }

  const std::size_t required_size = (shape.height() * shape.width() * to_channel_count(layout));
  const std::size_t actual_size = sizeof(T) * data.size();
  if (actual_size != required_size)
  {
    SDE_LOG_FATAL_FMT("Expected texture to have data len %lu but has %lu", required_size, actual_size);
    return make_unexpected(TextureError::kInvalidDataLength);
  }

  auto texture_or_error =
    create_native_texture_2D(reinterpret_cast<const void*>(data.data()), shape, layout, options, typecode<T>());
  if (!texture_or_error.has_value())
  {
    return make_unexpected(texture_or_error.error());
  }

  return TextureInfo{
    .layout = layout, .shape = shape, .options = options, .native_id = texture_or_error->exchange(kInvalidTextureID)};
}

template <typename T>
expected<TextureInfo, TextureError>
create_texture_impl(const TextureShape& shape, TextureLayout layout, const TextureOptions& options)
{
  auto texture_or_error = create_native_texture_2D(shape, layout, options, typecode<T>());

  if (!texture_or_error.has_value())
  {
    return make_unexpected(texture_or_error.error());
  }

  return TextureInfo{
    .layout = layout, .shape = shape, .options = options, .native_id = texture_or_error->exchange(kInvalidTextureID)};
}


void delete_texture_impl(const TextureInfo& texture_info)
{
  if (texture_info.native_id != 0)
  {
    glDeleteTextures(1, &texture_info.native_id);
  }
}

}  // namespace

std::ostream& operator<<(std::ostream& os, const TextureShape& shape)
{
  return os << "{ height: " << shape.value.y() << ", width: " << shape.value.x() << " }";
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

std::ostream& operator<<(std::ostream& os, TextureLayout layout)
{
  switch (layout)
  {
  case TextureLayout::kR:
    return os << "R";
  case TextureLayout::kRG:
    return os << "RG";
  case TextureLayout::kRGB:
    return os << "RGB";
  case TextureLayout::kRGBA:
    return os << "RGBA";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, TextureError error)
{
  switch (error)
  {
  case TextureError::kInvalidHandle:
    return os << "InvalidHandle";
  case TextureError::kInvalidDimensions:
    return os << "InvalidDimensions";
  case TextureError::kInvalidDataValue:
    return os << "InvalidDataValue";
  case TextureError::kInvalidDataLength:
    return os << "InvalidDataLength";
  case TextureError::kBackendCreationFailure:
    return os << "BackendCreationFailure";
  case TextureError::kBackendTransferFailure:
    return os << "BackendTransferFailure";
  case TextureError::kBackendMipMapGenerationFailure:
    return os << "BackendMipMapGenerationFailure";
  }
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


TextureCache::~TextureCache()
{
  std::for_each(std::begin(textures_), std::end(textures_), [](const auto& handle_and_texture) {
    delete_texture_impl(handle_and_texture.second);
  });
  textures_.clear();
}

bool TextureCache::remove(const TextureHandle& index)
{
  if (const auto& texture_itr = textures_.find(index); texture_itr != std::end(textures_))
  {
    delete_texture_impl(texture_itr->second);
    textures_.erase(texture_itr);
    return true;
  }
  return false;
}

template <typename T>
expected<void, TextureError> TextureCache::transfer(
  TextureHandle texture,
  View<const T> data,
  const TextureShape& shape,
  TextureLayout layout,
  const TextureOptions& options)
{
  auto texture_info_or_error = create_texture_impl(data, shape, layout, options);
  if (texture_info_or_error.has_value())
  {
    textures_.emplace(texture, std::move(*texture_info_or_error));
    return expected<void, TextureError>{};
  }
  return make_unexpected(texture_info_or_error.error());
}

template expected<void, TextureError> TextureCache::transfer(
  TextureHandle texture,
  View<const std::uint8_t> data,
  const TextureShape& shape,
  TextureLayout layout,
  const TextureOptions& options);

template expected<void, TextureError> TextureCache::transfer(
  TextureHandle texture,
  View<const std::uint16_t> data,
  const TextureShape& shape,
  TextureLayout layout,
  const TextureOptions& options);

template expected<void, TextureError> TextureCache::transfer(
  TextureHandle texture,
  View<const std::uint32_t> data,
  const TextureShape& shape,
  TextureLayout layout,
  const TextureOptions& options);

template expected<void, TextureError> TextureCache::transfer(
  TextureHandle texture,
  View<const float> data,
  const TextureShape& shape,
  TextureLayout layout,
  const TextureOptions& options);


expected<void, TextureError>
TextureCache::transfer(TextureHandle texture, const Image& image, const TextureOptions& options)
{
  return TextureCache::transfer(
    texture,
    make_view(reinterpret_cast<const std::uint8_t*>(image.data()), image.total_size_in_bytes()),
    TextureShape{image.shape().value},
    layout_from_channel_count(image.channel_count()),
    options);
}

template <typename T>
expected<void, TextureError> TextureCache::allocate(
  TextureHandle texture,
  const TextureShape& shape,
  TextureLayout layout,
  const TextureOptions& options)
{
  auto texture_info_or_error = create_texture_impl<T>(shape, layout, options);
  if (texture_info_or_error.has_value())
  {
    textures_.emplace(texture, std::move(*texture_info_or_error));
    return expected<void, TextureError>{};
  }
  return make_unexpected(texture_info_or_error.error());
}

template expected<void, TextureError> TextureCache::allocate<std::uint8_t>(
  TextureHandle texture,
  const TextureShape& shape,
  TextureLayout layout,
  const TextureOptions& options);

template expected<void, TextureError> TextureCache::allocate<std::uint16_t>(
  TextureHandle texture,
  const TextureShape& shape,
  TextureLayout layout,
  const TextureOptions& options);

template expected<void, TextureError> TextureCache::allocate<std::uint32_t>(
  TextureHandle texture,
  const TextureShape& shape,
  TextureLayout layout,
  const TextureOptions& options);

template expected<void, TextureError> TextureCache::allocate<float>(
  TextureHandle texture,
  const TextureShape& shape,
  TextureLayout layout,
  const TextureOptions& options);

const TextureInfo* TextureCache::get(TextureHandle texture) const
{
  if (auto itr = textures_.find(texture); itr != std::end(textures_))
  {
    return std::addressof(itr->second);
  }
  return nullptr;
}

}  // namespace sde::graphics
