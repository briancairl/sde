// C++ Standard Library
#include <algorithm>
#include <iomanip>
#include <iterator>
#include <ostream>

// Backend
#include "opengl.inl"

// SDE
#include "sde/geometry_utils.hpp"
#include "sde/graphics/image.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/logging.hpp"
#include "sde/resource_wrapper.hpp"

namespace sde::graphics
{
namespace
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

enum_t to_native_wrapping_mode_enum(const TextureWrapping mode)
{
  switch (mode)
  {
  case TextureWrapping::kClampToBorder:
    return GL_CLAMP_TO_BORDER;
  case TextureWrapping::kClampToEdge:
    return GL_CLAMP_TO_EDGE;
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

  if (options.unpack_alignment)
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
  const TextureLayout layout,
  const TypeCode type,
  const Vec2i& offset,
  const Vec2i& shape)
{
  static constexpr GLint kDefaultLevelOfDetail = 0;

  glTexSubImage2D(
    GL_TEXTURE_2D,
    kDefaultLevelOfDetail,
    offset.x(),
    offset.y(),
    shape.x(),
    shape.y(),
    to_native_layout_enum(layout),
    to_native_typecode(type),
    reinterpret_cast<const void*>(data));

  if (has_active_error())
  {
    SDE_LOG_DEBUG_FMT(
      "BackendTransferFailure: [offset_x=%d, offset_y=%d, shape_x=%d, shape_y=%d, format=%s, type=%s]",
      offset.x(),
      offset.y(),
      shape.x(),
      shape.y(),
      static_cast<int>(to_native_layout_enum(layout)),
      static_cast<int>(to_native_typecode(type)));
    return make_unexpected(TextureError::kBackendTransferFailure);
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

  if (const auto gl_error = has_active_error())
  {
    SDE_LOG_ERROR() << "BackendCreationFailure: GL_ERROR=" << gl_error;
    return make_unexpected(TextureError::kBackendCreationFailure);
  }

  if (const auto ok_or_error = upload_texture_2D(data, layout, type, Vec2i::Zero(), shape.value);
      !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << ok_or_error.error();
    return make_unexpected(ok_or_error.error());
  }

  if (options.generate_mip_map)
  {
    glGenerateMipmap(GL_TEXTURE_2D);
    if (const auto gl_error = has_active_error())
    {
      SDE_LOG_ERROR() << "BackendMipMapGenerationFailure: GL_ERROR=" << gl_error;
      return make_unexpected(TextureError::kBackendMipMapGenerationFailure);
    }
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

std::size_t size_in_bytes(const Vec2i& extents, TextureLayout layout)
{
  return (static_cast<std::size_t>(extents.prod()) * to_channel_count(layout));
}

expected<NativeTextureID, TextureError>
create_texture_impl(TypeCode type, const TextureShape& shape, TextureLayout layout, const TextureOptions& options)
{
  auto texture_id = allocate_texture_2D_and_bind(shape, layout, options, type);

  if (const auto gl_error = has_active_error())
  {
    SDE_LOG_ERROR() << "BackendCreationFailure: GL_ERROR=" << gl_error;
    return make_unexpected(TextureError::kBackendCreationFailure);
  }

  return texture_id;
}

template <typename DataT>
expected<NativeTextureID, TextureError>
create_texture_impl(View<DataT> data, const TextureShape& shape, TextureLayout layout, const TextureOptions& options)
{
  if (!data)
  {
    SDE_LOG_ERROR() << "InvalidDataValue";
    return make_unexpected(TextureError::kInvalidDataValue);
  }
  else if (shape.height() == 0 or shape.width() == 0)
  {
    SDE_LOG_ERROR() << "InvalidDimensions: " << SDE_OSNV(shape.height()) << ", " << SDE_OSNV(shape.width());
    return make_unexpected(TextureError::kInvalidDimensions);
  }

  const std::size_t required_size = size_in_bytes(shape.value, layout);
  const std::size_t actual_size = sizeof(DataT) * data.size();
  if (actual_size != required_size)
  {
    SDE_LOG_DEBUG_FMT("Expected texture to have data len %lu but has %lu", required_size, actual_size);
    return make_unexpected(TextureError::kInvalidDataLength);
  }

  auto texture_or_error =
    create_native_texture_2D(reinterpret_cast<const void*>(data.data()), shape, layout, options, typecode<DataT>());
  if (!texture_or_error.has_value())
  {
    return make_unexpected(texture_or_error.error());
  }

  return std::move(texture_or_error).value();
}

}  // namespace

void TextureNativeDeleter::operator()(native_texture_id_t id) const
{
  SDE_LOG_DEBUG_FMT("glDeleteTextures(1, &%u)", id);
  glDeleteTextures(1, &id);
}

std::ostream& operator<<(std::ostream& os, TextureWrapping wrapping)
{
  switch (wrapping)
  {
    SDE_OS_ENUM_CASE(TextureWrapping::kClampToBorder)
    SDE_OS_ENUM_CASE(TextureWrapping::kClampToEdge)
    SDE_OS_ENUM_CASE(TextureWrapping::kRepeat)
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, TextureSampling sampling)
{
  switch (sampling)
  {
    SDE_OS_ENUM_CASE(TextureSampling::kLinear)
    SDE_OS_ENUM_CASE(TextureSampling::kNearest)
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, TextureLayout layout)
{
  switch (layout)
  {
    SDE_OS_ENUM_CASE(TextureLayout::kR)
    SDE_OS_ENUM_CASE(TextureLayout::kRG)
    SDE_OS_ENUM_CASE(TextureLayout::kRGB)
    SDE_OS_ENUM_CASE(TextureLayout::kRGBA)
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, TextureError error)
{
  switch (error)
  {
    SDE_OS_ENUM_CASE(TextureError::kTextureNotFound)
    SDE_OS_ENUM_CASE(TextureError::kElementAlreadyExists)
    SDE_OS_ENUM_CASE(TextureError::kInvalidHandle)
    SDE_OS_ENUM_CASE(TextureError::kInvalidSourceImage)
    SDE_OS_ENUM_CASE(TextureError::kInvalidDimensions)
    SDE_OS_ENUM_CASE(TextureError::kInvalidDataValue)
    SDE_OS_ENUM_CASE(TextureError::kInvalidDataLength)
    SDE_OS_ENUM_CASE(TextureError::kBackendCreationFailure)
    SDE_OS_ENUM_CASE(TextureError::kBackendTransferFailure)
    SDE_OS_ENUM_CASE(TextureError::kBackendMipMapGenerationFailure)
    SDE_OS_ENUM_CASE(TextureError::kReplaceAreaEmpty)
    SDE_OS_ENUM_CASE(TextureError::kReplaceAreaOutOfBounds)
  }
}

template <typename DataT>
expected<void, TextureError> replace(const Texture& texture, View<const DataT> data, const Bounds2i& area)
{
  if (area.isEmpty())
  {
    SDE_LOG_ERROR() << "ReplaceAreaEmpty: " << SDE_OSNV(area);
    return make_unexpected(TextureError::kReplaceAreaEmpty);
  }

  const std::size_t required_size = size_in_bytes(area.max() - area.min(), texture.layout);
  const std::size_t actual_size = sizeof(DataT) * data.size();
  if (actual_size != required_size)
  {
    SDE_LOG_ERROR() << "InvalidDataLength: " << SDE_OSNV(actual_size) << ", " << SDE_OSNV(required_size);
    return make_unexpected(TextureError::kInvalidDataLength);
  }

  if (Bounds2i{Vec2i::Zero(), texture.shape.value}.contains(area))
  {
    glBindTexture(GL_TEXTURE_2D, texture.native_id);
    return upload_texture_2D(data.data(), texture.layout, typecode<DataT>(), area.min(), area.max() - area.min());
  }

  SDE_LOG_ERROR() << "ReplaceAreaOutOfBounds";
  return make_unexpected(TextureError::kReplaceAreaOutOfBounds);
}

template expected<void, TextureError>
replace(const Texture& texture, View<const std::uint8_t> data, const Bounds2i& area);

template expected<void, TextureError>
replace(const Texture& texture, View<const std::uint16_t> data, const Bounds2i& area);

template expected<void, TextureError>
replace(const Texture& texture, View<const std::uint32_t> data, const Bounds2i& area);

template expected<void, TextureError> replace(const Texture& texture, View<const float> data, const Bounds2i& area);

template <typename DataT>
expected<Texture, TextureError> TextureCache::generate(
  dependencies deps,
  View<const DataT> data,
  const TextureShape& shape,
  const TextureLayout layout,
  const TextureOptions& options)
{
  auto native_texture_or_error = create_texture_impl(data, shape, layout, options);
  if (!native_texture_or_error.has_value())
  {
    return make_unexpected(native_texture_or_error.error());
  }
  return Texture{
    .source_image = ImageHandle::null(),
    .element_type = typecode<DataT>(),
    .layout = layout,
    .shape = shape,
    .options = options,
    .native_id = std::move(native_texture_or_error).value()};
}

template expected<Texture, TextureError> TextureCache::generate(
  dependencies deps,
  View<const std::uint8_t> data,
  const TextureShape& shape,
  const TextureLayout layout,
  const TextureOptions& options);

template expected<Texture, TextureError> TextureCache::generate(
  dependencies deps,
  View<const std::uint16_t> data,
  const TextureShape& shape,
  const TextureLayout layout,
  const TextureOptions& options);

template expected<Texture, TextureError> TextureCache::generate(
  dependencies deps,
  View<const std::uint32_t> data,
  const TextureShape& shape,
  const TextureLayout layout,
  const TextureOptions& options);

template expected<Texture, TextureError> TextureCache::generate(
  dependencies deps,
  View<const float> data,
  const TextureShape& shape,
  const TextureLayout layout,
  const TextureOptions& options);

expected<Texture, TextureError>
TextureCache::generate(dependencies deps, const asset::path& image_path, const TextureOptions& options)
{
  auto image_or_error = deps.get<ImageCache>().create(image_path, ImageOptions{.flip_vertically = false});
  if (!image_or_error.has_value())
  {
    return make_unexpected(TextureError::kInvalidSourceImage);
  }
  return generate(deps, image_or_error->handle, options);
}

expected<Texture, TextureError>
TextureCache::generate(dependencies deps, const ImageHandle& image, const TextureOptions& options)
{
  if (image.isNull())
  {
    return make_unexpected(TextureError::kInvalidSourceImage);
  }
  auto image_info = deps.get<ImageCache>().get_if(image);
  if (image_info == nullptr)
  {
    return make_unexpected(TextureError::kInvalidSourceImage);
  }
  SDE_LOG_DEBUG_FMT(
    "Creating texture from image: %s (%d x %d) (%lu bytes)",
    image_info->path.string().c_str(),
    image_info->shape.value.x(),
    image_info->shape.value.y(),
    image_info->getTotalSizeInBytes());
  auto texture_or_error = TextureCache::generate(
    deps,
    image_info->data(),
    TextureShape{.value = image_info->shape.value},
    layout_from_channel_count(image_info->getChannelCount()),
    options);
  if (texture_or_error.has_value())
  {
    texture_or_error->source_image = image;
  }
  return texture_or_error;
}

expected<Texture, TextureError> TextureCache::generate(
  dependencies deps,
  TypeCode type,
  const TextureShape& shape,
  TextureLayout layout,
  const TextureOptions& options)
{
  Texture texture{
    .source_image = ImageHandle::null(),
    .element_type = type,
    .layout = layout,
    .shape = shape,
    .options = options,
    .native_id = NativeTextureID{0}};
  if (auto ok_or_error = reload(deps, texture); !ok_or_error.has_value())
  {
    return make_unexpected(ok_or_error.error());
  }
  return texture;
}

expected<void, TextureError> TextureCache::reload(dependencies deps, Texture& texture)
{
  auto native_texture_or_error =
    create_texture_impl(texture.element_type, texture.shape, texture.layout, texture.options);
  if (!native_texture_or_error.has_value())
  {
    return make_unexpected(native_texture_or_error.error());
  }
  texture.native_id = std::move(native_texture_or_error).value();

  if (texture.source_image.isNull())
  {
    SDE_LOG_DEBUG_FMT("Creating empty texture: (%d x %d)", texture.shape.value.x(), texture.shape.value.y());
    return {};
  }

  auto image = deps.get<ImageCache>().get_if(texture.source_image);
  if (image == nullptr)
  {
    return make_unexpected(TextureError::kInvalidSourceImage);
  }

  SDE_LOG_DEBUG_FMT(
    "Creating texture from image: %s (%d x %d) (%lu bytes)",
    image->path.string().c_str(),
    image->shape.value.x(),
    image->shape.value.y(),
    image->getTotalSizeInBytes());

  return replace(texture, image->data());
}


expected<void, TextureError> TextureCache::unload(dependencies deps, Texture& texture)
{
  texture.native_id = NativeTextureID{0};
  return {};
}

}  // namespace sde::graphics
